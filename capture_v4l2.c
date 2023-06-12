/*
 * This file is part of webcam.
 *
 * Copyright (c) 2013, 2022 Aleksander Mazur
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "capture_v4l2.h"

/**
 * @addtogroup capture_v4l2
 * @{
 */

/************************************************/

/** Information about a single frame buffer, allocated by @c VIDIOC_QUERYBUF and then mmap'ped. */
typedef struct {
	unsigned char *start;	/**< Pointer to the beginning of the buffer. */
	size_t size;			/**< Size of the buffer. */
} capture_buffer_t;

/** Instance of V4L2 capture interface implementation. */
typedef struct {
	capture_interface_t base;		/**< Base structure. */
	int fd;							/**< File descriptor of the V4L2 device. */
	size_t max_size;				/**< Maximum size of a frame, as determined by @c VIDIOC_G_FMT. */
	capture_data_format_t format;	/**< Capture data format returned by @ref capture_v4l2_streaming_GetFormat. */
	int buffers_cnt;				/**< Number of allocated frame buffers. */
	capture_buffer_t *buffers;		/**< Information about allocated buffers (@c buffers_cnt entries). */
} capture_v4l2_streaming_t;

/************************************************/

/** @copydoc capture_interface_ops_t::GetFormat */
static const capture_data_format_t* capture_v4l2_streaming_GetFormat(capture_interface_t *base)
{
	capture_v4l2_streaming_t *thiz = (capture_v4l2_streaming_t *) base;
	return &thiz->format;
}

/** @copydoc capture_interface_ops_t::Capture */
static int capture_v4l2_streaming_Capture(capture_interface_t *base, unsigned char **buffer, size_t *size)
{
	capture_v4l2_streaming_t *thiz = (capture_v4l2_streaming_t *) base;
	struct v4l2_buffer v4l2buf;

	memset(&v4l2buf, 0, sizeof(v4l2buf));
	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(thiz->fd, VIDIOC_DQBUF, &v4l2buf)) {
		fprintf(stderr, "VIDIOC_DQBUF: %s\n", strerror(errno));
		return -1;
	}

	*buffer = thiz->buffers[v4l2buf.index].start;
	*size = v4l2buf.bytesused;
	return v4l2buf.index;
}

/** @copydoc capture_interface_ops_t::ReleaseBuffer */
static void capture_v4l2_streaming_ReleaseBuffer(capture_interface_t *base, int index)
{
	capture_v4l2_streaming_t *thiz = (capture_v4l2_streaming_t *) base;
	struct v4l2_buffer buffer;

	memset(&buffer, 0, sizeof(buffer));
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = index;

	if (ioctl(thiz->fd, VIDIOC_QBUF, &buffer)) {
		fprintf(stderr, "VIDIOC_QBUF[%d]: %s\n", index, strerror(errno));
	}
}

/** @copydoc capture_interface_ops_t::Destroy */
static void capture_v4l2_streaming_Destroy(capture_interface_t *base)
{
	capture_v4l2_streaming_t *thiz = (capture_v4l2_streaming_t *) base;
	int i;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* turn off video capture */
	if (ioctl(thiz->fd, VIDIOC_STREAMOFF, &type)) {
		fprintf(stderr, "VIDIOC_STREAMOFF: %s\n", strerror(errno));
	}
	/* release buffers */
	for (i = 0; i < thiz->buffers_cnt; i++) {
		munmap(thiz->buffers[i].start, thiz->buffers[i].size);
	}
	close(thiz->fd);
	free(thiz);
}

/************************************************/

/** Operations of V4L2 capture. */
static capture_interface_ops_t capture_v4l2_streaming_ops = {
	.GetFormat = capture_v4l2_streaming_GetFormat,
	.Capture = capture_v4l2_streaming_Capture,
	.ReleaseBuffer = capture_v4l2_streaming_ReleaseBuffer,
	.Destroy = capture_v4l2_streaming_Destroy,
};

/**
 * Allocates an instance of V4L2 capture.
 *
 * @param path Path to the V4L2 device (e.g. /dev/video0).
 * @param fd File descriptor of the V4L2 device.
 * @param format Format of frame data.
 * @param width Frame width, in pixels.
 * @param height Frame height, in pixels.
 * @param reqbuf_count Number of frame buffers allocated by V4L2 layer.
 * @param sizeimage Maximum size of a frame, as determined by @c VIDIOC_G_FMT.
 * @param bytesperline Bytes per each line of the frame, including padding, if any.
 * @return An instance of V4L2 capture, or NULL on error.
 */
