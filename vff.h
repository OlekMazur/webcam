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

#ifndef	VIDEO_FRAME_FILTER_H
#define	VIDEO_FRAME_FILTER_H

/**
 * @defgroup vff Video Frame Filter interface
 * @{
 * Alters video data between capture interface and output interface
 */

#include <stdio.h>

/** Video frame filter instance. */
typedef struct video_frame_filter_t video_frame_filter_t;

/** Video frame filter operations. */
typedef struct {

	/**
	 * Puts a video frame into the filter. Processed data are available to read via
	 * GetSize() and Read() right after this call. Data passed to this function must
	 * remain valid until PutFrame() is called the next time or Destroy() is called.
	 *
	 * @param base Instance of a video frame filter.
	 * @param frame Pointer to the frame data.
	 * @param size Size of frame data.
	 */
	void (*PutFrame)(video_frame_filter_t *base, const unsigned char *frame, size_t size);

	/**
	 * Retrieves size of the video frame on the output of the filter.
	 *
	 * @param base Instance of a video frame filter.
	 * @return Size of frame data on the output.
	 */
	size_t (*GetSize)(video_frame_filter_t *base);

	/**
	 * Reads video data from filter, processed from a frame passed to the last PutFrame() call.
	 * If no more data are available (complete frame data has been already read out),
	 * *size becomes 0.
	 *
	 * @param base Instance of a video frame filter.
	 * @param data Receives pointer to the beginning of a data chunk.
	 * @param size Receives size of the data chunk, or 0 if no more video frame data are left.
	 */
	void (*Read)(video_frame_filter_t *base, const unsigned char **data, size_t *size);

	/**
	 * Destroys instance of video frame filter.
	 *
	 * @param base Instance of a video frame filter.
	 */
	void (*Destroy)(video_frame_filter_t *base);

} video_frame_filter_ops_t;

/** Video frame filter instance. */
struct video_frame_filter_t {
	/** Video frame filter operations. */
	video_frame_filter_ops_t *op;
};

/**
 * @}
 */

#endif
