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
#include "vfo_stdout.h"

/**
 * @addtogroup vfo_stdout
 * @{
 */

/**************************************/

/** @copydoc video_frame_output_ops_t::PutFrame */
static void video_frame_output_stdout_PutFrame(video_frame_output_t *base, video_frame_filter_t *filter)
{
	(void) base;

	for (;;) {
		const unsigned char *buffer;
		size_t size;

		filter->op->Read(filter, &buffer, &size);
		if (size) {
			size_t written = 0;

			while (written < size) {
				int once = fwrite(buffer + written, 1, size - written, stdout);
				if (once <= 0) {
					perror("fwrite");
					return;
				}
				written += once;
			}
			if (written != size) {
				break;
			}
		} else {
			break;
		}
	}
}

/** @copydoc video_frame_output_ops_t::Destroy */
static void video_frame_output_stdout_Destroy(video_frame_output_t *base)
{
	free(base);
}

/** Operations of the stdout output. */
video_frame_output_ops_t video_frame_output_stdout_ops = {
	.PutFrame = video_frame_output_stdout_PutFrame,
	.Destroy = video_frame_output_stdout_Destroy,
};

/**************************************/

video_frame_output_t *video_frame_output_stdout_init(void)
{
	video_frame_output_t *rv = (video_frame_output_t *) calloc(1, sizeof(video_frame_output_t));
	rv->op = &video_frame_output_stdout_ops;
	return rv;
}

/**
 * @}
 */
