/* arch-tag: 81ccb22b-9b50-4c3a-8220-e18079a5f058 */

/*  eXperience GTK engine: image_loading.h
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

#ifndef __IMAGE_LOADING_H
#define __IMAGE_LOADING_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "filter.h"

typedef struct {
	gchar * file;
	
	eXperienceFilter filter;
} eXperienceCacheImage;

GdkPixbuf * experience_get_image_pixbuf (eXperienceCacheImage * image, GtkStyle * style);
void experience_image_cache_destroy (void);

#endif /* __IMAGE_LOADING_H */
