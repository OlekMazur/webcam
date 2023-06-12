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

#ifndef VIDEO_FRAME_OUTPUT_H
#define VIDEO_FRAME_OUTPUT_H

/**
 * @defgroup vfo Video Frame Output interface
 * @{
 * Sinks filtered video frames
 */

#include "vff.h"

/** Video frame output instance. */
typedef struct video_frame_output_t video_frame_output_t;

/** Video frame filter operations. */
typedef struct {

	/**
	 * Reads a video frame data from a filter and writes it to the output.
	 *
	 * @param base Instance of a video frame output.
	 * @param filter Instance of a video frame filter from which the data will be read.
	 */
	void (*PutFrame)(video_frame_output_t *base, video_frame_filter_t *filter);

	/**
	 * Destroys instance of video frame output.
	 *
	 * @param base Instance of a video frame output.
	 */
	void (*Destroy)(video_frame_output_t *base);

} video_frame_output_ops_t;

/** Video frame filter instance. */
struct video_frame_output_t {
	/** Video frame filter operations. */
	video_frame_output_ops_t *op;
};

/**
 * @}
 */

#endif
