/*
 * This file is part of webcam.
 *
 * Copyright (c) 2023 Aleksander Mazur
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

#ifndef	VFF_YUV2JPEG_H
#define	VFF_YUV2JPEG_H

/**
 * @addtogroup vff
 * @{
 * @defgroup vff_yuv2jpeg YUV to JPEG filter
 * @{
 * Compresses YUV 4:2:2 (packed) frame to JPEG
 */

#include "vff.h"

/**
 * Creates an instance of a YUV 4:2:2 packed to JPEG frame filter.
 *
 * @param width Width of the frames that will be provided to the filter, in pixels.
 * @param height Height of the frames that will be provided to the filter, in pixels.
 * @param bytesperline Bytes per each line of the frame that will be provided to the filter, including padding, if any.
 * @param quality Desired quality of JPEG images, of UINT_MAX in case of no preference.
 * @return An instance of the YUV to JPEG frame filter, or NULL on error.
 */
video_frame_filter_t *vff_yuv2jpeg_create(unsigned width, unsigned height, unsigned bytesperline, unsigned quality);

/**
 * @}
 * @}
 */

#endif
