/*  eXperience GTK engine: drawing_functions.c
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

#include <glib.h>
#include <gtk/gtk.h>
#include "experience.h"
#include "drawing_functions.h"
#include "render.h"
#include "rcstyle.h"
#include "style.h"
#include "group.h"

#define REPLACE_WITH_LINE_WIDTH -2

typedef enum {
	CONTINUE_INVALID = 0,
        CONTINUE_LEFT    = 1 << 0,
        CONTINUE_RIGHT   = 1 << 1,
        CONTINUE_NONE    = 1 << 2,
} tmpContinueSide;

/* macro to exchange the last two bits, so that RTL locales work correctly */
#define experience_widget_continue_swap_if_rtl(widget, sides) { \
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) \
		sides = (sides & ~3) | ((sides&1)<<1) | ((sides&2)>>1); \
}

typedef enum {
	GROUPABLE_NO,
	GROUPABLE_YES,
	GROUPABLE_SKIP,
} is_groupable_result;

static is_groupable_result
is_groupable_widget (GtkWidget * widget)
{
	gboolean visible;
	is_groupable_result result = GROUPABLE_YES;
	
	if (!widget)
		return GROUPABLE_NO;
	if (!GTK_IS_WIDGET (widget))
		return GROUPABLE_NO;
	
	/* always return FALSE if it is a seperator */
	if (GTK_IS_SEPARATOR_TOOL_ITEM (widget))
		return GROUPABLE_NO;
	
	/* skip not visible objects that can be chained */
	g_object_get (G_OBJECT (widget), "visible", &visible, NULL);
	
	if (!visible)
		result = GROUPABLE_SKIP;
	
	if (GTK_IS_MENU_TOOL_BUTTON (widget)) {
		return result;
	}
	if (GTK_IS_TOOL_BUTTON (widget)) {
		return result;
	}
	if (GTK_IS_BUTTON (widget)) {
		return result;
	}
	return GROUPABLE_NO;
}

/* This code has problems with galeon. This is because galeon always uses
 * something GtkMenuToolItem like. It just sets the other arrow_button invisible. */
static tmpContinueSide
get_button_location_parent (GtkWidget * widget)
{
	tmpContinueSide result = CONTINUE_INVALID;
	GtkBox * box;
	GtkToolbar * toolbar;
	gint widget_pos, item;
	GList * list;
	GList * widget_list_item, * list_item;
	is_groupable_result groupable;
	
	/* check whether the parent is a GtkContainer
	 * This is the case for both GtkToolItem and GtkToolbar */
	if (!widget->parent)
		return CONTINUE_INVALID;
	
	/* there are multiple possibilities:
	 *  1. This is a GtkToolbarItem, this is just a container for one widget. So walk up the tree.
	 *  2. This is a GtkToolbar. Just check both sides then.
	 *  3. We have got a GtkBox. This means we need to check both sides. Also we might need to walk up in the tree. */
	
	if (GTK_IS_TOOL_ITEM (widget->parent)) {
		/* walk up the tree */
		result |= get_button_location_parent (widget->parent);
	} else
	
	/* first the toolbar case*/
	if (GTK_IS_TOOLBAR (widget->parent) && GTK_IS_TOOL_ITEM (widget)) {
		result = CONTINUE_NONE;
		
		toolbar = GTK_TOOLBAR (widget->parent);
		widget_pos = gtk_toolbar_get_item_index (toolbar, GTK_TOOL_ITEM (widget)); /* get the position */
		
		groupable = GROUPABLE_NO;
		item = widget_pos - 1;
		while ((groupable = is_groupable_widget ((GtkWidget*) gtk_toolbar_get_nth_item (toolbar, item))) == GROUPABLE_SKIP) {
			item--;
		}
		if (groupable == GROUPABLE_YES) {
			result |= CONTINUE_LEFT;
		}
		
		groupable = GROUPABLE_NO;
		item = widget_pos + 1;
		while ((groupable = is_groupable_widget ((GtkWidget*) gtk_toolbar_get_nth_item (toolbar, item))) == GROUPABLE_SKIP) {
			item++;
		}
		if (groupable == GROUPABLE_YES) {
			result |= CONTINUE_RIGHT;
		}
		/* mirror if RTL */
		experience_widget_continue_swap_if_rtl (widget->parent, result);
	} else
	
	if (GTK_IS_HBOX (widget->parent)) {
		/* there are two again possibilities:
		 *  1. Just a Box
		 *  2. A GtkToolMenuButton
		 * For now this difference doesn't matter ... it decides just if we will walk up the tree */
		
		box = GTK_BOX (widget->parent);
		
		if (gtk_box_get_spacing (box) == 0) {
			result = CONTINUE_NONE;
			
			/* only works if spacing is 0 */
			
			/* this is pretty inefficient, since the list is created just for us, but whatever*/
			list = gtk_container_get_children (GTK_CONTAINER (box));
			
			/* get the widget */
			widget_list_item = g_list_find (list, widget);
			
			if (!widget_list_item) {
				experience_warning ("something went wrong, object not in the box?\n");
				g_list_free (list);
				return CONTINUE_NONE;
			}
			groupable = GROUPABLE_NO;
			list_item = g_list_previous (widget_list_item);
			while (list_item && ((groupable = is_groupable_widget ((GtkWidget*) list_item->data)) == GROUPABLE_SKIP)) {
				list_item = g_list_previous (list_item);
			}
			if (groupable == GROUPABLE_YES) {
				result |= CONTINUE_LEFT;
			}
			
			groupable = GROUPABLE_NO;
			list_item = g_list_next (widget_list_item);
			while (list_item && ((groupable = is_groupable_widget ((GtkWidget*) list_item->data)) == GROUPABLE_SKIP)) {
				list_item = g_list_next (list_item);
			}
			if (groupable == GROUPABLE_YES) {
				result |= CONTINUE_RIGHT;
			}
			
			/* mirror if RTL */
			experience_widget_continue_swap_if_rtl (widget->parent, result);
			
			g_list_free (list);
		}
		
		/* OK, we got everything so far. So now we just need to walk up, if the parent
		 * of the box is a GtkToolItem (GtkMenuToolItem, GulToolbutton, etc.) */
		if (GTK_WIDGET (box)->parent && GTK_IS_TOOL_ITEM (GTK_WIDGET (box)->parent)) {
			result |= get_button_location_parent (GTK_WIDGET (box)->parent);
		}
	}
	return result;
}

