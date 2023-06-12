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

#ifndef CAPTURE_V4L2_H
#define CAPTURE_V4L2_H

/**
 * @addtogroup capture
 * @{
 * @defgroup capture_v4l2 V4L2 capture
 * @{
 * Captures video data from Video4Linux2 devices
 */

#include "capture.h"

/**
 * Initializes V4L2 capture.
 *
 * @param verbose Whether to produce verbose messages on stderr.
 * @param user_path User-specified path to the V4L2 device (e.g. /dev/video0),
 *                  or NULL - in this case the function will try to
 *                  find a suitable device among available ones.
 * @param user_width Desired frame width, in pixels.
 * @param user_height Desired frame height, in pixels.
 * @param user_fr Desired frame rate, per second.
 * @param max_mem Maximum amount of RAM to allocate for buffers, in bytes.
 * @return An instance of V4L2 capture, or NULL on error.
 */
capture_interface_t *capture_init_v4l2(int verbose, const char *user_path, unsigned user_width, unsigned user_height, unsigned user_fr, size_t max_mem);

/**
 * @}
 * @}
 */

#endif
