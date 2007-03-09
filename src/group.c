/*  eXperience GTK engine: group.c
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

#include <math.h>
#include <string.h>
#include <glib/gprintf.h>
#include "group.h"
#include "experience.h"

#define CHECK_AND_SET_SETTING(group, setting, message) { \
	if (group->settings_set & setting) { \
		experience_warning ("In group \"%s\": %s\n", group->name, message); \
		return; \
	} else group->settings_set |= setting; \
}

eXperienceGroup *
experience_group_create (gchar * name, gchar * inherit)
{
	eXperienceGroup * new_group = g_new0 (eXperienceGroup, 1);
	gchar buffer[500];
	
	new_group->_refcount = 1;
	
	new_group->name = g_strdup(name);
	new_group->parent_name = g_strdup(inherit);
	
	experience_filter_init (&new_group->filter, FILTER_SATURATION | FILTER_BRIGHTNESS | FILTER_OPACITY | FILTER_ROTATE | FILTER_MIRROR);
	
	g_sprintf (buffer, "group \"%s\"", name);
	experience_filter_set_info_string (&new_group->filter, buffer);
	
	new_group->line_width = 2;
	
	experience_match_init (&new_group->match);
	
	return new_group;
}

void
experience_group_ref (eXperienceGroup * group)
{
	g_assert (group != NULL);
	
	group->_refcount++;
}

static void
free_drawable(gpointer data, gpointer user_data)
{
	experience_drawable_unref ((eXperienceDrawable*) data);
}

gboolean
experience_group_unref (eXperienceGroup * group)
{
	g_assert (group != NULL);
	g_assert (group->_refcount != 0);
	
	group->_refcount--;
	
	if (group->_refcount == 0) {
		experience_match_finalize (&group->match);
		
		g_list_foreach (group->drawables, free_drawable, NULL);
		g_list_free (group->drawables);
		
		g_assert (group->name != NULL);
		g_free (group->name);
		
		if (group->parent_name != NULL)
			g_free (group->parent_name);
		
		experience_filter_finalize (&group->filter);
		
		g_free (group);
		
		return TRUE;
	}
	
	return FALSE;
}


eXperienceDrawable *
experience_group_add_drawable (eXperienceGroup * group, guint number, eXperienceDrawableClass * class)
{
	eXperienceDrawable * new_drawable = NULL;
	GList * current_drawable;
	gboolean done;
	
	g_assert (group != NULL);
	
	
	current_drawable = group->drawables;
	done = FALSE;
		
	while ((current_drawable != NULL) && (!done)) {
		if (((eXperienceDrawable*) current_drawable->data)->number == number) {
			if (((eXperienceDrawable*) current_drawable->data)->class != class) {
				g_printerr ("Error in group \"%s\". Drawable #%i has diffrent types.\n", group->name, number);
				/* don't ref drawable here, so that it will be destroyed by parsetree.c */
			} else {
				new_drawable = current_drawable->data;
				experience_drawable_ref (new_drawable);
			}
		} else if (((eXperienceDrawable*)current_drawable->data)->number > number) {
			new_drawable = experience_drawable_create(class, number, group->name);
			experience_drawable_ref (new_drawable);
			
			group->drawables = g_list_insert_before(group->drawables, current_drawable, new_drawable);
			
			done = TRUE;
		}
		current_drawable = g_list_next(current_drawable);
	}
	if (!done) {
		new_drawable = experience_drawable_create(class, number, group->name);
		experience_drawable_ref (new_drawable);
		
		group->drawables = g_list_append(group->drawables, new_drawable);
		done = TRUE;
	}
	
	return new_drawable;
}

void
experience_group_set_line_width (eXperienceGroup * group, guint width)
{
	g_assert (group != NULL);
	
	CHECK_AND_SET_SETTING (group, SETTINGS_LINE_WIDTH, "Line width was set more than once!");
	
	group->line_width = width;
}

void
experience_group_set_gap_size (eXperienceGroup * group, guint size)
{
	g_assert (group != NULL);
	
	CHECK_AND_SET_SETTING (group, SETTINGS_GAP_SIZE, "Gap size was set more than once!");
	
	group->gap_size = size;
}

void
experience_group_set_dont_clip (eXperienceGroup * group, gboolean dont_clip)
{
	g_assert (group != NULL);
	
	CHECK_AND_SET_SETTING (group, SETTINGS_DONT_CLIP, "dont_clip was set more than once!");
	
	group->dont_clip = dont_clip;
}