static void
get_continue_side (eXperienceMatchTemp * match)
{
	tmpContinueSide sides = CONTINUE_INVALID;
	
	match->flags |= MATCH_CONTINUE_SIDE;
	match->continue_side = EXPERIENCE_CONTINUE_SIDE_NONE;
	
	/* This only works for buttons (we always draw buttons ... even if it is inside a GtkToolItem) */
	if (!(match->widget && GTK_IS_BUTTON (match->widget)))
		return;
	
	/* ok, got a button. Now we need to move up to the parent */
	sides = sides | get_button_location_parent (GTK_WIDGET (match->widget));
	
	if ((sides & CONTINUE_RIGHT) && (sides & CONTINUE_LEFT)) {
		match->continue_side |= EXPERIENCE_CONTINUE_SIDE_BOTH;
	} else if (sides & CONTINUE_LEFT) {
		match->continue_side |= EXPERIENCE_CONTINUE_SIDE_LEFT;
	} else if (sides & CONTINUE_RIGHT) {
		match->continue_side |= EXPERIENCE_CONTINUE_SIDE_RIGHT;
	} else if (sides & CONTINUE_NONE) {
		match->continue_side |= EXPERIENCE_CONTINUE_SIDE_SINGLE;
	}
}

static void
get_missing_match_flags (GtkWidget * widget, eXperienceMatchTemp * match, GdkRectangle * object_area)
{
	GtkProgressBarOrientation progress_orientation;
	match->widget = G_OBJECT (widget);
	
	if (widget != NULL) {
		if (!(match->flags & MATCH_ORIENTATION)) {
			if (GTK_IS_PROGRESS_BAR (widget)) {
				match->flags |= MATCH_ORIENTATION;
				progress_orientation = gtk_progress_bar_get_orientation (GTK_PROGRESS_BAR (widget));
				if (   (progress_orientation == GTK_PROGRESS_LEFT_TO_RIGHT)
				    || (progress_orientation == GTK_PROGRESS_RIGHT_TO_LEFT)) {
					match->orientation = 1 << GTK_ORIENTATION_HORIZONTAL;
				} else {
					match->orientation = 1 << GTK_ORIENTATION_VERTICAL;
				}
			}
			if (GTK_IS_TOOLBAR (widget)) {
				match->flags |= MATCH_ORIENTATION;
				match->orientation = 1 << gtk_toolbar_get_orientation (GTK_TOOLBAR (widget));
			}
		}
		
		if (!(match->flags & MATCH_TEXT_DIRECTION)) {
			match->flags |= MATCH_TEXT_DIRECTION;
			match->text_direction = 1 << gtk_widget_get_direction (widget);
		}
		
		if (!(match->flags & MATCH_CONTINUE_SIDE)) {
			get_continue_side (match);
		}
	}
	
	if (!(match->flags & MATCH_ORIENTATION)) {
		match->flags |= MATCH_ORIENTATION;
		match->orientation = object_area->width >= object_area->height ? 1 << GTK_ORIENTATION_HORIZONTAL : 1 << GTK_ORIENTATION_VERTICAL;
	}
}

