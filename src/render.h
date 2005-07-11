/* arch-tag: b9e8e988-1edb-4e10-a8e4-5b50c55e28a4 */

/*  eXperience GTK engine: render.c
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

#ifndef __RENDER_H
#define __RENDER_H

#include "group.h"
#include "image.h"
#include "gdk/gdk.h"

gboolean experience_render_group_to_cr (eXperienceGroup * group, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style);
void experience_render_pixbuf_to_window (GdkWindow * window, GdkPixbuf * experience, GdkRectangle * area, GdkRegion * clip_region, gfloat opacity);

gboolean experience_render_group (eXperienceGroup * group, GdkWindow *window, GdkRectangle * object_area, GdkRectangle * area, GtkStyle * style);

#endif /* __RENDER_H */
