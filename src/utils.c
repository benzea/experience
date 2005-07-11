/* arch-tag: 2168ecf0-9abc-45d2-ad33-d396361cca1d */

/*  eXperience GTK engine: utils.c
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
 *
 *
 *  Thanks to the Magic Chicken engine, where I found out, how I can modify
 *  the pixbufs for filtering.
 *    Magic Chicken author:
 *      James M. Cape <jcape@ignore-your.tv>
 *
*/

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "experience.h"

gint
experience_round (eXperienceRoundingMethod method, gfloat value)
{
	switch (method) {
		case ROUND_CEIL:
			return (gint) ceil (value);
			break;
		case ROUND_FLOOR:
			return (gint) floor (value);
			break;
		case ROUND_NORMAL:
			return (gint) floor (value + 0.5);
			break;
		case ROUND_TO_ZERO:
				return (gint) value;
	}
	g_return_val_if_reached (0);
}

/* this function is now pretty usesless ... */
GdkPixbuf *
experience_gdk_pixbuf_scale_or_ref (GdkPixbuf * pixbuf, GdkRectangle * src, gint dest_width, gint dest_height, GdkInterpType interp_type)
{
	static GdkPixbuf * subpixbuf = NULL; /* we reuse this ... so it needs to be garanteed that nobody keeps it around ... */
	GdkPixbuf * result;
	
	if ((src->x == 0) && (src->y == 0) && (src->width == dest_width) && (src->height == dest_height) &&
	    (dest_width  == gdk_pixbuf_get_width (pixbuf)) && (dest_height == gdk_pixbuf_get_height (pixbuf))) {
		/* we can just use the original pixbuf */
		g_object_ref (pixbuf);
		return pixbuf;
	} else {
		subpixbuf = gdk_pixbuf_new_subpixbuf (pixbuf, src->x, src->y, src->width, src->height);
		result = experience_gdk_pixbuf_scale_simple_or_ref (subpixbuf, dest_width, dest_height, interp_type);
		g_object_unref (subpixbuf);
		return result;
	}
}

GdkPixbuf *
experience_gdk_pixbuf_scale_simple_or_ref (GdkPixbuf * pixbuf, gint width, gint height, GdkInterpType interp_type)
{
	if ((width  == gdk_pixbuf_get_width (pixbuf)) && (height == gdk_pixbuf_get_height (pixbuf))) {
		g_object_ref (pixbuf);
		return pixbuf;
	} else {
		return gdk_pixbuf_scale_simple (pixbuf, width, height, interp_type);
	}
}

/*----------*/

GdkPixbuf *
experience_rotate (GdkPixbuf * pixbuf, eXperienceRotate rotation)
{
	GdkPixbuf * target = NULL;
	guint x, y, rowstride, height, width;
	guint target_rowstride, n_channels;
	guchar *row, *pixel, *target_row, *target_pixel;
	
	g_return_val_if_fail (pixbuf != NULL, NULL);
	g_return_val_if_fail (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8, NULL); /* we can only handle one byte per sample */
	g_return_val_if_fail (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB, NULL);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_return_val_if_fail ((n_channels == 3) || (n_channels == 4), NULL); /*RGB || RGBA*/
	
	width  = gdk_pixbuf_get_width  (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	
	target = pixbuf;
	
	if (rotation == ROTATE_AROUND) {
		target = gdk_pixbuf_new (GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha (pixbuf), 8,
		                         width, height);
		
		target_rowstride = gdk_pixbuf_get_rowstride (target);
		
		row = gdk_pixbuf_get_pixels (pixbuf);
		
		target_row = gdk_pixbuf_get_pixels (target);
		target_row = target_row + (height - 1) * target_rowstride;
		
		for (y = 0; y < height; y++) {
			pixel = row;
			target_pixel = target_row + (width - 1) * n_channels;
			
			for (x = 0; x < width; x++) {
				g_memmove (target_pixel, pixel, n_channels);
				
				pixel += n_channels;
				target_pixel -= n_channels;
			}
			row += rowstride;
			target_row -= target_rowstride;
		}
		
		g_object_unref (pixbuf);
	} else if (rotation == ROTATE_CW) {
		target = gdk_pixbuf_new (GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha (pixbuf), 8,
		                         height, width);

		target_rowstride = gdk_pixbuf_get_rowstride (target);
		
		row = gdk_pixbuf_get_pixels (pixbuf);
		
		target_row = gdk_pixbuf_get_pixels (target);
		target_row = target_row + (height - 1) * n_channels;
		
		for (y = 0; y < height; y++) {
			pixel = row;
			target_pixel = target_row;
			
			for (x = 0; x < width; x++) {
				g_memmove (target_pixel, pixel, n_channels);
				
				pixel += n_channels;
				target_pixel += target_rowstride;
			}
			row += rowstride;
			target_row -= n_channels;
		}
		
		g_object_unref (pixbuf);
	} else if (rotation == ROTATE_CCW) {
		target = gdk_pixbuf_new (GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha (pixbuf), 8,
		                         gdk_pixbuf_get_height (pixbuf), gdk_pixbuf_get_width (pixbuf));

		target_rowstride = gdk_pixbuf_get_rowstride (target);
		
		row = gdk_pixbuf_get_pixels (pixbuf);
		
		target_row = gdk_pixbuf_get_pixels (target);
		target_row = target_row + (width - 1) * target_rowstride;
		
		for (y = 0; y < height; y++) {
			pixel = row;
			target_pixel = target_row;
			
			for (x = 0; x < width; x++) {
				g_memmove (target_pixel, pixel, n_channels);
				
				pixel += n_channels;
				target_pixel -= target_rowstride;
			}
			row += rowstride;
			target_row += n_channels;
		}
		
		g_object_unref (pixbuf);
	}
	
	return target;
}

GdkPixbuf *
experience_mirror (GdkPixbuf * pixbuf, eXperienceRotate rotation)
{
	GdkPixbuf * source, * dest;
	guint x, y, rowstride, height, width;
	guint dest_rowstride, n_channels;
	guchar *row, *pixel, *dest_row, *dest_pixel;
	
	g_return_val_if_fail (pixbuf != NULL, NULL);
	g_return_val_if_fail (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8, NULL); /* we can only handle one byte per sample */
	g_return_val_if_fail (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB, NULL);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_return_val_if_fail ((n_channels == 3) || (n_channels == 4), NULL); /*RGB || RGBA*/
	
	source = pixbuf;
	
	width  = gdk_pixbuf_get_width  (source);
	height = gdk_pixbuf_get_height (source);
	
	if (rotation & ORIENTATION_HORIZONTAL) {
		rowstride = gdk_pixbuf_get_rowstride (source);
		
		dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha (source), 8,
		                         width, height);
		dest_rowstride = gdk_pixbuf_get_rowstride (source);
		
		row = gdk_pixbuf_get_pixels (source);
		dest_row = gdk_pixbuf_get_pixels (dest);
		
		for (y = 0; y < height; y++) {
			pixel = row;
			dest_pixel = dest_row + (width - 1) * n_channels;
			
			for (x = 0; x < width; x++) {
				g_memmove (dest_pixel, pixel, n_channels);
				
				pixel += n_channels;
				dest_pixel -= n_channels;
			}
			
			row += rowstride;
			dest_row += dest_rowstride;
		}
		
		g_object_unref (source);
		source = dest;
	}
	
	if (rotation & ORIENTATION_VERTICAL) {
		rowstride = gdk_pixbuf_get_rowstride (source);
		
		dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha (source), 8,
		                       width, height);
		dest_rowstride = gdk_pixbuf_get_rowstride (source);
		
		row = gdk_pixbuf_get_pixels (source);
		dest_row = gdk_pixbuf_get_pixels (dest);
		dest_row += (height - 1) * dest_rowstride;
		
		for (y = 0; y < height; y++) {
			pixel = row;
			dest_pixel = dest_row;
			
			for (x = 0; x < width; x++) {
				g_memmove (dest_pixel, pixel, n_channels);
				
				pixel += n_channels;
				dest_pixel += n_channels;
			}
			
			row += rowstride;
			dest_row -= dest_rowstride;
		}
		
		g_object_unref (source);
		source = dest;
	}
	
	return source;
}