static eXperienceGroup *
get_matching_group (eXperienceRcStyle *style, eXperienceMatchTemp * match)
{
	GList * list_entry;
	eXperienceGroup * group;
	
	g_return_val_if_fail (style != NULL, NULL);
	g_return_val_if_fail (match != NULL, NULL);
	
	experience_rc_style_load_groups(style);
	
	list_entry = g_list_first(style->groups);
	while (list_entry) {
		group = (eXperienceGroup*) list_entry->data;
		if (experience_match(&group->match, match)) {
			return group;
		}
		
		list_entry = g_list_next(list_entry);
	}
	return NULL;
}

static gboolean
draw_matching_group (GtkStyle *style, GtkWidget * widget, eXperienceMatchTemp * match, GdkWindow * window, GdkRectangle * object_area, GdkRectangle * area, eXperienceComponents draw_components)
{
	eXperienceGroup * group;
	gchar * class_path, * path;
	eXperienceRcStyle * rcstyle = EXPERIENCE_RC_STYLE(style->rc_style);
	
	get_missing_match_flags (widget, match, object_area);
	
	group = get_matching_group (rcstyle, match);
	
	if (widget && print_widget_path) {
		gtk_widget_class_path (widget, NULL, &class_path, NULL);
		gtk_widget_path (widget, NULL, &path, NULL);
		
		g_printerr ("Classpath: %s : \"%s\"\n", class_path, group != NULL ? group->name : "DEFAULT ENGINE");
		
		if (!g_str_equal (path, class_path)) {
			g_printerr ("Path:      %s\n", path);
		}
		
		g_free (path);
		g_free (class_path);
	}
	
	if (group != NULL) {
		if (object_area->width == REPLACE_WITH_LINE_WIDTH) {
			object_area->width = group->line_width;
		}
		if (object_area->height == REPLACE_WITH_LINE_WIDTH) {
			object_area->height = group->line_width;
		}
		
		if (experience_render_group (group, window, object_area, area, style)) {
			return TRUE;
		}
	}
	
	return FALSE;
}

/*---------------*/

