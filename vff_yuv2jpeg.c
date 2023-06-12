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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <jpeglib.h>
#include <jerror.h>
#include "vff_yuv2jpeg.h"
#include "vff.h"

/**
 * @addtogroup vff_yuv2jpeg
 * @{
 */

/**************************************/

/** Custom destination memory manager for capturing compressed JPEG from jpeglib. */
typedef struct {
	struct jpeg_destination_mgr base;	/**< Base JPEG destination structure. */
	unsigned char *result;				/**< Result buffer. */
	size_t length;						/**< Length of compressed data in @c result buffer. */
	size_t size;						/**< Total amount of space available in @c result buffer (or to be allocated if @c result is NULL). */
} jpeg_destination_mgr_mem_t;

/**
 * Starts collecting compressed frame.
 *
 * @param cinfo Memory manager used by jpeglib.
 */
static void jpeg_destination_init(j_compress_ptr cinfo)
{
	jpeg_destination_mgr_mem_t *jdst = (jpeg_destination_mgr_mem_t *) cinfo->dest;

	if (!jdst->result)
		jdst->result = malloc(jdst->size);
	jdst->base.next_output_byte = jdst->result;
	jdst->base.free_in_buffer = jdst->size;
}

/**
 * Extends space for compressed frame.
 *
 * @param cinfo Memory manager used by jpeglib.
 * @return true on success, false on memory allocation error.
 */
static boolean jpeg_destination_empty_output_buffer(j_compress_ptr cinfo)
{
	jpeg_destination_mgr_mem_t *jdst = (jpeg_destination_mgr_mem_t *) cinfo->dest;
	size_t new_size = jdst->size * 2;	/* double the size of our buffer */

	jdst->result = realloc(jdst->result, new_size);
	jdst->base.next_output_byte = jdst->result + jdst->size;
	jdst->base.free_in_buffer = new_size - jdst->size;
	jdst->size = new_size;
	return !!jdst->result;
}

/**
 * Terminates compression of a frame into JPEG.
 *
 * Takes current length as final size of a compressed frame.
 *
 * @param cinfo Memory manager used by jpeglib.
 */
static void jpeg_destination_term(j_compress_ptr cinfo)
{
	jpeg_destination_mgr_mem_t *jdst = (jpeg_destination_mgr_mem_t *) cinfo->dest;

	jdst->length = jdst->size - jdst->base.free_in_buffer;
}

/**
 * Creates custom destination memory manager for jpeglib.
 *
 * @param jdst Pointer to the memory manager structure to be initialized.
 * @return Pointer to base memory manager structure associated with @c jdst.
 */
static struct jpeg_destination_mgr *jpeg_destination_mgr_mem_create(jpeg_destination_mgr_mem_t *jdst)
{
	jdst->result = NULL;
	jdst->size = 4096;	/* start with 4 KB */
	jdst->base.init_destination = jpeg_destination_init;
	jdst->base.empty_output_buffer = jpeg_destination_empty_output_buffer;
	jdst->base.term_destination = jpeg_destination_term;
	return &jdst->base;
}

/**
 * Destroys custom destination memory manager for jpeglib.
 *
 * @param jdst Pointer to the memory manager structure initialized by @ref jpeg_destination_mgr_mem_create.
 */
static void jpeg_destination_mgr_mem_destroy(jpeg_destination_mgr_mem_t *jdst)
{
	free(jdst->result);
	jdst->result = NULL;
}

/**************************************/

/** Instance of a YUV to JPEG video filter. */
typedef struct {
	video_frame_filter_t base;			/**< Base structure. */
	unsigned bytesperline;				/**< Bytes per each line of the frame, including padding, if any. */
	struct jpeg_compress_struct cinfo;	/**< jpeglib's compress info structure. */
	struct jpeg_error_mgr jerr;			/**< jpeglib's error manager. */
	jpeg_destination_mgr_mem_t jdst;	/**< jpeglib's destination memory manager used to capture JPEG output. */
	JSAMPROW y_rows[DCTSIZE];			/**< Pointers to minimum number of Y plane rows compressed into JPEG at once (DCTSIZE). */
	JSAMPROW u_rows[DCTSIZE];			/**< Pointers to minimum number of U plane rows compressed into JPEG at once (DCTSIZE since v_samp_factor is 1). */
	JSAMPROW v_rows[DCTSIZE];			/**< Pointers to minimum number of V plane rows compressed into JPEG at once (DCTSIZE since v_samp_factor is 1). */
	JSAMPARRAY samples[3];				/**< Pointers to Y, U & V row pointers, compressed into JPEG at once. */
	const unsigned char *frame;			/**< Pointer to a frame provided by @ref video_frame_filter_yuv2jpeg_PutFrame. */
	size_t size;						/**< Size of a frame provided by @ref video_frame_filter_yuv2jpeg_PutFrame. */
} video_frame_filter_yuv2jpeg_t;

/**************************************/

