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

#include <stdlib.h>
#include "vff_null.h"
#include "vff.h"

/**
 * @addtogroup vff_null
 * @{
 */

/**************************************/

/** Instance of a NULL video filter. */
typedef struct {
	video_frame_filter_t base;	/**< Base structure. */
	const unsigned char *frame;	/**< Pointer to a frame provided by @ref video_frame_filter_null_PutFrame. */
	size_t size;				/**< Size of a frame provided by @ref video_frame_filter_null_PutFrame. */
} video_frame_filter_null_t;

/**************************************/

/** @copydoc video_frame_filter_ops_t::PutFrame */
static void video_frame_filter_null_PutFrame(video_frame_filter_t *base, const unsigned char *frame, size_t size)
{
	video_frame_filter_null_t *thiz = (video_frame_filter_null_t *) base;

	thiz->frame = frame;
	thiz->size = size;
}

/** @copydoc video_frame_filter_ops_t::GetSize */
static size_t video_frame_filter_null_GetSize(video_frame_filter_t *base)
{
	video_frame_filter_null_t *thiz = (video_frame_filter_null_t *) base;

	return thiz->size;
}

/** @copydoc video_frame_filter_ops_t::Read */
static void video_frame_filter_null_Read(video_frame_filter_t *base, const unsigned char **data, size_t *size)
{
	video_frame_filter_null_t *thiz = (video_frame_filter_null_t *) base;

	*data = thiz->frame;
	*size = thiz->size;
	thiz->size = 0;
}

/** @copydoc video_frame_filter_ops_t::Destroy */
static void video_frame_filter_null_Destroy(video_frame_filter_t *base)
{
	video_frame_filter_null_t *thiz = (video_frame_filter_null_t *) base;

	free(thiz);
}

/** Operations of the NULL video filter. */
video_frame_filter_ops_t video_frame_filter_null_ops = {
	.PutFrame = video_frame_filter_null_PutFrame,
	.GetSize = video_frame_filter_null_GetSize,
	.Read = video_frame_filter_null_Read,
	.Destroy = video_frame_filter_null_Destroy,
};

/**************************************/

video_frame_filter_t *vff_null_create(void)
{
	video_frame_filter_null_t *rv = (video_frame_filter_null_t *) calloc(1, sizeof(video_frame_filter_null_t));
	rv->base.op = &video_frame_filter_null_ops;
	return &rv->base;
}

/**
 * @}
 */
