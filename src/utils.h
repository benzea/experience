/* arch-tag: 4fbc3b21-07fd-468a-a1b3-d8b101b53ffe */

/*  eXperience GTK engine: utils.h
 *  
 *  Copyright (C) 2004-2005  Benjamin Berg <benjamin@sipsolutions.net>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __UTILS_H
#define __UTILS_H

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "experience.h"

#define PIXEL_SIZE_WITHOUT_ALPHA 3
#define PIXEL_SIZE_WITH_ALPHA 4

#define RED_OFFSET 0
#define GREEN_OFFSET 1
#define BLUE_OFFSET 2
#define ALPHA_OFFSET 3

/* Macro used to ensure valid values */
#define CLAMP_UCHAR(value) CLAMP(value,0,255)
                                  
typedef enum {
	ROTATE_NONE   = 0,
	ROTATE_CW     = 1,
	ROTATE_CCW    = 3,
	ROTATE_AROUND = 2
} eXperienceRotate;

typedef enum {
	ROUND_CEIL,
	ROUND_FLOOR,
	ROUND_NORMAL,
	ROUND_TO_ZERO
} eXperienceRoundingMethod;

typedef enum {
	ORIENTATION_HORIZONTAL = 1 << 0,
	ORIENTATION_VERTICAL   = 1 << 1
} eXperienceOrientation;

gint experience_round (eXperienceRoundingMethod method, gfloat value);

void experience_rectangle_union (GdkRectangle * src1, GdkRectangle * src2, GdkRectangle * dest);

/* The following functions exist to save time used for creating GdkPixbufs */
GdkPixbuf * experience_gdk_pixbuf_scale_or_ref (GdkPixbuf * pixbuf, GdkRectangle * src, gint dest_width, gint dest_height, GdkInterpType interp_type);
GdkPixbuf * experience_gdk_pixbuf_scale_simple_or_ref (GdkPixbuf * pixbuf, gint width, gint height, GdkInterpType interp_type);

GdkPixbuf * experience_change_pixbuf_opacity (GdkPixbuf * pixbuf, gfloat opacity);
GdkPixbuf * experience_rotate (GdkPixbuf * pixbuf, eXperienceRotate rotation);
GdkPixbuf * experience_mirror (GdkPixbuf * pixbuf, eXperienceRotate rotation);
void experience_set_pixbuf_brightness (GdkPixbuf * pixbuf, gfloat brightness);

void experience_gdk_rectangle_union (GdkRectangle * src1, GdkRectangle * src2, GdkRectangle * dest);

typedef GdkPixbuf * (*eXperience_get_image_from_info) (gpointer info);

void experience_pixbuf_composite (GdkPixbuf * dest, GdkRectangle * dst_area, GdkRectangle * clip_area, GdkRegion * dirty_region, eXperience_get_image_from_info get_image, gpointer info);

guint hash_mem (gpointer start, guint count, guint init);

#endif /* __UTILS_H */