static gboolean
draw_gap (GtkDrawingFunctions function,
          GtkStyle        *style,
          GdkWindow       *window,
          GtkStateType     state_type,
          GtkShadowType    shadow_type,
          GdkRectangle    *area,
          GtkWidget       *widget,
          const gchar     *detail,
          gint             x,
          gint             y,
          gint             width,
          gint             height,
          GtkPositionType  gap_side,
          gint             gap_x,
          gint             gap_width)
{
	#define _START 0
	#define _GAP 1
	#define _END 2
	
	gint xthickness[3], ythickness[3];
	eXperienceGroup * group_bg, * group_gap[3];
	eXperienceSize tmp_size;
	eXperienceRcStyle * rc_style;
	eXperienceMatchTemp match;
	GdkRectangle clip_area, gap_area[3];
	GdkRectangle object_area = { x, y, width, height };
	cairo_t * cr;
	gint i;
	
	g_return_val_if_fail (style  != NULL, FALSE);
	g_return_val_if_fail (window != NULL, FALSE);
	
	rc_style = EXPERIENCE_RC_STYLE (style->rc_style);
	
	if (area == NULL) {
		clip_area = clip_area;
	} else {
		clip_area = *area;
	}
	
	/* get match */
	match.detail   = (gchar*) detail;
	match.function = function;

	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.gap_side = 1 << gap_side;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_GAP_SIDE;
	
	get_missing_match_flags (widget, &match, &object_area);
	
	group_bg = get_matching_group (rc_style, &match);

	if (group_bg == NULL) return FALSE;
	/* ok. there is a drawable "background". */
	
	/* get the cairo context */
	cr = experience_get_cairo_context (window, &object_area, area);
	
	/* ############ */
	
	/* get the gap groups */
	/* now the other images */
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP_START : FUNCTION_BOX_GAP_START;
	group_gap[0] = get_matching_group (rc_style, &match);
	
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP : FUNCTION_BOX_GAP;
	group_gap[1] = get_matching_group (rc_style, &match);
	
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP_END : FUNCTION_BOX_GAP_END;
	group_gap[2] = get_matching_group (rc_style, &match);
	
	/* and their thickness */
	for (i = 0; i < 3; i++) {
		xthickness[i] = style->xthickness;
		ythickness[i] = style->ythickness;
		if (group_gap[i] != NULL) {
			if (group_gap[i]->settings_set & SETTINGS_GAP_SIZE) {
				xthickness[i] = group_gap[i]->gap_size;
				ythickness[i] = group_gap[i]->gap_size;
			}
		}
	}
	
	/* calculate the positions of the START/GAP/END */
	switch (gap_side) {
		case GTK_POS_LEFT:
			gap_area[_START].height = gap_x;
			gap_area[_GAP]  .height = gap_width;
			gap_area[_END]  .height = object_area.height - gap_x - gap_width;
			
			gap_area[_START].y = 0;
			gap_area[_GAP]  .y = gap_x;
			gap_area[_END]  .y = gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].x     = 0;
				gap_area[i].width = xthickness[i];
			}
		break;
		
		case GTK_POS_RIGHT:
			gap_area[_START].height = gap_x;
			gap_area[_GAP]  .height = gap_width;
			gap_area[_END]  .height = object_area.height - gap_x - gap_width;
			
			gap_area[_START].y = 0;
			gap_area[_GAP]  .y = gap_x;
			gap_area[_END]  .y = gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].x = object_area.width - xthickness[i];
				gap_area[i].width = xthickness[i];
			}
		break;
		
		case GTK_POS_TOP:
			gap_area[_START].width = gap_x;
			gap_area[_GAP]  .width = gap_width;
			gap_area[_END]  .width = object_area.width - gap_x - gap_width;
			
			gap_area[_START].x = 0;
			gap_area[_GAP]  .x = gap_x;
			gap_area[_END]  .x = gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].y = 0;
				gap_area[i].height = ythickness[i];
			}
		break;
		
		case GTK_POS_BOTTOM:
			gap_area[_START].width = gap_x;
			gap_area[_GAP]  .width = gap_width;
			gap_area[_END]  .width = object_area.width - gap_x - gap_width;
			
			gap_area[_GAP]  .x = gap_x;
			gap_area[_END]  .x = gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].height = ythickness[i];
				gap_area[i].y = object_area.height - ythickness[i];
			}
		break;
	}
	
	/* clip the unwanted regions */
	for (i = 0; i < 3; i++) {
		if (group_gap[i] != NULL) {
			/* clear the area */
			if (group_gap[i]->clear_area) {
				/* XXX: BUG! This forces cliping! */
				cairo_rectangle (cr, 0, 0, width, height);
				cairo_rectangle (cr, gap_area[i].x, gap_area[i].y, gap_area[i].width, gap_area[i].height);
				cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
				cairo_clip (cr);
			}
		}
	}
	
	/* render the box */
	tmp_size.width  = object_area.width;
	tmp_size.height = object_area.height;
	experience_render_group_to_cr (group_bg, cr, &tmp_size, style);
	
	for (i = 0; i < 3; i++) {
		if (group_gap[i] != NULL) {
			/* general cairo setup ... */
			cairo_save (cr);
			
			cairo_translate (cr, gap_area[i].x, gap_area[i].y);
			
			/* render the gaps */
			tmp_size.width  = gap_area[i].width;
			tmp_size.height = gap_area[i].height;
			
			experience_render_group_to_cr (group_gap[i], cr, &tmp_size, style);
			
			cairo_restore (cr);
		}
	}

	cairo_destroy (cr);
	
	#undef _START
	#undef _GAP
	#undef _END

	return TRUE;
}

/*---------------*/

void
experience_draw_hline  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x1,
                        gint             x2,
                        gint             y)
{
	GdkRectangle object_area = { x1, y, x2 - x1 + 1, REPLACE_WITH_LINE_WIDTH };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_LINE;
	match.state    = 1 << state_type;
	match.orientation = 1 << GTK_ORIENTATION_HORIZONTAL;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_STATE | MATCH_ORIENTATION;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_hline (style, window, state_type, area, widget, detail, x1, x2, y);
}
/*##################################*/

void
experience_draw_vline  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             y1,
                        gint             y2,
                        gint             x)
{
	GdkRectangle object_area = { x, y1, REPLACE_WITH_LINE_WIDTH, y2 - y1 + 1};
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_LINE;
	match.state    = 1 << state_type;
	match.orientation = 1 << GTK_ORIENTATION_VERTICAL;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_STATE | MATCH_ORIENTATION;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_vline (style, window, state_type, area, widget, detail, y1, y2, x);
}
/*##################################*/

