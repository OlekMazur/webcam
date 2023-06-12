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

#ifndef	VFO_STDOUT_H
#define	VFO_STDOUT_H

/**
 * @addtogroup vfo
 * @{
 * @defgroup vfo_stdout stdout output
 * @{
 * Emits video frames to stdout
 */

#include "vfo.h"

/**
 * Initializes stdout output, which just passes by all incoming frame
 * data to stdout.
 *
 * @return A stdout output interface, or NULL on error.
 */
video_frame_output_t *video_frame_output_stdout_init(void);

/**
 * @}
 * @}
 */

#endif
