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

#ifndef	VFO_FILES_H
#define	VFO_FILES_H

/**
 * @addtogroup vfo
 * @{
 * @defgroup vfo_files Files output
 * @{
 * Saves each frame as separate JPEG file
 */

#include "vfo.h"

/**
 * Initializes output to multiple files.
 *
 * Each frame is saved as a separate file in @c capture/ subdirectory.
 *
 * @return An instance of multiple files output interface, or NULL on error.
 */
video_frame_output_t *video_frame_output_files_init(void);

/**
 * @}
 * @}
 */

#endif
