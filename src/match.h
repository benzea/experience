/*  eXperience GTK engine: match.h
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

#ifndef __MATCH_H
#define __MATCH_H

#include <glib.h>
#include <gtk/gtk.h>
#include "experience.h"

typedef enum {
	MATCH_NONE = 0,
	MATCH_FUNCTION        = 1 <<  0,
	MATCH_STATE           = 1 <<  1,
	MATCH_DETAIL          = 1 <<  2,
	MATCH_SHADOW          = 1 <<  5,
	MATCH_ARROW_DIRECTION = 1 <<  6,
	MATCH_ORIENTATION     = 1 <<  7,
	MATCH_GAP_SIDE        = 1 <<  8,
	MATCH_EXPANDER_STYLE  = 1 <<  9,
	MATCH_WINDOW_EDGE     = 1 << 10,
	MATCH_TEXT_DIRECTION  = 1 << 11,
	MATCH_PROGRAM_NAME    = 1 << 12,
	MATCH_CONTINUE_SIDE   = 1 << 13,
} eXperienceMatchFlags;

typedef enum {
	GAP_START      = 1 << 0,
	GAP_CENTER     = 1 << 1,
	GAP_END        = 1 << 2,
} eXperienceGapPos;

typedef enum {
	EXPERIENCE_TRUE  = 1 << 0,
	EXPERIENCE_FALSE = 1 << 1
} eXperienceBoolean;

typedef enum {
	EXPERIENCE_CONTINUE_SIDE_NONE   = 1 << 0, /* invalid */
	EXPERIENCE_CONTINUE_SIDE_LEFT   = 1 << 1,
	EXPERIENCE_CONTINUE_SIDE_RIGHT  = 1 << 2,
	EXPERIENCE_CONTINUE_SIDE_BOTH   = 1 << 3,
	EXPERIENCE_CONTINUE_SIDE_SINGLE = 1 << 4, /* single button */
} eXperienceContinueSide;

/* macro to exchange the last two bits, so that RTL locales work correctly */
#define experience_widget_continue_swap_if_rtl(widget, sides) { \
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) \
		sides = (sides & ~3) | ((sides&1)<<1) | ((sides&2)>>1); \
}

typedef struct {
	eXperienceMatchFlags flags;
	
	GList             * property_list;
	
	GtkStateType        state;
	GtkDrawingFunctions functions;
	GtkTextDirection    text_directions;
	GList             * details;
	GList             * program_names;
	GtkShadowType       shadow;
	GtkArrowType        arrow_direction;
	GtkOrientation      orientation;
	GtkPositionType     gap_side;
	GtkExpanderStyle    expander_style;
	GdkWindowEdge       window_edge;
	eXperienceContinueSide continue_side;
} eXperienceMatch;

#define POS_NONE (GTK_POS_BOTTOM + 1)

typedef struct {
	eXperienceMatchFlags flags;
	
	GObject           * widget;
	
	GtkStateType        state;
	GtkDrawingFunctions function;
	GtkTextDirection    text_direction;
	gchar             * detail;
	GtkShadowType       shadow;
	GtkArrowType        arrow_direction;
	GtkOrientation      orientation;
	GtkPositionType     gap_side;
	eXperienceGapPos    gap_pos;
	GtkExpanderStyle    expander_style;
	GdkWindowEdge       window_edge;
	eXperienceContinueSide continue_side;
} eXperienceMatchTemp;

void experience_match_init (eXperienceMatch * match);
void experience_match_finalize (eXperienceMatch * match);

void experience_match_set_states           (eXperienceMatch * match, GtkStateType state);
void experience_match_set_functions        (eXperienceMatch * match, GtkDrawingFunctions function);
void experience_match_add_detail           (eXperienceMatch * match, gchar * detail);
void experience_match_add_program_name     (eXperienceMatch * match, gchar * program_name);
void experience_match_set_shadows          (eXperienceMatch * match, GtkShadowType shadow);
void experience_match_set_arrow_directions (eXperienceMatch * match, GtkArrowType arrow_direction);
void experience_match_set_orientations     (eXperienceMatch * match, GtkOrientation orientation);
void experience_match_set_gap_sides        (eXperienceMatch * match, GtkPositionType gap_side);
void experience_match_set_expander_styles  (eXperienceMatch * match, GtkExpanderStyle expander_style);
void experience_match_set_window_edges     (eXperienceMatch * match, GdkWindowEdge window_edge);
void experience_match_set_text_directions  (eXperienceMatch * match, GtkTextDirection text_direction);
void experience_match_set_continue_sides   (eXperienceMatch * match, eXperienceContinueSide sides);

void experience_match_set_property (eXperienceMatch * match, gchar * property, GValueArray * properties);

void experience_match_inherit(eXperienceMatch * match, eXperienceMatch * from);

gboolean experience_match(eXperienceMatch * match, eXperienceMatchTemp * to);

#endif /* __MATCH_H */