GdkPixbuf *
experience_change_pixbuf_opacity (GdkPixbuf * pixbuf, gfloat opacity)
{
	GdkPixbuf * target;
	gint width, height, rowstride, n_channels;
	guchar *row, *pixel;
	gint x, y;
	
	g_return_val_if_fail (pixbuf != NULL, NULL);
	g_return_val_if_fail (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8, NULL); /* we can only handle one byte per sample */
	g_return_val_if_fail (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB, NULL);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_return_val_if_fail ((n_channels == 3) || (n_channels == 4), NULL); /*RGB || RGBA*/
	
	opacity = CLAMP (opacity, 0, 1);
	
	if (opacity == 1)
		return pixbuf;
	
	if (!gdk_pixbuf_get_has_alpha (pixbuf)) {
		target = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
		g_object_unref (pixbuf);
		n_channels = gdk_pixbuf_get_n_channels (target);
	} else {
		target = pixbuf;
	}

	if (opacity == 0) {
		gdk_pixbuf_fill (pixbuf, 0x00000000);
		return pixbuf;
	}
	
	width  = gdk_pixbuf_get_width (target);
	height = gdk_pixbuf_get_height (target);
	
	rowstride = gdk_pixbuf_get_rowstride (target);
	row = gdk_pixbuf_get_pixels (target);
	
	for (y = 0; y < height; y++) {
		pixel = row;
		for (x = 0; x < width; x++) {
			pixel[ALPHA_OFFSET] = pixel[ALPHA_OFFSET] * opacity;
			
			pixel += n_channels;
		}
		row += rowstride;
	}
	
	return target;
}

void
experience_set_pixbuf_brightness (GdkPixbuf * pixbuf, gfloat brightness)
{
	gint width, height, rowstride, n_channels;
	guchar *row, *pixel;
	gint x, y;
	
	g_return_if_fail (pixbuf != NULL);
	g_return_if_fail (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8); /* we can only handle one byte per sample */
	g_return_if_fail (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_return_if_fail ((n_channels == 3) || (n_channels == 4)); /*RGB || RGBA*/
	
	brightness = CLAMP (brightness, -1, 1);
	
	if (brightness == 0)
		return;
	
	width  = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	row = gdk_pixbuf_get_pixels (pixbuf);
	
	for (y = 0; y < height; y++) {
		pixel = row;
		for (x = 0; x < width; x++) {
			pixel[RED_OFFSET]   = CLAMP_UCHAR (pixel[RED_OFFSET] + brightness * 255);
			pixel[GREEN_OFFSET] = CLAMP_UCHAR (pixel[GREEN_OFFSET] + brightness * 255);
			pixel[BLUE_OFFSET]  = CLAMP_UCHAR (pixel[BLUE_OFFSET] + brightness * 255);
			
			pixel += n_channels;
		}
		row += rowstride;
	}
	
	return;
}

/*###########*/

guint
hash_mem (gpointer start, guint count, guint init)
{
	guint result = init;
	const char * p;
	
	for (p = start; (gpointer) p < (gpointer) (start + count); p++) {
		result = (result << 5) - result + *p;
	}
	
	return result;
}
