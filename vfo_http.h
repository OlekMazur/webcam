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

#ifndef	VFO_HTTP_H
#define	VFO_HTTP_H

/**
 * @addtogroup vfo
 * @{
 * @defgroup vfo_http HTTP output
 * @{
 * Provides JPEG frames as multipart/x-mixed-replace to single HTTP client
 */

#include "vfo.h"

/**
 * Initializes HTTP output.
 *
 * Opens a port, accepts one connection, parses GET / query, then returns
 * an interface which works exactly like @ref vfo_cgi.
 *
 * @param port TCP port number on which we should listen to incoming HTTP query.
 * @return An HTTP output interface, or NULL on error.
 */
video_frame_output_t *video_frame_output_http_init(unsigned short port);

/**
 * @}
 * @}
 */

#endif
