/* arch-tag: 43e21a92-ae82-468c-9c48-1aa2dff4beb9 */

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
		}
		
		if (!(match->flags & MATCH_TEXT_DIRECTION)) {
			match->flags |= MATCH_TEXT_DIRECTION;
			match->text_direction = 1 << gtk_widget_get_direction (widget);
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
#if 0	
	gint xthickness[3], ythickness[3];
	gint i;
	eXperienceRcStyle * rc_style;
	GdkRectangle object_area = { x, y, width, height },
	             gap_area[3];
	GdkRectangle clip_region;
	GdkRectangle dest_area_bg, tmp_area;
	eXperienceMatchTemp match;
	eXperienceGroup * group_bg, * group_gap[3];
	GdkPixbuf * pixbuf_bg        = NULL,
	          * tmp_pixbuf;
	
	g_return_val_if_fail (style != NULL, FALSE);
	g_return_val_if_fail (window != NULL, FALSE);
	
	rc_style = EXPERIENCE_RC_STYLE (style->rc_style);
	
	if ((object_area.width == -1) && (object_area.height == -1)) {
		gdk_window_get_size(window, &object_area.width, &object_area.height);
	} else if (width == -1)
		gdk_window_get_size(window, &object_area.width, NULL);
	else if (height == -1)
		gdk_window_get_size(window, NULL, &object_area.height);
	
	if (area == NULL) {
		clip_region = object_area;
	} else {
		clip_region = *area;
	}
	
	/* ---- */
	
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
	
	for (i = 0; i < 3; i++) {
		gap_area[i] = object_area;
	}
	
	dest_area_bg = clip_region;
	
	/* render it to a experience. */
	/* don't get it from the pixbuf cache ... */
	pixbuf_bg = experience_render_group_to_new_pixbuf (group_bg, &dest_area_bg, &object_area, NULL, style);
	
	if (pixbuf_bg == NULL) return FALSE;
	
	/* now the other images */
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP_START : FUNCTION_BOX_GAP_START;
	group_gap[0] = get_matching_group (rc_style, &match);
	
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP : FUNCTION_BOX_GAP;
	group_gap[1] = get_matching_group (rc_style, &match);
	
	match.function = function == FUNCTION_SHADOW ? FUNCTION_SHADOW_GAP_END : FUNCTION_BOX_GAP_END;
	group_gap[2] = get_matching_group (rc_style, &match);
	
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
	
	switch (gap_side) {
		case GTK_POS_LEFT:
			gap_area[_START].height = gap_x;
			gap_area[_GAP]  .height = gap_width;
			gap_area[_END]  .height = object_area.height - gap_x - gap_width;
			
			gap_area[_GAP]  .y += gap_x;
			gap_area[_END]  .y += gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].width = xthickness[i];
			}
		break;
		
		case GTK_POS_RIGHT:
			gap_area[_START].height = gap_x;
			gap_area[_GAP]  .height = gap_width;
			gap_area[_END]  .height = object_area.height - gap_x - gap_width;
			
			gap_area[_GAP]  .y += gap_x;
			gap_area[_END]  .y += gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].width = xthickness[i];
				gap_area[i].x += object_area.width - xthickness[i];
			}
		break;
		
		case GTK_POS_TOP:
			gap_area[_START].width = gap_x;
			gap_area[_GAP]  .width = gap_width;
			gap_area[_END]  .width = object_area.width - gap_x - gap_width;
			
			gap_area[_GAP]  .x += gap_x;
			gap_area[_END]  .x += gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].height = ythickness[i];
			}
		break;
		
		case GTK_POS_BOTTOM:
			gap_area[_START].width = gap_x;
			gap_area[_GAP]  .width = gap_width;
			gap_area[_END]  .width = object_area.width - gap_x - gap_width;
			
			gap_area[_GAP]  .x += gap_x;
			gap_area[_END]  .x += gap_x + gap_width;
			
			for (i = 0; i < 3; i++) {
				gap_area[i].height = ythickness[i];
				gap_area[i].y += object_area.height - ythickness[i];
			}
		break;
	}
	
	/* clear area if wanted. */
	for (i = 0; i < 3; i++) {
		if (group_gap[i] != NULL) {
			if (group_gap[i]->clear_area) {
				tmp_area.x = gap_area[i].x - dest_area_bg.x;
				tmp_area.y = gap_area[i].y - dest_area_bg.y;
				tmp_area.width  = gap_area[i].width;
				tmp_area.height = gap_area[i].height;
				
				if (tmp_area.x < 0) {
					tmp_area.width += tmp_area.x;
					tmp_area.x = 0;
				}
				if (tmp_area.y < 0) {
					tmp_area.height += tmp_area.y;
					tmp_area.y = 0;
				}
				if (tmp_area.width + tmp_area.x > gdk_pixbuf_get_width (pixbuf_bg)) {
					tmp_area.width = gdk_pixbuf_get_width (pixbuf_bg) - tmp_area.x;
				}
				if (tmp_area.height + tmp_area.y > gdk_pixbuf_get_height (pixbuf_bg)) {
					tmp_area.height = gdk_pixbuf_get_height (pixbuf_bg) - tmp_area.y;
				}
				
				if ((tmp_area.width > 0) && (tmp_area.height > 0)) {
					tmp_pixbuf = gdk_pixbuf_new_subpixbuf (pixbuf_bg, tmp_area.x, tmp_area.y, tmp_area.width, tmp_area.height);
					gdk_pixbuf_fill (tmp_pixbuf, 0x00000000);
					g_object_unref (tmp_pixbuf);
				}
			}
		}
	}
	
	/* finaly draw everything. */
	if (pixbuf_bg != NULL) { 
		experience_render_pixbuf_to_window (window, pixbuf_bg, &dest_area_bg, NULL, group_bg->filter.opacity);
	}
	
	for (i = 0; i < 3; i++) {
		if (group_gap[i] != NULL) {
			experience_render_group (group_gap[i], window, &gap_area[i], &clip_region, style);
		}
	}
	
	if (pixbuf_bg != NULL) g_object_unref (pixbuf_bg);
#endif
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
	GtkWidget * menu_item_widget;
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
				/* TODO: add a match, so that it is possible to check wether it is a secondary stepper (c/d). */
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
	
	/* if not done there would be a gap drawn. */
/*	this can now be done with padding and clip = FALSE
	if (object_area.width > 0) {
		object_area.width++;
	}
	if (object_area.height > 0){
		object_area.height++;
	}*/
	
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
