/*  eXperience GTK engine: dynamic_color.h
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

#ifndef __DYNAMIC_COLOR_H
#define __DYNAMIC_COLOR_H

#include "gdk/gdk.h"
#include "gtk/gtk.h"

typedef enum {
	STYLE_COLOR_FG,
	STYLE_COLOR_BG,
	STYLE_COLOR_TEXT,
	STYLE_COLOR_BASE
} eXperienceStyleColor;

typedef enum {
	GET_STYLE_COLOR,
	GET_SET_COLOR
} eXperienceGetColor;

typedef struct  {
	eXperienceGetColor source;
	
	GdkColor output_color;
	struct {
		eXperienceStyleColor color_array;
		GtkStateType state;
	} output_style_color;
} eXperienceDynamicColor;

void experience_dynamic_color_update (eXperienceDynamicColor *  color, GtkStyle * style);



#endif /*__DYNAMIC_COLOR_H*/
