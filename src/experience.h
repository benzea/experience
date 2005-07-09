/* arch-tag: 63c2011d-2a98-476b-ae05-e619852ec934 */

/*  eXperience GTK engine: experience.h
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

#ifndef __EXPERIENCE_H
#define __EXPERIENCE_H

#include <gdk/gdk.h>
#include <glib.h>

#define GDK_COLOR_MAX 65535

#define experience_warning(...) { \
	if (!parser_error) { \
		g_printerr("eXperience engine: "); \
		g_printerr(__VA_ARGS__); \
	} \
} 

#define CLAMP_COLOR(color) CLAMP(color, 0, GDK_COLOR_MAX)

enum {
	NONE,
	PARSING,
	RUNNING
} experience_engine_state;

GQuark experience_g_quark;

gboolean print_widget_path;
gboolean warn_invalid_gtype;
gboolean parser_error;

/* All GTK drawing functions: */
typedef enum {
	FUNCTION_NONE 				= 0,
	FUNCTION_LINE				= 1 <<  0,
	FUNCTION_SHADOW				= 1 <<  1,
	FUNCTION_ARROW				= 1 <<  2,
	FUNCTION_BOX				= 1 <<  3,
	FUNCTION_FLAT_BOX			= 1 <<  4,
	FUNCTION_CHECK				= 1 <<  5,
	FUNCTION_OPTION				= 1 <<  6,
	FUNCTION_TAB				= 1 <<  7,
	FUNCTION_EXTENSION			= 1 <<  8,
	FUNCTION_FOCUS				= 1 <<  9,
	FUNCTION_SLIDER				= 1 << 10,
	FUNCTION_HANDLE				= 1 << 11,
	FUNCTION_EXPANDER			= 1 << 12,
	FUNCTION_RESIZE_GRIP		= 1 << 13,
	FUNCTION_BOX_GAP_START		= 1 << 14,
	FUNCTION_BOX_GAP			= 1 << 15,
	FUNCTION_BOX_GAP_END		= 1 << 16,
	FUNCTION_SHADOW_GAP_START	= 1 << 17,
	FUNCTION_SHADOW_GAP			= 1 << 18,
	FUNCTION_SHADOW_GAP_END		= 1 << 19,
} GtkDrawingFunctions;


#endif /* __EXPERIENCE_H */
