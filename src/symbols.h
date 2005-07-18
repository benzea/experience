/* arch-tag: ce515cb3-76d6-4214-bacf-026aa7397c97 */

/*  eXperience GTK engine: symbols.h
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

#include <cairo.h>

static struct
{
	gchar              *name;
	guint               token;
} theme_symbols[] =
{
	{ "image",				IMAGE_LITERAL },
	{ "group",				GROUP_LITERAL },
	{ "icons",				ICONS_LITERAL },
	{ "function",			FUNCTION_LITERAL },
	{ "detail",				DETAIL_LITERAL },
	{ "program",			PROGRAM_LITERAL },
	{ "has_default",		HAS_DEFAULT_LITERAL },
	{ "has_focus",			HAS_FOCUS_LITERAL },
	{ "expander_style",		EXPANDER_STYLE_LITERAL },

	{ "file",				FILE_LITERAL },
	{ "border",				BORDER_LITERAL },
	{ "repeat",				REPEAT_LITERAL },
	{ "padding",			PADDING_LITERAL },

	{ "state",				STATE_LITERAL },
	{ "arrow_direction",	ARROW_DIRECTION_LITERAL },
	{ "ypos",				Y_POS_LITERAL },
	{ "xpos",				X_POS_LITERAL },
	{ "pos",				POS_LITERAL },
	
	{ "tile",				TILE_LITERAL },
	
	{ "shadow", 			SHADOW_LITERAL },
	{ "none",			NONE_LITERAL },
	
	{ "orientation",		ORIENTATION_LITERAL },
	
	{ "gap_size",			GAP_SIZE_LITERAL },
	{ "gap_side",			GAP_SIDE_LITERAL },
	{ "line_width",			LINE_WIDTH_LITERAL },

	{ "saturation",			SATURATION_LITERAL },
	{ "pixelate",			PIXELATE_LITERAL },
	{ "brightness",			BRIGHTNESS_LITERAL },
	{ "opacity",			OPACITY_LITERAL },
	
	{ "dont_clip",			DONT_CLIP_LITERAL },
	{ "dont_draw",			DONT_DRAW_LITERAL },
	{ "dont_inherit",		DONT_INHERIT_LITERAL },
	
	{ "recolor",			RECOLOR_LITERAL },
	{ "recolor_gradient",	RECOLOR_GRADIENT_LITERAL },
	
	{ "clear_area",			CLEAR_AREA_LITERAL },
	
	{ "draw_components",	DRAW_COMPONENTS_LITERAL },
	
	{ "center",				CENTER_LITERAL },
	
	{ "draw_components",	DRAW_COMPONENTS_LITERAL },
	{ "interpolation_type",	INTERPOLATION_TYPE_LITERAL },
	
	{ "rotate",				ROTATE_LITERAL },
	{ "mirror",				MIRROR_LITERAL },
	
	{ "width",				WIDTH_LITERAL },
	{ "height", 			HEIGHT_LITERAL },
	
	{ "inner_padding",		INNER_PADDING_LITERAL },
	{ "draw_entire_only",	DRAW_ENTIRE_ONLY_LITERAL },
	
	{ "fill",				FILL_LITERAL },
	{ "color",				COLOR_LITERAL },
	
	{ "ws",					WS_LITERAL },
	{ "is",					IS_LITERAL },
	{ "px",					PX_LITERAL },	
	
	{ "rounding",			ROUNDING_LITERAL },
	{ "text_direction",		TEXT_DIRECTION_LITERAL },
	{ "property",			PROPERTY_LITERAL },
};

#define OTHER_SYMBOLS_START YY_ALWAYS_KEEP_LAST
#define OTHER_SYMBOLS_END   (OTHER_SYMBOLS_START + G_N_ELEMENTS(other_symbols) - 1)

static struct
{
	gchar              *name;
	guint               value;
	guint				identifier;
} other_symbols[] = 
{
	{ "line",			FUNCTION_LINE,					FUNCTION_IDENTIFIER },
	{ "arrow",			FUNCTION_ARROW,					FUNCTION_IDENTIFIER },
	{ "box",			FUNCTION_BOX,					FUNCTION_IDENTIFIER },
	{ "flat_box",		FUNCTION_FLAT_BOX,				FUNCTION_IDENTIFIER },
	{ "check",			FUNCTION_CHECK,					FUNCTION_IDENTIFIER },
	{ "option",			FUNCTION_OPTION,				FUNCTION_IDENTIFIER },
	{ "tab",			FUNCTION_TAB,					FUNCTION_IDENTIFIER },
	{ "extension",		FUNCTION_EXTENSION,				FUNCTION_IDENTIFIER },
	{ "focus",			FUNCTION_FOCUS,					FUNCTION_IDENTIFIER },
	{ "slider",			FUNCTION_SLIDER,				FUNCTION_IDENTIFIER },
	{ "handle",			FUNCTION_HANDLE,				FUNCTION_IDENTIFIER },
	{ "expander",		FUNCTION_EXPANDER,				FUNCTION_IDENTIFIER },
	{ "resize_grip",	FUNCTION_RESIZE_GRIP,			FUNCTION_IDENTIFIER },
	
	{ "box_gap",			FUNCTION_BOX_GAP,			FUNCTION_IDENTIFIER },
	{ "box_gap_start",		FUNCTION_BOX_GAP_START,		FUNCTION_IDENTIFIER },
	{ "box_gap_end",		FUNCTION_BOX_GAP_END,		FUNCTION_IDENTIFIER },
	{ "shadow_gap",			FUNCTION_SHADOW_GAP,		FUNCTION_IDENTIFIER },
	{ "shadow_gap_start",	FUNCTION_SHADOW_GAP_START,	FUNCTION_IDENTIFIER },
	{ "shadow_gap_end",		FUNCTION_SHADOW_GAP_END,	FUNCTION_IDENTIFIER },
	
	{ "normal",			GTK_STATE_NORMAL,				STATE_IDENTIFIER },
	{ "active",			GTK_STATE_ACTIVE,				STATE_IDENTIFIER },
	{ "prelight",		GTK_STATE_PRELIGHT,				STATE_IDENTIFIER },
	{ "selected",		GTK_STATE_SELECTED,				STATE_IDENTIFIER },
	{ "insensitive",	GTK_STATE_INSENSITIVE,			STATE_IDENTIFIER },
	
	{ "in",				GTK_SHADOW_IN,					SHADOW_IDENTIFIER },
	{ "out",			GTK_SHADOW_OUT,					SHADOW_IDENTIFIER },
	{ "etched_in",		GTK_SHADOW_ETCHED_IN,			SHADOW_IDENTIFIER },
	{ "etched_out",		GTK_SHADOW_ETCHED_OUT,			SHADOW_IDENTIFIER },
	
	{ "up",				GTK_ARROW_UP,					ARROW_DIRECTION_IDENTIFIER },
	{ "down",			GTK_ARROW_DOWN,					ARROW_DIRECTION_IDENTIFIER },
	
	{ "horizontal",		GTK_ORIENTATION_HORIZONTAL,		ORIENTATION_IDENTIFIER },
	{ "vertical",		GTK_ORIENTATION_VERTICAL,		ORIENTATION_IDENTIFIER },
	
	{ "top",			GTK_POS_TOP,					POSITION_IDENTIFIER },
	{ "bottom",			GTK_POS_BOTTOM,					POSITION_IDENTIFIER },
	
	{ "expanded",		GTK_EXPANDER_EXPANDED,			EXPANDER_STYLE_IDENTIFIER },
	{ "collapsed",		GTK_EXPANDER_COLLAPSED,			EXPANDER_STYLE_IDENTIFIER },
	{ "semi_expanded",	GTK_EXPANDER_SEMI_EXPANDED,		EXPANDER_STYLE_IDENTIFIER },
	{ "semi_collapsed",	GTK_EXPANDER_SEMI_COLLAPSED,	EXPANDER_STYLE_IDENTIFIER },
	
	{ "north_west",		GDK_WINDOW_EDGE_NORTH_WEST,		WINDOW_EDGE_IDENTIFIER },
	{ "north",			GDK_WINDOW_EDGE_NORTH,			WINDOW_EDGE_IDENTIFIER },
	{ "north_east",		GDK_WINDOW_EDGE_NORTH_EAST,		WINDOW_EDGE_IDENTIFIER },
	{ "west",			GDK_WINDOW_EDGE_WEST,			WINDOW_EDGE_IDENTIFIER },
	{ "east",			GDK_WINDOW_EDGE_EAST,			WINDOW_EDGE_IDENTIFIER },
	{ "south_west",		GDK_WINDOW_EDGE_SOUTH_WEST,		WINDOW_EDGE_IDENTIFIER },
	{ "south",			GDK_WINDOW_EDGE_SOUTH,			WINDOW_EDGE_IDENTIFIER },
	{ "south_east",		GDK_WINDOW_EDGE_SOUTH_EAST,		WINDOW_EDGE_IDENTIFIER },
	
	{ "true",			TRUE,							BOOLEAN },
	{ "false",			FALSE,							BOOLEAN },
	{ "left",			0,								LEFT_RIGHT},
	{ "right",			1,								LEFT_RIGHT},

	{ "bg",					STYLE_COLOR_BG,				STYLE_COLOR_IDENTIFIER },
	{ "fg",					STYLE_COLOR_FG,				STYLE_COLOR_IDENTIFIER },
	{ "text",				STYLE_COLOR_TEXT,			STYLE_COLOR_IDENTIFIER },
	{ "base",				STYLE_COLOR_BASE,			STYLE_COLOR_IDENTIFIER },
	
	{ "all",			COMPONENT_ALL,				DRAW_COMPONENTS_IDENTIFIER },
	
	/* whoops, now cairo can't do all of these ... */
	{ "nearest",			CAIRO_FILTER_NEAREST,		INTERPOLATION_TYPE_IDENTIFIER },
	{ "bilinear",			CAIRO_FILTER_BILINEAR,		INTERPOLATION_TYPE_IDENTIFIER },
	{ "gaussian",			CAIRO_FILTER_GAUSSIAN,		INTERPOLATION_TYPE_IDENTIFIER },
	
	{ "fast",				CAIRO_FILTER_FAST,			INTERPOLATION_TYPE_IDENTIFIER },
	{ "good",				CAIRO_FILTER_GOOD,			INTERPOLATION_TYPE_IDENTIFIER },
	{ "best",				CAIRO_FILTER_BEST,			INTERPOLATION_TYPE_IDENTIFIER },
	
	/* deprecated, fallback to bilinear */
	{ "tiles",				CAIRO_FILTER_BILINEAR,		INTERPOLATION_TYPE_IDENTIFIER },
	{ "huge",				CAIRO_FILTER_GAUSSIAN,		INTERPOLATION_TYPE_IDENTIFIER },
	
	{ "ceil",				ROUND_CEIL,					ROUNDING_TYPE_IDENTIFIER },
	{ "floor",				ROUND_FLOOR,				ROUNDING_TYPE_IDENTIFIER },
	{ "to_zero",			ROUND_TO_ZERO,				ROUNDING_TYPE_IDENTIFIER },
	{ "math",				ROUND_NORMAL,				ROUNDING_TYPE_IDENTIFIER },
	
	{ "cw",					ROTATE_CW,					ROTATE_IDENTIFIER },
	{ "ccw",				ROTATE_CCW,					ROTATE_IDENTIFIER },
	{ "around",				ROTATE_AROUND,				ROTATE_IDENTIFIER },
	
	{ "ltr",				GTK_TEXT_DIR_LTR,				TEXT_DIRECTION_IDENTIFIER },
	{ "rtl",				GTK_TEXT_DIR_RTL,				TEXT_DIRECTION_IDENTIFIER },
};