void
experience_group_set_clear_area (eXperienceGroup * group, gboolean clear_area)
{
	g_assert (group != NULL);
	
	CHECK_AND_SET_SETTING (group, SETTINGS_CLEAR_AREA, "clear_area was set more than once!");
	
	group->clear_area = clear_area;
}

void
experience_group_set_padding (eXperienceGroup * group, gint left, gint right, guint top, gint bottom)
{
	g_assert (group != NULL);
	
	CHECK_AND_SET_SETTING (group, SETTINGS_PADDING, "Padding was set more than once!");
	
	group->padding.left   = left;
	group->padding.right  = right;
	group->padding.top    = top;
	group->padding.bottom = bottom;
}

/*-------------*/

void
experience_group_apply_inheritance (eXperienceGroup * group)
{
	GList * list1, * list2;
	eXperienceDrawable * drawable1, * drawable2;
	gboolean next_drawable;
	
	g_assert (group  != NULL);
	
	if (group->parent == NULL) return;
	
	/* now copy all data from the other group! */
	/* filters */
	experience_filter_inherit_from (&group->filter, &group->parent->filter);
	
	/* matching rules */
	experience_match_inherit (&group->match, &group->parent->match);
	
	/* gap size */
	if (!(group->settings_set & SETTINGS_GAP_SIZE)) {
		group->gap_size  = group->parent->gap_size;
	}
	
	/* dont_clip */
	if (!(group->settings_set & SETTINGS_DONT_CLIP)) {
		group->dont_clip = group->parent->dont_clip;
	}

	/* padding */
	if (!(group->settings_set & SETTINGS_PADDING)) {
		group->padding = group->parent->padding;
	}
	
	group->settings_set |= group->parent->settings_set;
	
	
	/* copy stuff from images: */
	list1 = g_list_first (group->drawables);
	list2 = g_list_first (group->parent->drawables);
	
	while (list1) {
		drawable1 = list1->data;
		next_drawable = FALSE;
		while (list2 && !next_drawable) {
			drawable2 = list2->data;
			if (drawable2->number < drawable1->number) {
				group->drawables = g_list_insert_before (group->drawables, list1, experience_drawable_duplicate (drawable2, group->name));
				list2 = g_list_next(list2);
			} else {
				if (drawable2->number == drawable1->number) {
					experience_drawable_inherit_from (drawable1, drawable2);
					list2 = g_list_next (list2);
				} else {
					next_drawable = TRUE;
				}
			}
		}
		list1 = g_list_next(list1);
	}
	while (list2) {
		drawable2 = list2->data;
		
		group->drawables = g_list_append(group->drawables, experience_drawable_duplicate (drawable2, group->name));
		
		list2 = g_list_next(list2);
	}
	
	/* set parent pointer to NULL */
	group->parent = NULL;
}

void
experience_group_cleanup (eXperienceGroup * group)
{
	GList * link;
	eXperienceDrawable * drawable;
	
	g_assert(group);
	
	/* cleanup images */
	link = g_list_first (group->drawables);
	while (link) {
		drawable = link->data;
		experience_drawable_apply_group_settings (drawable, group);
		
		link = g_list_next (link);
	}
	
	/* Clean out all the stuff in the filter, that is applied to the drawables.
	 * This needs to be done, so that I can create copies, and then apply the
	 * filters again. (For embeding groups inside a drawable) */
	group->filter.saturation = 0;
	group->filter.brightness = 0;
	
	return;
}

eXperienceGroup *
experience_group_deep_copy (eXperienceGroup * group, gchar * prepend_name)
{
	eXperienceGroup * new_group;
	gchar * new_name;
	
	g_assert (group != NULL);
	if (prepend_name != NULL) {
		new_name = g_strjoin (NULL, prepend_name, "->", group->name, NULL);
	} else {
		new_name = g_strdup (group->name);
	}
	
	new_group = experience_group_create (new_name, NULL);
	
	/* copy all data from this group to the new one, by forcing an inheritance. */
	new_group->parent = group;
	experience_group_apply_inheritance (new_group);
	
	g_free (new_name);
	
	return new_group;
}

/*---------*/