static capture_v4l2_streaming_t *capture_new_v4l2(const char *path, int fd, capture_data_format_e format, unsigned width, unsigned height, unsigned reqbuf_count, unsigned sizeimage, unsigned bytesperline)
{
    capture_v4l2_streaming_t *rv = (capture_v4l2_streaming_t *) calloc(1, sizeof(capture_v4l2_streaming_t) + reqbuf_count * sizeof(capture_buffer_t));
    int j;

	rv->base.op = &capture_v4l2_streaming_ops;
	rv->fd = fd;
	rv->max_size = sizeimage;
	rv->format.fmt = format;
	rv->format.width = width;
	rv->format.height = height;
	rv->format.bytesperline = bytesperline;
	rv->buffers_cnt = reqbuf_count;
	rv->buffers = (capture_buffer_t *) (rv + 1);
	/* query & mmap buffers allocated by V4L2 layer, fill buffers array */
	for (j = 0; j < (int) reqbuf_count; j++) {
		struct v4l2_buffer buffer;

		memset(&buffer, 0, sizeof(buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = (__u32) j;

		if (ioctl(fd, VIDIOC_QUERYBUF, &buffer)) {
			fprintf(stderr, "%s: VIDIOC_QUERYBUF[%d]: %s\n", path, j, strerror(errno));
			break;
		}

		rv->buffers[j].start = mmap(NULL, buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
		if (rv->buffers[j].start == MAP_FAILED) {
			fprintf(stderr, "%s: mmap[%d](length=%u, fd=%d, offset=%u): %s\n", path, j, buffer.length, fd, buffer.m.offset, strerror(errno));
			break;
		}
		rv->buffers[j].size = buffer.length;
	}
	if (j == (int) reqbuf_count) {
		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		/* enqueue all buffers (let V4L2 layer fill them with captured frame data) */
		for (j = 0; j < (int) reqbuf_count; j++) {
			capture_v4l2_streaming_ReleaseBuffer(&rv->base, j);
		}
		/* turn on video capture */
		if (ioctl(fd, VIDIOC_STREAMON, &type)) {
			fprintf(stderr, "VIDIOC_STREAMON: %s\n", strerror(errno));
		}
		/* success */
		return rv;
	}

	/* roll back */
	for (j--; j >= 0; j--) {
		munmap(rv->buffers[j].start, rv->buffers[j].size);
	}
	free(rv);
	return NULL;
}

/************************************************/

/**
 * Initialize video capture by given V4L2 device.
 *
 * @param verbose Whether to print verbose messages to stderr.
 * @param path Path to the V4L2 device (e.g. /dev/video0).
 * @param user_width Desired frame width, in pixels.
 * @param user_height Desired frame height, in pixels.
 * @param user_fr Desired frame rate, per second.
 * @param max_mem Maximum amount of RAM to allocate for buffers, in bytes.
 * @return An instance of V4L2 capture, or NULL on error.
 */
static capture_v4l2_streaming_t *capture_init_v4l2_dev(int verbose, const char *path, unsigned user_width, unsigned user_height, unsigned user_fr, size_t max_mem)
{
	int fd = open(path, O_RDWR);

	if (fd < 0) {
		if (errno != ENOENT)
			perror(path);
		return NULL;
	}
	do {
		static const struct {
			__u32 fmt_v4l2;
			capture_data_format_e fmt_my;
		} fmt[] = {
			{ V4L2_PIX_FMT_JPEG, CAPTURE_FMT__JPEG },
			{ V4L2_PIX_FMT_MJPEG, CAPTURE_FMT__MJPEG },
			{ V4L2_PIX_FMT_YUYV, CAPTURE_FMT__YUV422_PACKED },
		};
		unsigned i, ok = 0;
		struct v4l2_capability cap;
		struct v4l2_format format;
		struct v4l2_requestbuffers reqbuf;
		capture_data_format_e selected;

		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
			fprintf(stderr, "%s: VIDIOC_QUERYCAP: %s\n", path, strerror(errno));
			break;
		}
		if (verbose)
			fprintf(stderr, "%s: %s @ %s, %s %u.%u.%u, caps:0x%08X\n",
				path, cap.card, cap.bus_info, cap.driver,
				(cap.version >> 16) & 0xFF,
				(cap.version >> 8) & 0xFF,
				(cap.version) & 0xFF,
				cap.capabilities);
		if ((cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_STREAMING)) != (V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s: V4L2_CAP_VIDEO_CAPTURE and/or V4L2_CAP_STREAMING is not supported\n", path);
			break;
		}
		memset(&format, 0, sizeof(format));
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(fd, VIDIOC_G_FMT, &format) == -1) {
			fprintf(stderr, "%s: VIDIOC_G_FMT: %s", path, strerror(errno));
			break;
		}
		if (verbose)
			fprintf(stderr, "%s: %u x %u, %.4s, size %u, bpl %u\n",
				path,
				format.fmt.pix.width, format.fmt.pix.height,
				(const char *) &format.fmt.pix.pixelformat,
				format.fmt.pix.sizeimage, format.fmt.pix.bytesperline);
		for (i = 0; i < sizeof(fmt) / sizeof(fmt[0]); i++) {
			if (fmt[i].fmt_v4l2 == format.fmt.pix.pixelformat &&
				(!user_width || user_width == format.fmt.pix.width) &&
				(!user_height || user_height == format.fmt.pix.height)) {
				ok = 1;
				break;
			}
			format.fmt.pix.pixelformat = fmt[i].fmt_v4l2;
			if (user_width)
				format.fmt.pix.width = user_width;
			if (user_height)
				format.fmt.pix.height = user_height;
			if (ioctl(fd, VIDIOC_S_FMT, &format) == -1) {
				fprintf(stderr, "%s: VIDIOC_S_FMT[%u]: %s\n", path, i, strerror(errno));
			}
			if (ioctl(fd, VIDIOC_G_FMT, &format) == -1) {
				fprintf(stderr, "%s: VIDIOC_G_FMT[%u]: %s\n", path, i, strerror(errno));
				break;
			}
			if (fmt[i].fmt_v4l2 == format.fmt.pix.pixelformat &&
				(!user_width || user_width == format.fmt.pix.width) &&
				(!user_height || user_height == format.fmt.pix.height)) {
				ok = 1;
				break;
			}
		}
		if (!ok) {
			fprintf(stderr, "%s: couldn't initialize pixel format %u x %u\n", path, user_width, user_height);
			break;
		}
		selected = fmt[i].fmt_my;
		if (verbose)
			fprintf(stderr, "%s: %u x %u, %.4s, size %u, bpl %u\n",
				path,
				format.fmt.pix.width, format.fmt.pix.height,
				(const char *) &format.fmt.pix.pixelformat,
				format.fmt.pix.sizeimage, format.fmt.pix.bytesperline);
		if (user_fr) {
			struct v4l2_streamparm stream;

			memset(&stream, 0, sizeof(stream));
			stream.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (!ioctl(fd, VIDIOC_G_PARM, &stream) && (stream.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
				stream.parm.capture.timeperframe.numerator = 1;
				stream.parm.capture.timeperframe.denominator = user_fr;
				if (ioctl(fd, VIDIOC_S_PARM, &stream) == -1) {
					fprintf(stderr, "%s: VIDIOC_S_PARM: %s\n", path, strerror(errno));
				}
				if (verbose && ioctl(fd, VIDIOC_G_PARM, &stream) == 0) {
					fprintf(stderr, "%s: %u/%u s frame duration\n", path,
						stream.parm.capture.timeperframe.numerator,
						stream.parm.capture.timeperframe.denominator);
				}
			} else {
				fprintf(stderr, "%s: VIDIOC_G_PARM: %s\n", path, strerror(errno));
			}
		}

		memset(&reqbuf, 0, sizeof(reqbuf));
		reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf.memory = V4L2_MEMORY_MMAP;
		reqbuf.count = max_mem / format.fmt.pix.sizeimage;
		if (reqbuf.count < 2)
			reqbuf.count = 2;
		if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) {
			fprintf(stderr, "%s: VIDIOC_REQBUFS: %s\n", path, strerror(errno));
			break;
		}
		if (verbose)
			fprintf(stderr, "%s: buffers = %u\n", path, reqbuf.count);
		if (reqbuf.count < 2) {
			break;
		}
		return capture_new_v4l2(path, fd, selected, format.fmt.pix.width, format.fmt.pix.height, reqbuf.count, format.fmt.pix.sizeimage, format.fmt.pix.bytesperline);
	} while (0);

	close(fd);
	return NULL;
}

capture_interface_t *capture_init_v4l2(int verbose, const char *user_path, unsigned user_width, unsigned user_height, unsigned user_fr, size_t max_mem)
{
    capture_v4l2_streaming_t *rv = NULL;

	if (user_path) {
		rv = capture_init_v4l2_dev(verbose, user_path, user_width, user_height, user_fr, max_mem);
	} else {
		unsigned i;

		for (i = 0; i < 16; i++) {
			char path[64];

			snprintf(path, sizeof(path), "/dev/video%u", i);
			rv = capture_init_v4l2_dev(verbose, path, user_width, user_height, user_fr, max_mem);
			if (rv)
				break;
		}
	}

    return &rv->base;
}

/**
 * @}
 */
