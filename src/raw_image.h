/* arch-tag: 92fe4169-140d-42fb-ad4b-20554e9013ea */

/*  eXperience GTK engine: raw_image.h
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

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

struct _raw_image {
	guchar * pixel;
	
	gboolean has_alpha;
	
	gint width;
	gint height;
	gint rowstride;
};
typedef struct _raw_image eXperienceRawImage;

eXperienceRawImage * experience_raw_image_create (GdkPixbuf * pixbuf);
cairo_surface_t * experience_raw_image_get_surface (eXperienceRawImage * raw_image, GdkRectangle * area);
void experience_raw_image_destroy (eXperienceRawImage * raw_image);
