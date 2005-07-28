/* arch-tag: 6717c565-4152-4d88-b7be-c04f04df76f0 */

/*  eXperience GTK engine: raw_image.c
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

#include "raw_image.h"

/* a lot of this is copied from gtk+ gdk/gdkcairo.c */
eXperienceRawImage *
experience_raw_image_create (GdkPixbuf * pixbuf)
{
	eXperienceRawImage * result;
	guchar *gdk_pixels = gdk_pixbuf_get_pixels (pixbuf);
	int gdk_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	int width, height;
	guchar * pixels;
	cairo_surface_t *surface;
	int j;
	
	result = g_new0 (eXperienceRawImage, 1);
	
	result->width  = gdk_pixbuf_get_width  (pixbuf);
	result->height = gdk_pixbuf_get_height (pixbuf);
	width  = result->width;
	height = result->height;
	
	result->rowstride = 4 * width;
	
	if (gdk_pixbuf_get_has_alpha (pixbuf)) {
		result->format = CAIRO_FORMAT_ARGB32;
	} else {
		result->format = CAIRO_FORMAT_RGB24;
	}
	
	pixels = g_malloc (4 * width * height);
	result->pixel = pixels;
	surface = cairo_image_surface_create_for_data ((unsigned char *)pixels,
	                                               result->format,
	                                               width, height, result->rowstride);
	
	for (j = height; j; j--)
	{
		guchar *p = gdk_pixels;
		guchar *q = pixels;

		if (n_channels == 3)
		{
			guchar *end = p + 3 * width;
		
			while (p < end)
			{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
				q[0] = p[2];
				q[1] = p[1];
				q[2] = p[0];
#else	  
				q[1] = p[0];
				q[2] = p[1];
				q[3] = p[2];
#endif
				p += 3;
				q += 4;
			}
		}
		else
		{
			guchar *end = p + 4 * width;
			guint t1,t2,t3;
			
#define MULT(d,c,a,t) G_STMT_START { t = c * a; d = ((t >> 8) + t) >> 8; } G_STMT_END
			
			while (p < end)
			{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
				MULT(q[0], p[2], p[3], t1);
				MULT(q[1], p[1], p[3], t2);
				MULT(q[2], p[0], p[3], t3);
				q[3] = p[3];
#else	  
				q[0] = p[3];
				MULT(q[1], p[0], p[3], t1);
				MULT(q[2], p[1], p[3], t2);
				MULT(q[3], p[2], p[3], t3);
#endif
	      
				p += 4;
				q += 4;
			}
		
#undef MULT
		}

		gdk_pixels += gdk_rowstride;
		pixels += 4 * width;
	}
	
	return result;
}

cairo_surface_t *
experience_raw_image_get_surface (eXperienceRawImage * raw_image, GdkRectangle * area)
{
	cairo_surface_t * result = NULL;
	
	g_assert (area->x >= 0);
	g_assert (area->y >= 0);
	g_assert (area->width  + area->x <= raw_image->width);
	g_assert (area->height + area->y <= raw_image->height);
	
	if (raw_image != NULL) {
		result = cairo_image_surface_create_for_data ((unsigned char *)raw_image->pixel + area->x * 4 + area->y * raw_image->rowstride,
		                                              raw_image->format,
		                                              area->width, area->height, raw_image->rowstride);
	} else {
		g_assert_not_reached ();
	}
	
	return result;
}

void
experience_raw_image_destroy (eXperienceRawImage * raw_image)
{
	if (raw_image != NULL) {
		if (raw_image->pixel != NULL) {
			g_free (raw_image->pixel);
		} else {
			g_assert_not_reached ();
		}
		g_free (raw_image);
	} else {
		g_assert_not_reached ();
	}
}