void
experience_draw_shadow (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height};
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_SHADOW;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.gap_side = 1 << POS_NONE;
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_GAP_SIDE;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_BORDER))
		experience_style_parent_class->draw_shadow (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

static void
reverse_engineer_spin_button_button_size (GdkRectangle * area, GtkWidget * widget, GtkArrowType arrow_type, GdkRectangle * object_area)
{
	/* This function is not 100% acurate, which I think is not possible at all, but it is good enough. */
	area->x = 0;
	area->y = 0;
	
	if (arrow_type == GTK_ARROW_DOWN) {
		area->y = widget->requisition.height / 2;
		area->height = widget->requisition.height / 2;
	} else {
		area->height = (widget->requisition.height + 1) / 2;
	}
	
	area->width = object_area->x * 2 + object_area->width;
}

void
experience_draw_arrow  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        GtkArrowType     arrow_type,
                        gboolean         fill,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	GdkRectangle my_area;
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.arrow_direction = 1 << arrow_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_ARROW_DIRECTION;
	
	if (detail != NULL) {
		if (g_str_equal (detail, "spinbutton") && GTK_IS_SPIN_BUTTON (widget)) {
			/* My brother wants to be able to draw the spinbuttons bigger,
			   this means that the area has to be bigger than just the given
			   size of the arrow. */
			
			if (area == NULL) {
				reverse_engineer_spin_button_button_size (&my_area, widget, arrow_type, &object_area);
				/* draw and return. */
				
				match.function = FUNCTION_ARROW;
				
				/* If not drawn, then we don't return, and the normal image function will be called.
				   It will try again to draw the arrow, but I don't think the speed loss is too big. */
				if (draw_matching_group (style, widget, &match, window, &object_area, &my_area, COMPONENT_ALL))
					return;
			} else experience_warning ("Area of a spinbutton not NULL, this means GTK has changed.\n");
		}
	}
	
	match.function = FUNCTION_ARROW;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_arrow (style, window, state_type, shadow_type, area, widget, detail, arrow_type, fill, x, y, width, height);
}

/*##################################*/

