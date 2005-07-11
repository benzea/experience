/* arch-tag: ae037c53-13be-4e44-881f-03cc88186bd0 */

/*  eXperience GTK engine: image.h
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

#ifndef __IMAGE_H
#define __IMAGE_H

#include <gdk/gdk.h>
#include "drawable.h"

eXperienceDrawableClass * experience_image_class;

typedef enum
{
	COMPONENT_NONE       = 0,
	COMPONENT_NORTH      = 1 << GDK_WINDOW_EDGE_NORTH,
	COMPONENT_NORTH_EAST = 1 << GDK_WINDOW_EDGE_NORTH_EAST,
	COMPONENT_NORTH_WEST = 1 << GDK_WINDOW_EDGE_NORTH_WEST,
	COMPONENT_SOUTH      = 1 << GDK_WINDOW_EDGE_SOUTH,
	COMPONENT_SOUTH_EAST = 1 << GDK_WINDOW_EDGE_SOUTH_EAST,
	COMPONENT_SOUTH_WEST = 1 << GDK_WINDOW_EDGE_SOUTH_WEST,
	COMPONENT_EAST       = 1 << GDK_WINDOW_EDGE_EAST,
	COMPONENT_WEST       = 1 << GDK_WINDOW_EDGE_WEST,
	COMPONENT_CENTER     = 1 << (GDK_WINDOW_EDGE_SOUTH_EAST + 1),
	COMPONENT_ALL        =   COMPONENT_NORTH | COMPONENT_NORTH_EAST | COMPONENT_NORTH_WEST | COMPONENT_SOUTH
	                       | COMPONENT_SOUTH_EAST | COMPONENT_SOUTH_WEST | COMPONENT_EAST | COMPONENT_WEST
	                       | COMPONENT_CENTER
} eXperienceComponents;

#define COMPONENT_BORDER_TOP     (COMPONENT_NORTH | COMPONENT_NORTH_EAST | COMPONENT_NORTH_WEST)
#define COMPONENT_BORDER_BOTTOM  (COMPONENT_SOUTH | COMPONENT_SOUTH_EAST | COMPONENT_SOUTH_WEST)
#define COMPONENT_BORDER_LEFT    (COMPONENT_EAST  | COMPONENT_NORTH_EAST | COMPONENT_SOUTH_EAST)
#define COMPONENT_BORDER_RIGHT   (COMPONENT_WEST  | COMPONENT_NORTH_WEST | COMPONENT_SOUTH_WEST)
#define COMPONENT_BORDER    (COMPONENT_NORTH | COMPONENT_NORTH_EAST | COMPONENT_NORTH_WEST | COMPONENT_SOUTH \
                           | COMPONENT_SOUTH_EAST | COMPONENT_SOUTH_WEST | COMPONENT_EAST | COMPONENT_WEST)

typedef enum {
	OPTION_FILE        = 1 << 0,
	OPTION_INTERP_TYPE = 1 << 1,
	OPTION_DRAW_COMPONENTS = 1 << 2,
	OPTION_BORDER      = 1 << 3,
} eXperienceImageOptionList;

struct _image {
	eXperienceDrawable drawable;
	
	eXperienceImageOptionList options_set;
	
	gchar * file;
	cairo_filter_t interp_type;
	
	eXperienceBorder border;
	eXperienceComponents draw_components;
};
typedef struct _image eXperienceImage;

void experience_image_set_file (eXperienceImage * image, gchar * filename);
void experience_image_set_draw_components (eXperienceImage * image, eXperienceComponents draw_components);
void experience_image_set_interp_type (eXperienceImage * image, GdkInterpType interp_type);
void experience_image_set_border (eXperienceImage * image, guint left, guint right, guint top, guint bottom);

void experience_image_init_class (void);

#endif /* __IMAGE_H */
