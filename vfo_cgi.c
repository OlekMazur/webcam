/*
 * This file is part of webcam.
 *
 * Copyright (c) 2013 Aleksander Mazur
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
#include "vfo_cgi.h"

/**
 * @addtogroup vfo_cgi
 * @{
 */

/**************************************/

/** Instance of a CGI output. */
typedef struct {
	video_frame_output_t base;	/**< Base structure. */
	FILE *output;				/**< Destination file. */
	char boundary[32];			/**< Boundary separating parts of multipart/x-mixed-replace MIME type. */
	int boundary_started;		/**< Whether the boundary has been already emitted to the output. */
} video_frame_output_cgi_t;

/**************************************/

/** @copydoc video_frame_output_ops_t::PutFrame */
static void video_frame_output_cgi_PutFrame(video_frame_output_t *base, video_frame_filter_t *filter)
{
	video_frame_output_cgi_t *thiz = (video_frame_output_cgi_t *) base;
	int ok;

	if (!thiz->boundary_started) {
		fprintf(thiz->output, "--%s\r\n", thiz->boundary);
		thiz->boundary_started++;
	}
	fprintf(thiz->output, 
		"Content-type: image/jpeg\r\n"
		"Content-length: %tu\r\n"
		"\r\n",
		filter->op->GetSize(filter));

	for (ok = 1; ok;) {
		const unsigned char *buffer;
		size_t size;

		filter->op->Read(filter, &buffer, &size);
		if (size) {
			size_t written = 0;

			while (written < size) {
				int once = fwrite(buffer + written, 1, size - written, thiz->output);
				if (once <= 0) {
					perror("fwrite");
					return;
				}
				written += once;
			}
			if (written != size) {
				ok = 0;
			}
		} else {
			break;
		}
	}

	if (ok) {
		fprintf(thiz->output, "\n--%s\r\n", thiz->boundary);
	} else {
		fprintf(thiz->output, "\n--%s--\r\n", thiz->boundary);
	}
	fflush(thiz->output);
}

/** @copydoc video_frame_output_ops_t::Destroy */
static void video_frame_output_cgi_Destroy(video_frame_output_t *base)
{
	video_frame_output_cgi_t *thiz = (video_frame_output_cgi_t *) base;

	fclose(thiz->output);
	free(thiz);
}

/** Operations of the CGI output. */
video_frame_output_ops_t video_frame_output_cgi_ops = {
	.PutFrame = video_frame_output_cgi_PutFrame,
	.Destroy = video_frame_output_cgi_Destroy,
};

/**************************************/

video_frame_output_t *video_frame_output_cgi_init(FILE *output)
{
	video_frame_output_cgi_t *rv = (video_frame_output_cgi_t *) calloc(1, sizeof(video_frame_output_cgi_t));
	size_t i;

	rv->base.op = &video_frame_output_cgi_ops;
	rv->output = output;

	for (i = 0; i < sizeof(rv->boundary) - 1; i++) {
		int x = rand() % ('9' - '0' + 1 + 'z' - 'a' + 1);
		int c;

		if (x <= 9)
			c = x + '0';
		else
			c = x - 10 + 'a';
		rv->boundary[i] = c;
	}
	rv->boundary[sizeof(rv->boundary) - 1] = 0;
	rv->boundary_started = 0;

	fprintf(output, 
		"HTTP/1.0 200 OK\r\n"
		"Connection: close\r\n"
		"Server: OLO Webcam CGI v1.2\r\n"
		"Pragma: no-cache\r\n"
		"Content-type: multipart/x-mixed-replace; boundary=%s\r\n"
		"\r\n",
		rv->boundary);

	return &rv->base;
}

/**
 * @}
 */