void
experience_draw_box    (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	GtkWidget * menu_item_widget, * menu_widget;
	GtkMenu * menu;
	gchar * new_detail = NULL;
	GtkMenuItem * menu_item;
	GdkRectangle menu_item_area, menu_area;
	gint horizontal_offset;
	gint stepper_size, trough_border;
	gboolean has_forward = TRUE, has_backward = TRUE;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);

	/* detail is set later again, to new_detail (in case it has changed because of the secondary_) */
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_BOX;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.gap_side = 1 << POS_NONE;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_GAP_SIDE;	
	
	
	if (GTK_IS_SCROLLBAR (widget)) {
		/* This is a scrollbar ... now I need to know, wether this is the trough or not. */
		
		gtk_widget_style_get (widget, "stepper_size", &stepper_size,
		                              "trough_border", &trough_border,
		                              "has_forward_stepper", &has_forward,
		                              "has_backward_stepper", &has_backward,
		                              NULL);
		
		if (detail != NULL) {
			if (!(g_str_equal (detail, "trough") || g_str_equal (detail, "slider"))) {
				/* this is not the trough so it is a stepper button. */
				
				if (GTK_IS_HSCROLLBAR (widget)) {
					/* horizontal */
					/* the box can be at four positions. (stepper_a, b, c, d) */
					if (x - widget->allocation.x == trough_border) {
						/* stepper a or b */
						if (has_backward) {
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_LEFT;
						} else {
							/* secondary */
							new_detail = g_strconcat ("secondary_", detail, NULL);
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_RIGHT;
						}
					} else if (x  - widget->allocation.x == widget->allocation.width - trough_border - stepper_size) {
						/* stepper d or c */
						if (has_forward) {
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_RIGHT;
						} else {
							/* secondary */
							new_detail = g_strconcat ("secondary_", detail, NULL);
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_LEFT;
						}
					} else if (x  - widget->allocation.x == trough_border + stepper_size) {
						/* stepper b */
						new_detail = g_strconcat ("secondary_", detail, NULL);
						match.flags |= MATCH_ARROW_DIRECTION;
						match.arrow_direction = 1 << GTK_ARROW_RIGHT;
					} else if (x  - widget->allocation.x == widget->allocation.width - trough_border - 2 * stepper_size) {
						/* stepper c */
						new_detail = g_strconcat ("secondary_", detail, NULL);
						match.flags |= MATCH_ARROW_DIRECTION;
						match.arrow_direction = 1 << GTK_ARROW_LEFT;
					}
				} else if (GTK_IS_VSCROLLBAR (widget)) {
					/* vertical */
					if (y - widget->allocation.y == trough_border) {
						/* stepper a or b*/
						if (has_backward) {
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_UP;
						} else {
							/* secondary */
							new_detail = g_strconcat ("secondary_", detail, NULL);
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_DOWN;
						}
					} else if (y - widget->allocation.y == widget->allocation.height - trough_border - stepper_size) {
						/* stepper d or c */
						if (has_forward) {
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_DOWN;
						} else {
							/* secondary */
							new_detail = g_strconcat ("secondary_", detail, NULL);
							match.flags |= MATCH_ARROW_DIRECTION;
							match.arrow_direction = 1 << GTK_ARROW_UP;
						}
					} else if (y - widget->allocation.y == trough_border + stepper_size) {
						/* stepper b */
						new_detail = g_strconcat ("secondary_", detail, NULL);
						match.flags |= MATCH_ARROW_DIRECTION;
						match.arrow_direction = 1 << GTK_ARROW_DOWN;
					} else if (y - widget->allocation.y == widget->allocation.height - trough_border - 2 * stepper_size) {
						/* stepper c */
						new_detail = g_strconcat ("secondary_", detail, NULL);
						match.flags |= MATCH_ARROW_DIRECTION;
						match.arrow_direction = 1 << GTK_ARROW_UP;
					}
				} else experience_warning ("Got a scrollbar which is nether a vertical nor a horizontal scrollbar!\n");
			}
		}
	}
	
	if (GTK_IS_MENU (widget) && GDK_IS_WINDOW (window)) {
		menu = GTK_MENU (widget);
		menu_item_widget = gtk_menu_get_attach_widget (menu);
		if (GTK_IS_MENU_ITEM (menu_item_widget) && GDK_IS_WINDOW (menu_item_widget->window)) {
			menu_item = GTK_MENU_ITEM (menu_item_widget);
			
			gtk_widget_style_get (widget,
			                      "horizontal_offset", &horizontal_offset,
			                      NULL);
			
			gdk_window_get_origin (window, &menu_area.x, &menu_area.y);
			
			menu_area.x += x;
			menu_area.y += y;
			menu_area.width  = widget->allocation.width;
			menu_area.height = widget->allocation.height;
			
			gdk_window_get_origin (menu_item_widget->window, &menu_item_area.x, &menu_item_area.y);
			
			menu_item_area.x += menu_item_widget->allocation.x;
			menu_item_area.y += menu_item_widget->allocation.y;
			
			menu_item_area.width  = menu_item_widget->allocation.width;
			menu_item_area.height = menu_item_widget->allocation.height;
			
			if ((menu_area.x + menu_area.width - menu_item_area.x + 1) == -horizontal_offset) {
				/* left */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_RIGHT, menu_item_area.y - menu_area.y, menu_item_area.height))
					return;
			} else if ((menu_area.x - menu_item_area.x - menu_item_area.width - 1) == horizontal_offset) {
				/* right */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_LEFT, menu_item_area.y - menu_area.y, menu_item_area.height))
					return;
			} else if ((menu_area.y + menu_area.height - menu_item_area.y + 1) == 1) {
				/* top */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_BOTTOM, menu_item_area.x - menu_area.x, menu_item_area.width))
					return;
			} else if ((menu_area.y - menu_item_area.y - menu_item_area.height - 1) == -1) {
				/* bottom */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_TOP, menu_item_area.x - menu_area.x, menu_item_area.width))
					return;
			}
		}
	}
	
	/* Code duplication ahead! */
	if (GTK_IS_MENU_ITEM (widget) && GDK_IS_WINDOW (window)) {
		menu_item = GTK_MENU_ITEM (widget);
		menu_widget = GTK_WIDGET (gtk_menu_item_get_submenu (menu_item));
		if (GTK_IS_MENU (menu_widget) && GDK_IS_WINDOW (menu_widget->window)) {
			menu = GTK_MENU (menu_widget);
			
			gtk_widget_style_get (menu_widget,
			                      "horizontal_offset", &horizontal_offset,
			                      NULL);
			
			gdk_window_get_origin (menu_widget->window, &menu_area.x, &menu_area.y);
			
			menu_area.x += menu_widget->allocation.x;
			menu_area.y += menu_widget->allocation.y;
			menu_area.width  = menu_widget->allocation.width;
			menu_area.height = menu_widget->allocation.height;
			
			gdk_window_get_origin (window, &menu_item_area.x, &menu_item_area.y);
			
			menu_item_area.x += widget->allocation.x;
			menu_item_area.y += widget->allocation.y;
			
			menu_item_area.width  = widget->allocation.width;
			menu_item_area.height = widget->allocation.height;
			
			if ((menu_area.x + menu_area.width - menu_item_area.x + 1) == -horizontal_offset) {
				if (menu_area.y - menu_item_area.y < 0) {
					menu_area.height += menu_area.y - menu_item_area.y; /* make width/height smaller */
					menu_area.y = menu_item_area.y;
				}
				if (menu_area.y + menu_area.height > menu_item_area.y + menu_item_area.height) {
					menu_area.height = menu_item_area.height - (menu_item_area.y - menu_area.y);
				}
				/* left */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_LEFT, menu_area.y - menu_item_area.y, menu_area.height))
					return;
			} else if ((menu_area.x - menu_item_area.x - menu_item_area.height - 1) == horizontal_offset) {
				if (menu_area.y - menu_item_area.y < 0) {
					menu_area.height += menu_area.y - menu_item_area.y; /* make width/height smaller */
					menu_area.y = menu_item_area.y;
				}
				if (menu_area.y + menu_area.height > menu_item_area.y + menu_item_area.height) {
					menu_area.height = menu_item_area.height - (menu_item_area.y - menu_area.y);
				}
				/* right */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_RIGHT, menu_area.y - menu_item_area.y, menu_area.height))
					return;
			} else if ((menu_area.y + menu_area.height - menu_item_area.y + 1) == 1) {
				if (menu_area.x - menu_item_area.x < 0) {
					menu_area.width += menu_area.x - menu_item_area.x; /* make width/height smaller */
					menu_area.x = menu_item_area.x;
				}
				if (menu_area.x + menu_area.width > menu_item_area.x + menu_item_area.width) {
					menu_area.width = menu_item_area.width - (menu_item_area.x - menu_area.x);
				}
				/* top */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_TOP, menu_area.x - menu_item_area.x, menu_area.width))
					return;
			} else if ((menu_area.y - menu_item_area.y - menu_item_area.height - 1) == -1) {
				if (menu_area.x - menu_item_area.x < 0) {
					menu_area.width += menu_area.x - menu_item_area.x; /* make width/height smaller */
					menu_area.x = menu_item_area.x;
				}
				if (menu_area.x + menu_area.width > menu_item_area.x + menu_item_area.width) {
					menu_area.width = menu_item_area.width - (menu_item_area.x - menu_area.x);
				}
				/* bottom */
				if (draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, GTK_POS_BOTTOM, menu_area.x - menu_item_area.x, menu_area.width))
					return;
			}
		}
	}
	
	if (new_detail != NULL)
		match.detail   = new_detail;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_box (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
	
	if (new_detail != NULL) {
		g_free (new_detail);
	}
}
/*##################################*/

