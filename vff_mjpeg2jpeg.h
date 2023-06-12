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

#ifndef	VFF_MJPEG2JPEG_H
#define	VFF_MJPEG2JPEG_H

/**
 * @addtogroup vff
 * @{
 * @defgroup vff_mjpeg2jpeg MJPEG to JPEG filter
 * @{
 * Converts MJPEG frame to JPEG
 */

#include "vff.h"

/**
 * Creates an instance of a MJPEG to JPEG video frame filter.
 *
 * @return An instance of a MJPEG to JPEG video frame filter, or NULL on error.
 */
video_frame_filter_t *vff_mjpeg2jpeg_create(void);

/**
 * @}
 * @}
 */

#endif