gboolean
experience_group_draw (eXperienceGroup * group, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	GList * list;
	cairo_surface_t * orig_surface, * sub_surface = NULL;
	cairo_t * sub_cr;
	eXperienceDrawable * drawable;
	eXperienceSize real_dest_size;
	cairo_matrix_t matrix;
	gboolean result = TRUE;
	gint tmp;
	
	g_return_val_if_fail (group != NULL, FALSE);
	g_return_val_if_fail (cr    != NULL, FALSE);
	g_return_val_if_fail (style != NULL, FALSE);
	g_return_val_if_fail (dest_size != NULL, FALSE);
	
	real_dest_size = *dest_size;
	
	orig_surface = cairo_get_target (cr);
	
	/* There are two cases:
	 *  1. opacity == 1
	 *  2. opacity < 1 
	 * If the opacity is 1, we will not create a new surface, and clip the old one ...
	 * otherwise we create a surface of the correct size. */
	
	cairo_save (cr);
	
	if (group->filter.opacity == 1) {
		if (!group->dont_clip) {
			cairo_rectangle (cr, 0, 0, dest_size->width, dest_size->height);
			cairo_clip (cr);
		}
		
		sub_cr = cr; /* draw directly on the cr */
		
		cairo_translate (cr, group->padding.left, group->padding.top);
	} else {
		/* create a subsurface */
/*		if (!group->dont_clip) {*/
		/* XXX: BUG! It now clips always! */
			sub_surface = cairo_surface_create_similar (orig_surface, CAIRO_CONTENT_COLOR_ALPHA,
			                                            dest_size->width, dest_size->height);
			sub_cr = cairo_create (sub_surface);
			cairo_surface_destroy (sub_surface);
			
			cairo_translate (sub_cr, group->padding.left, group->padding.top);
/*		} else {
			sub_surface = cairo_surface_create_similar (orig_surface, CAIRO_CONTENT_COLOR_ALPHA,
			                                            
		}*/
	}
	
	/* create a new surface */
	real_dest_size.width  -= group->padding.left + group->padding.right;
	real_dest_size.height -= group->padding.top  + group->padding.bottom;
	
	/* first mirror */
	cairo_matrix_init_identity (&matrix);
	
	if (group->filter.mirror & ORIENTATION_HORIZONTAL) {
		matrix.xx = -1.0;
		matrix.x0 = real_dest_size.width;
	}
	if (group->filter.mirror & ORIENTATION_VERTICAL) {
		matrix.yy = -1.0;
		matrix.y0 = real_dest_size.height;
	}
	
	/* then rotate */
	/* Rotation is broken in cairo! See bug #2488 */
	switch (group->filter.rotation) {
		case ROTATE_CW:
			cairo_matrix_rotate (&matrix, M_PI_2);
			
			matrix.x0 += real_dest_size.width;
			
			/* switch width<->height */
			tmp = real_dest_size.width;
			real_dest_size.width  = real_dest_size.height;
			real_dest_size.height = tmp;
			break;
		case ROTATE_CCW:
			cairo_matrix_rotate (&matrix, -M_PI_2);
			
			matrix.y0 += real_dest_size.height;
			
			/* switch width<->height */
			tmp = real_dest_size.width;
			real_dest_size.width  = real_dest_size.height;
			real_dest_size.height = tmp;
			break;
		case ROTATE_AROUND:
			cairo_matrix_rotate (&matrix, M_PI);
			
			matrix.x0 += real_dest_size.height;
			matrix.y0 += real_dest_size.width;
			break;
		default:
			break;
	}
	
	/* then transform */
	cairo_transform (sub_cr, &matrix);
	
	list = group->drawables;
	
	while (list != NULL) {
		drawable = list->data;
		
		if (!experience_drawable_draw (drawable, sub_cr, &real_dest_size, style)) {
			g_printerr ("Couldn't draw widget, because \"%s %i\" in group \"%s\" couldn't be drawn.\n", drawable->class->object_type, drawable->number, group->name);
			
			result = FALSE;
			goto end;
		}
		
		list = g_list_next (list);
	}
	
	if (group->filter.opacity != 1) { /* we created a subsurface */
		cairo_set_source_surface (cr, sub_surface, 0, 0);
		
		cairo_paint_with_alpha (cr, group->filter.opacity);
	}
	
end:
	if (sub_cr != cr) {
		cairo_destroy (sub_cr); /* destroy cairo context */
	}
	
	cairo_restore (cr);
	return result;
}


void
experience_group_unref_ptr (void * groups)
{
	experience_group_unref ((eXperienceGroup*) groups);
}

