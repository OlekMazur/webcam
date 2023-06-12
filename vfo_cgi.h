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

#ifndef	VFO_CGI_H
#define	VFO_CGI_H

/**
 * @addtogroup vfo
 * @{
 * @defgroup vfo_cgi CGI output
 * @{
 * Provides JPEG frames as multipart/x-mixed-replace via CGI responder interface
 */

#include "vfo.h"

/**
 * Initializes CGI output and emits initial part of a 200 OK response
 * of multipart/x-mixed-replace MIME type.
 *
 * Further frames are emitted as boundary-separated parts of image/jpeg
 * MIME type.
 *
 * @param output 
 * @return An instance of CGI output interface, or NULL on error.
 */
video_frame_output_t *video_frame_output_cgi_init(FILE *output);

/**
 * @}
 * @}
 */

#endif
