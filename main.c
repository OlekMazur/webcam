/*
 * This file is part of webcam.
 *
 * Copyright (c) 2013, 2023 Aleksander Mazur
 *
 * webcam is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * webcam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with webcam. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include "capture.h"
#include "capture_v4l2.h"
#include "vff_null.h"
#include "vff_mjpeg2jpeg.h"
#include "vff_yuv2jpeg.h"
#include "vfo_stdout.h"
#include "vfo_files.h"
#include "vfo_cgi.h"
#include "vfo_http.h"

/**
 * @defgroup main Main module
 * @{
 */

/** If non-zero, verbose messages are printed on stderr. */
static int verbose;
/** If non-zero, main loop keeps iterating. */
static int run;

/**
 * Handles a signal to stop the program.
 *
 * Stopping program is done by clearing @ref run.
 *
 * @param signum Signal number.
 */
static void signal_stop(int signum)
{
	if (verbose)
		fprintf(stderr, "%s: quitting\n", strsignal(signum));
	run = 0;
}

/**
 * Initializes signal handling.
 *
 * Assigns an action for @c SIGTERM which gracefully quits the program.
 */
static void init_signals(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_stop;
	sigaction(SIGTERM, &act, NULL);
}

/**
 * Entrypoint and main loop of the program.
 *
 * @param argc Arguments count.
 * @param argv Array of argument values.
 * @return Exit code.
 */
int main(int argc, char **argv)
{
	int opt, rv = 0;
	unsigned width = 0, height = 0, frame_rate = 0;
#ifdef	USE_JPEGLIB
	unsigned jpeg_quality = UINT_MAX;
#endif
	unsigned short port = 0;
	size_t max_mem = 8;	/* 8 MB */
	const char *dev_path = NULL;
	const char *mode = "cgi";
	capture_interface_t *cap = NULL;
	video_frame_filter_t *filter = NULL;
	video_frame_output_t *out = NULL;

	/* initialize signals */
	init_signals();

	/* parse arguments */
	while (!rv && (opt = getopt(argc, argv, "vd:w:h:r:m:o:p:q:")) != -1) {
		switch (opt) {
			case 'v':
				verbose = 1;
				break;
			case 'd':
				dev_path = optarg;
				break;
			case 'w':
				if (sscanf(optarg, "%u", &width) != 1) {
					fprintf(stderr, "Width expected, but found %s\n", optarg);
					rv = 1;
				}
				break;
			case 'h':
				if (sscanf(optarg, "%u", &height) != 1) {
					fprintf(stderr, "Height expected, but found %s\n", optarg);
					rv = 2;
				}
				break;
			case 'r':
				if (sscanf(optarg, "%u", &frame_rate) != 1) {
					fprintf(stderr, "Frame rate expected, but found %s\n", optarg);
					rv = 3;
				}
				break;
			case 'm':
				if (sscanf(optarg, "%tu", &max_mem) != 1) {
					fprintf(stderr, "Memory limit in megabytes expected, but found %s\n", optarg);
					rv = 4;
				}
				break;
			case 'p':
				if (sscanf(optarg, "%hu", &port) != 1) {
					fprintf(stderr, "Port expected, but found %s\n", optarg);
					rv = 5;
				}
				break;
#ifdef	USE_JPEGLIB
			case 'q':
				if (sscanf(optarg, "%u", &jpeg_quality) != 1) {
					fprintf(stderr, "JPEG quality expected, but found %s\n", optarg);
					rv = 5;
				}
				break;
#endif
			case 'o':
				mode = optarg;
				break;
			default:
				fprintf(stderr, "Usage: %s [-v] [-w width] [-h height] [-r frame-rate] [-m max-memory-MB] [-o {stdout|files|cgi|http}]\n", argv[0]);
				rv = 6;
				break;
		}
	}

	if (verbose) {
		fprintf(stderr, "%s: output='%s', dev path='%s', width=%u, height=%u, frame rate=%u, max mem=%tu\n",
			argv[0], mode, dev_path, width, height, frame_rate, max_mem);
	}
	max_mem *= 1024 * 1024;
	/* setup output */
	if (!strcmp(mode, "stdout")) {
		out = video_frame_output_stdout_init();
	} else if (!strcmp(mode, "files")) {
		out = video_frame_output_files_init();
	} else if (!strcmp(mode, "cgi")) {
		out = video_frame_output_cgi_init(stdout);
	} else if (!strcmp(mode, "http")) {
		out = video_frame_output_http_init(port);
	} else {
		rv = 7;
	}

	do {
		const capture_data_format_t *format;

		if (rv)
			break;

		if (!out) {
			fprintf(stderr, "Could not initialize frame output\n");
			rv = 8;
			break;
		}

		/* setup input */
		cap = capture_init_v4l2(verbose, dev_path, width, height, frame_rate, max_mem);
		if (!cap) {
			fprintf(stderr, "Could not initialize capture interface\n");
			rv = 9;
			break;
		}
		format = cap->op->GetFormat(cap);
		/* setup filter appropriate for given input */
		switch (format->fmt) {
			case CAPTURE_FMT__JPEG:
				filter = vff_null_create();
				break;
			case CAPTURE_FMT__MJPEG:
				filter = vff_mjpeg2jpeg_create();
				break;
#ifdef	USE_JPEGLIB
			case CAPTURE_FMT__YUV422_PACKED:
				filter = vff_yuv2jpeg_create(format->width, format->height, format->bytesperline, jpeg_quality);
				break;
#endif
			default:
				fprintf(stderr, "Unsupported input format\n");
				break;
		}
		if (!filter) {
			fprintf(stderr, "Could not initialize data filter\n");
			rv = 10;
			break;
		}

		/* main loop */
		for (run = 1; run; ) {
			unsigned char *buffer;
			size_t size;
			/* capture frame */
			int index = cap->op->Capture(cap, &buffer, &size);

			if (index < 0)
				break;

			/* pass frame to the filter */
			filter->op->PutFrame(filter, buffer, size);
			/* pass filtered frame to the output */
			out->op->PutFrame(out, filter);
			/* release captured frame */
			cap->op->ReleaseBuffer(cap, index);
		}
	} while (0);

	/* cleanup */
	if (out)
		out->op->Destroy(out);
	if (filter)
		filter->op->Destroy(filter);
	if (cap)
		cap->op->Destroy(cap);
    return rv;
}

/**
 * @}
 */