void
experience_draw_flat_box   (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);

	match.detail   = (gchar*) detail;
	match.function = FUNCTION_FLAT_BOX;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE;

	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_flat_box (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

void
experience_draw_check  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_CHECK;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_check (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

void
experience_draw_option (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_OPTION;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_option (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

void
experience_draw_tab    (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_TAB;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_tab (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

void
experience_draw_shadow_gap (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height,
                        GtkPositionType  gap_side,
                        gint             gap_x,
                        gint             gap_width)
{
	if (!draw_gap (FUNCTION_SHADOW, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, gap_side, gap_x, gap_width))
		experience_style_parent_class->draw_shadow_gap (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, gap_side, gap_x, gap_width);
}
/*##################################*/

void
experience_draw_box_gap(GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height,
                        GtkPositionType  gap_side,
                        gint             gap_x,
                        gint             gap_width)
{
	if (!draw_gap (FUNCTION_BOX, style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, gap_side, gap_x, gap_width))
		experience_style_parent_class->draw_shadow_gap (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, gap_side, gap_x, gap_width);
}
/*##################################*/

void
experience_draw_extension  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height,
                        GtkPositionType  gap_side)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_EXTENSION;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.gap_side = 1 << gap_side;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_GAP_SIDE;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_extension (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, gap_side);
}
/*##################################*/

void
experience_draw_focus  (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_FOCUS;
	match.state    = 1 << state_type;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_STATE;
	
	if (!draw_matching_group(style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_focus (style, window, state_type, area, widget, detail, x, y, width, height);
}
/*##################################*/

void
experience_draw_slider (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height,
                        GtkOrientation   orientation)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_SLIDER;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.orientation = 1 << orientation;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_ORIENTATION;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_slider (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, orientation);
}
/*##################################*/

void
experience_draw_handle (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GtkShadowType    shadow_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        gint             width,
                        gint             height,
                        GtkOrientation   orientation)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_HANDLE;
	match.shadow   = 1 << shadow_type;
	match.state    = 1 << state_type;
	match.orientation = 1 << orientation;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_SHADOW | MATCH_STATE | MATCH_ORIENTATION;
	
	if (!draw_matching_group(style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_handle (style, window, state_type, shadow_type, area, widget, detail, x, y, width, height, orientation);
}
/*##################################*/

void
experience_draw_expander   (GtkStyle        *style,
                        GdkWindow       *window,
                        GtkStateType     state_type,
                        GdkRectangle    *area,
                        GtkWidget       *widget,
                        const gchar     *detail,
                        gint             x,
                        gint             y,
                        GtkExpanderStyle expander_style)
{
	GdkRectangle object_area = { x, y, 0, 0 };
	eXperienceMatchTemp match;
	gint expander_size;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_EXPANDER;
	match.state    = 1 << state_type;
	match.expander_style = 1 << expander_style;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_STATE | MATCH_EXPANDER_STYLE;
	
	gtk_widget_style_get (widget,
	                      "expander-size", &expander_size,
	                      NULL);
	
	object_area.width  = expander_size;
	object_area.height = expander_size;
	object_area.x = object_area.x - expander_size / 2;
	object_area.y = object_area.y - expander_size / 2;
	
	if (!draw_matching_group (style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_expander (style, window, state_type, area, widget, detail, x, y, expander_style);
}
/*##################################*/

void
experience_draw_resize_grip (GtkStyle       *style,
                         GdkWindow      *window,
                         GtkStateType    state_type,
                         GdkRectangle   *area,
                         GtkWidget      *widget,
                         const gchar    *detail,
                         GdkWindowEdge   edge,
                         gint            x,
                         gint            y,
                         gint            width,
                         gint            height)
{
	GdkRectangle object_area = { x, y, width, height };
	eXperienceMatchTemp match;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);
	
	match.detail   = (gchar*) detail;
	match.function = FUNCTION_RESIZE_GRIP;
	match.state    = 1 << state_type;
	match.window_edge = 1 << edge;
	
	match.flags = MATCH_DETAIL | MATCH_FUNCTION | MATCH_STATE | MATCH_WINDOW_EDGE;
	
	if (!draw_matching_group(style, widget, &match, window, &object_area, area, COMPONENT_ALL))
		experience_style_parent_class->draw_resize_grip (style, window, state_type, area, widget, detail, edge, x, y, width, height);
}
/*##################################*/
/* copied from gtkstyle.c. See authors of gtk+ for information. LGPL */
GdkPixbuf *
experience_render_icon (GtkStyle * style,
                    const GtkIconSource * source,
                    GtkTextDirection direction,
                    GtkStateType state,
                    GtkIconSize size,
                    GtkWidget * widget,
                    const gchar * detail)
{
	gint width = 1;
	gint height = 1;
	GdkPixbuf *scaled;
	GdkPixbuf *stated;
	GdkPixbuf *base_pixbuf;
	GdkScreen *screen;
	GtkSettings *settings;
	
	/* Oddly, style can be NULL in this function, because
	 * GtkIconSet can be used without a style and if so
	 * it uses this function.
	 */
	
	base_pixbuf = gtk_icon_source_get_pixbuf (source);
	
	g_return_val_if_fail (base_pixbuf != NULL, NULL);
	
	if (widget && gtk_widget_has_screen (widget)) {
		screen = gtk_widget_get_screen (widget);
		settings = gtk_settings_get_for_screen (screen);
	} else if (style && style->colormap) {
		screen = gdk_colormap_get_screen (style->colormap);
		settings = gtk_settings_get_for_screen (screen);
	} else {
		settings = gtk_settings_get_default ();
		GTK_NOTE (MULTIHEAD, g_warning ("Using the default screen for gtk_default_render_icon()"));
	}
	
	
	if (size != (GtkIconSize) -1 && !gtk_icon_size_lookup_for_settings (settings, size, &width, &height)) {
		g_warning (G_STRLOC ": invalid icon size '%d'", size);
		return NULL;
	}
	
	/* If the size was wildcarded, and we're allowed to scale, then scale; otherwise,
	 * leave it alone.
	 */
	if (size != (GtkIconSize)-1 && gtk_icon_source_get_size_wildcarded (source))
		scaled = experience_gdk_pixbuf_scale_simple_or_ref (base_pixbuf, width, height, GDK_INTERP_BILINEAR);
	else
		scaled = g_object_ref (base_pixbuf);
	
	/* If the state was wildcarded, then generate a state. */
	if (gtk_icon_source_get_state_wildcarded (source)) {
		if ((style != NULL) && EXPERIENCE_IS_STYLE (style) && (state >= 0) && (state <= 4)) {
			stated = gdk_pixbuf_copy (scaled);      
			
			stated = experience_apply_filters (stated, &EXPERIENCE_RC_STYLE (style->rc_style)->icon_filter [state]);
			
			g_object_unref (scaled);
		} else {
			stated = scaled;
		}
	} else {
		stated = scaled;
	}
  
	return stated;
}
