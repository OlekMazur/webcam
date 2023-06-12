/*
 * This file is part of webcam.
 *
 * Copyright (c) 2013, 2023 Aleksander Mazur
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

#ifndef CAPTURE_H
#define CAPTURE_H

/**
 * @defgroup capture Capture interface
 * @{
 * Captures video data
 */

/** Capture interface. */
typedef struct capture_interface_t capture_interface_t;

/** Capture data format. */
typedef enum {
	/** Data given frame by frame, each as a YUV 4:2:2 image, packed. */
	CAPTURE_FMT__YUV422_PACKED,
	/** Data given frame by frame, each as a JPEG image. */
	CAPTURE_FMT__JPEG,
	/** Data given frame by frame, each as a MJPEG image (JPEG without Huffman table). */
	CAPTURE_FMT__MJPEG,
} capture_data_format_e;

/** Capture data format. */
typedef struct {
	capture_data_format_e fmt;	/**< Data format. */
	unsigned width;				/**< Width of captured frame. */
	unsigned height;			/**< Height of captured frame. */
	unsigned bytesperline;		/**< Bytes per line. Meaningful in @ref CAPTURE_FMT__YUV422_PACKED. */
} capture_data_format_t;

/** Capture interface operations. */
typedef struct {

	/**
	 * Returns format of the data captured by given instance of the interface.
	 *
	 * @param base Pointer to the instance of the capture interface.
	 * @return Capture data format. Returned structure must be valid
	 *         until interface is destroyed.
	 */
	const capture_data_format_t* (*GetFormat)(capture_interface_t *base);

	/**
	 * Captures data. The returned buffer must be passed to ReleaseBuffer() when no longer needed.
	 *
	 * @param base Pointer to the instance of the capture interface.
	 * @param buffer Pointer to a pointer which receives the address of the beginning of the capture buffer on success.
	 * @param size Pointer to a size of the data in capture buffer, set on success.
	 * @return On success, a positive number is returned and this number must be passed to ReleaseBuffer when
	 * 		   processing of the data is finished. On failure, a negative number is returned.
	 */
	int (*Capture)(capture_interface_t *base, unsigned char **buffer, size_t *size);

	/**
	 * Releases buffer obtained from Capture().
	 *
	 * @param base Pointer to the instance of the capture interface.
	 * @param index Index of the buffer to release, returned by Capture().
	 */
	void (*ReleaseBuffer)(capture_interface_t *base, int index);

	/**
	 * Destroys the interface instance, freeing all allocated resources.
	 *
	 * @param base Pointer to the instance of the capture interface.
	 */
	void (*Destroy)(capture_interface_t *base);

} capture_interface_ops_t;

/** Capture interface. */
struct capture_interface_t {
	/** Capture interface operations. */
	capture_interface_ops_t *op;
};

/**
 * @}
 */

#endif