/** @copydoc video_frame_filter_ops_t::PutFrame */
static void video_frame_filter_yuv2jpeg_PutFrame(video_frame_filter_t *base, const unsigned char *frame, size_t size)
{
	video_frame_filter_yuv2jpeg_t *thiz = (video_frame_filter_yuv2jpeg_t *) base;
	unsigned total_rows, c, y, x, skipbytes;

	jpeg_start_compress(&thiz->cinfo, TRUE);

	total_rows = thiz->cinfo.comp_info[0].height_in_blocks * DCTSIZE;
	skipbytes = thiz->bytesperline - thiz->cinfo.image_width * 2;
	for (c = 0; c < total_rows; c += y, frame += skipbytes) {
		for (y = 0; y < DCTSIZE; y++) {
			JSAMPROW yp = thiz->y_rows[y];
			JSAMPROW up = thiz->u_rows[y];
			JSAMPROW vp = thiz->v_rows[y];

			for (x = thiz->cinfo.image_width / 2; size >= 4 && x > 0; x--, size -= 4) {
				*yp++ = *frame++;
				*up++ = *frame++;
				*yp++ = *frame++;
				*vp++ = *frame++;
			}
		}

		jpeg_write_raw_data(&thiz->cinfo, thiz->samples, DCTSIZE);
	}

	jpeg_finish_compress(&thiz->cinfo);

	thiz->frame = thiz->jdst.result;
	thiz->size = thiz->jdst.length;
}

/** @copydoc video_frame_filter_ops_t::GetSize */
static size_t video_frame_filter_yuv2jpeg_GetSize(video_frame_filter_t *base)
{
	video_frame_filter_yuv2jpeg_t *thiz = (video_frame_filter_yuv2jpeg_t *) base;

	return thiz->size;
}

/** @copydoc video_frame_filter_ops_t::Read */
static void video_frame_filter_yuv2jpeg_Read(video_frame_filter_t *base, const unsigned char **data, size_t *size)
{
	video_frame_filter_yuv2jpeg_t *thiz = (video_frame_filter_yuv2jpeg_t *) base;

	*data = thiz->frame;
	*size = thiz->size;
	thiz->size = 0;
}

/** @copydoc video_frame_filter_ops_t::Destroy */
static void video_frame_filter_yuv2jpeg_Destroy(video_frame_filter_t *base)
{
	video_frame_filter_yuv2jpeg_t *thiz = (video_frame_filter_yuv2jpeg_t *) base;
	unsigned c, y;

	for (c = 0; c < 3; c++)
		for (y = DCTSIZE; y > 0; y--)
			free(thiz->samples[c][y - 1]);
	jpeg_destroy_compress(&thiz->cinfo);
	jpeg_destination_mgr_mem_destroy(&thiz->jdst);
	free(thiz);
}

/** Operations of the YUV to JPEG video filter. */
video_frame_filter_ops_t video_frame_filter_yuv2jpeg_ops = {
	.PutFrame = video_frame_filter_yuv2jpeg_PutFrame,
	.GetSize = video_frame_filter_yuv2jpeg_GetSize,
	.Read = video_frame_filter_yuv2jpeg_Read,
	.Destroy = video_frame_filter_yuv2jpeg_Destroy,
};

/**************************************/

video_frame_filter_t *vff_yuv2jpeg_create(unsigned width, unsigned height, unsigned bytesperline, unsigned quality)
{
	video_frame_filter_yuv2jpeg_t *rv = (video_frame_filter_yuv2jpeg_t *) calloc(1, sizeof(video_frame_filter_yuv2jpeg_t));
	unsigned c, y;

	rv->bytesperline = bytesperline;
	jpeg_create_compress(&rv->cinfo);
	rv->cinfo.err = jpeg_std_error(&rv->jerr);
	rv->cinfo.dest = jpeg_destination_mgr_mem_create(&rv->jdst);
	rv->cinfo.image_width = width;
	rv->cinfo.image_height = height;
	rv->cinfo.input_components = 3;
	rv->cinfo.in_color_space = JCS_YCbCr;
	jpeg_set_defaults(&rv->cinfo);
	rv->cinfo.raw_data_in = TRUE;
	if (quality != UINT_MAX)
		jpeg_set_quality(&rv->cinfo, quality, TRUE);
	jpeg_set_colorspace(&rv->cinfo, JCS_YCbCr);
	/* Y */
	rv->cinfo.comp_info[0].h_samp_factor = 2;
	rv->cinfo.comp_info[0].v_samp_factor = 1;
	/* U */
	rv->cinfo.comp_info[1].h_samp_factor = 1;
	rv->cinfo.comp_info[1].v_samp_factor = 1;
	/* V */
	rv->cinfo.comp_info[2].h_samp_factor = 1;
	rv->cinfo.comp_info[2].v_samp_factor = 1;
	/* pointers */
	rv->samples[0] = rv->y_rows;
	rv->samples[1] = rv->u_rows;
	rv->samples[2] = rv->v_rows;
	for (c = 0; c < 3; c++) {
		unsigned bytesperline = (width + DCTSIZE - 1) / DCTSIZE * (!c + 1) * DCTSIZE;

		for (y = DCTSIZE; y > 0; y--)
			rv->samples[c][y - 1] = malloc(bytesperline);
	}

	rv->base.op = &video_frame_filter_yuv2jpeg_ops;
	return &rv->base;
}

/**
 * @}
 */
