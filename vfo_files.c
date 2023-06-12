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
#include "vfo_files.h"

/**
 * @addtogroup vfo_files
 * @{
 */

/**************************************/

/** Instance of a multiple files output. */
typedef struct {
	video_frame_output_t base;	/**< Base structure. */
	int frame_no;				/**< Frame number, incremented each frame, used to create output file name. */
} video_frame_output_files_t;

/**************************************/

/** @copydoc video_frame_output_ops_t::PutFrame */
static void video_frame_output_files_PutFrame(video_frame_output_t *base, video_frame_filter_t *filter)
{
	video_frame_output_files_t *thiz = (video_frame_output_files_t *) base;
    char fname[32];
    FILE *f;

	if (!filter->op->GetSize(filter))
		return;

    sprintf(fname, "capture/%08d.jpg", thiz->frame_no++);
    f = fopen(fname, "w");
    if (f) {
		for (;;) {
			const unsigned char *buffer;
			size_t size;

			filter->op->Read(filter, &buffer, &size);
			if (size) {
				size_t written;
				ssize_t once;

				for (written = 0; written < size; written += once) {
					once = fwrite(buffer + written, 1, size - written, f);
					if (once <= 0) {
						perror("fwrite");
						return;
					}
				}
			} else {
				break;
			}
		}
		fclose(f);
		fprintf(stderr, "%s created\n", fname);
	} else {
        perror(fname);
	}
}

/** @copydoc video_frame_output_ops_t::Destroy */
static void video_frame_output_files_Destroy(video_frame_output_t *base)
{
	video_frame_output_files_t *thiz = (video_frame_output_files_t *) base;

	free(thiz);
}

/** Operations of the multiple files output. */
video_frame_output_ops_t video_frame_output_files_ops = {
	.PutFrame = video_frame_output_files_PutFrame,
	.Destroy = video_frame_output_files_Destroy,
};

/**************************************/

video_frame_output_t *video_frame_output_files_init(void)
{
	video_frame_output_files_t *rv = (video_frame_output_files_t *) calloc(1, sizeof(video_frame_output_files_t));
	rv->base.op = &video_frame_output_files_ops;
	return &rv->base;
}

/**
 * @}
 */
