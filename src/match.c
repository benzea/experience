/* arch-tag: 4c90aaf8-a5c0-49fb-9fdc-305380d48923 */

/*  eXperience GTK engine: match.c
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

#include <string.h>
#include "match.h"

typedef struct {
	gchar * property;
	GValueArray * array;
} gvalue_list_item;

/* If GPatternSpec was refcounted, or could be copied ... I wouldn't need all of this crap :) */
typedef struct {
	gint refcount;
	GPatternSpec * pspec;
} eXperiencePatternSpec;

static eXperiencePatternSpec *
experience_pattern_spec_create (gchar * pattern)
{
	g_assert (pattern != NULL);
	
	eXperiencePatternSpec * new_pattern = g_new0 (eXperiencePatternSpec, 1);
	new_pattern->refcount = 1;
	new_pattern->pspec = g_pattern_spec_new (pattern);
	
	return new_pattern;
}

static void
experience_pattern_spec_unref (eXperiencePatternSpec * pspec)
{
	g_assert (pspec != NULL);
	
	pspec->refcount--;
	
	if (pspec->refcount == 0) {
		/* free the pattern spec */
		g_pattern_spec_free (pspec->pspec);
		
		/* and the struct */
		g_free (pspec);
	}
	/* this would happen, if a pspec is unrefed too often */
	g_assert (pspec->refcount >= 0);
}

static void
experience_pattern_spec_ref (eXperiencePatternSpec * pspec)
{
	g_assert (pspec != NULL);
	g_assert (pspec->refcount > 0); /* can't ref a destroyed object */
	
	pspec->refcount++;
}

/* ############### */

void
experience_match_init (eXperienceMatch * match)
{
	g_assert (match != NULL);
	match->flags = MATCH_NONE;
	
	match->property_list = NULL;
	
	/* no other initialization needed here. */
}

void
experience_match_finalize (eXperienceMatch * match)
{
	GList * item;
	gvalue_list_item * list_item;
	
	g_assert (match != NULL);
	
	/* destroy the eXperiencePatternSpecs */
	g_list_foreach (match->details, (GFunc) experience_pattern_spec_unref, NULL);
	g_list_foreach (match->program_names, (GFunc) experience_pattern_spec_unref, NULL);
	
	/* and the lists */
	g_list_free (match->details);
	g_list_free (match->program_names);

	item = g_list_first (match->property_list);
	
	while (item != NULL) {
		list_item = (gvalue_list_item*) item->data;
		
		g_free (list_item->property);
		if (list_item->array != NULL) {
			g_value_array_free (list_item->array);
		}
		
		g_free (list_item);
		
		item = g_list_next (item);
	}
	g_list_free (match->property_list);
}

void
experience_match_set_states (eXperienceMatch * match, GtkStateType state)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_STATE;
	match->state |= state;
}

void
experience_match_set_functions (eXperienceMatch * match, GtkDrawingFunctions functions)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_FUNCTION;
	match->functions |= functions;
}

void
experience_match_set_text_directions (eXperienceMatch * match, GtkTextDirection text_directions)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_TEXT_DIRECTION;
	match->text_directions |= text_directions;
}

/* adds a GPatternSpec to a g_list if it is not in the list. */
static GList *
add_pattern_spec_to_list (GList * list, eXperiencePatternSpec * pspec)
{
	GList * item;
	
	item = g_list_first (list);
	
	while (item != NULL) {
		if (g_pattern_spec_equal (pspec->pspec, (GPatternSpec*) item->data)) {
			return list; /* just return ... don't need to do anything */
		}
		item = g_list_next (item);
	}
	/* needs to be inserted ... ref it and then append it */
	experience_pattern_spec_ref (pspec);
	return g_list_append (list, pspec);
}


void
experience_match_add_detail (eXperienceMatch * match, gchar * detail)
{
	eXperiencePatternSpec * pspec;
	
	g_assert (match != NULL);
	g_assert (detail != NULL);
	
	match->flags |= MATCH_DETAIL;
	
	pspec = experience_pattern_spec_create (detail);
	match->details = add_pattern_spec_to_list (match->details, pspec);
	experience_pattern_spec_unref (pspec);
	
	g_free (detail);
}

void
experience_match_add_program_name (eXperienceMatch * match, gchar * program_name)
{
	eXperiencePatternSpec * pspec;
	
	g_assert (match != NULL);
	g_assert (program_name != NULL);
	
	match->flags |= MATCH_PROGRAM_NAME;
	
	pspec = experience_pattern_spec_create (program_name);
	match->program_names = add_pattern_spec_to_list (match->program_names, pspec);
	experience_pattern_spec_unref (pspec);
	
	g_free (program_name);
}

void
experience_match_set_shadows (eXperienceMatch * match, GtkShadowType shadow)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_SHADOW;
	match->shadow |= shadow;
}

void
experience_match_set_arrow_directions (eXperienceMatch * match, GtkArrowType arrow_direction)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_ARROW_DIRECTION;
	match->arrow_direction |= arrow_direction;
}

void
experience_match_set_orientations (eXperienceMatch * match, GtkOrientation orientation)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_ORIENTATION;
	match->orientation |= orientation;
}

void
experience_match_set_gap_sides (eXperienceMatch * match, GtkPositionType gap_side)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_GAP_SIDE;
	match->gap_side |= gap_side;
}

void
experience_match_set_expander_styles (eXperienceMatch * match, GtkExpanderStyle expander_style)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_EXPANDER_STYLE;
	match->expander_style |= expander_style;
}

void
experience_match_set_window_edges (eXperienceMatch * match, GdkWindowEdge window_edge)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_WINDOW_EDGE;
	match->window_edge |= window_edge;
}

void
experience_match_set_continue_sides (eXperienceMatch * match, eXperienceContinueSide sides)
{
	g_assert (match != NULL);
	
	match->flags |= MATCH_CONTINUE_SIDE;
	match->continue_side |= sides;
}

/* ---- */

static gvalue_list_item *
get_value_array (eXperienceMatch * match, gchar * property, gboolean new_if_none)
{
	GList * item;
	gvalue_list_item * tmp_item;
	
	item = g_list_first (match->property_list);
	
	while (item != NULL) {
		if (g_str_equal (((gvalue_list_item*) item->data)->property, property)) {
			return (gvalue_list_item*) item;
		}
		item = g_list_next (item);
	}
	
	if (!new_if_none) return NULL;
	
	/* the property name does not yet exists, so append it. */
	tmp_item = g_new (gvalue_list_item, 1);
	tmp_item->property = g_strdup (property);
	tmp_item->array = g_value_array_new (1);
	match->property_list = g_list_append (match->property_list, (gpointer) tmp_item);
	
	return tmp_item;
}

void
experience_match_set_property (eXperienceMatch * match, gchar * property, GValueArray * properties)
{
	gvalue_list_item * item;
	GValue * new_value;
	gint i;
	
	g_assert (match != NULL);
	
	item = get_value_array (match, property, TRUE);
	
	if (properties == NULL) {
		/* needs to be deleted again, this is kinda stupid */
		g_value_array_free (item->array);
		item->array = NULL;
		g_free (property);
		return;
	}
	
	if (item->array == NULL) {
		/* needs to be deleted again, this is kinda stupid */
		g_value_array_free (item->array);
		item->array = properties;
		g_free (property);
		return;
	}
	
	new_value = g_new0 (GValue, 1);
	for (i = 0; i < properties->n_values; i++) {
		g_value_init (new_value, G_VALUE_TYPE (g_value_array_get_nth (properties, i)));
		
		g_value_copy (g_value_array_get_nth (properties, i), new_value);
		g_value_array_append (item->array, new_value);
		
		/* g_value_array_append copies the value so we need to free everything again */
		g_value_unset (new_value);
	}
	g_free (new_value);
	
	g_value_array_free (properties);
	g_free (property);
}

/* ---- */

static void
copy_param_specs (eXperiencePatternSpec * pspec, GList ** dest)
{
	(*dest) = add_pattern_spec_to_list (*dest, pspec);
}

void
experience_match_inherit (eXperienceMatch * match, eXperienceMatch * from)
{
	GList * item;
	gvalue_list_item * list_item, * new_item;
	
	if (!(match->flags & MATCH_STATE))          match->state           = from->state;
	if (!(match->flags & MATCH_FUNCTION))       match->functions       = from->functions;
	if (!(match->flags & MATCH_TEXT_DIRECTION)) match->text_directions = from->text_directions;
	
	if (!(match->flags & MATCH_DETAIL)) {
		g_list_foreach (from->details, (GFunc) copy_param_specs, &match->details);
	}
	if (!(match->flags & MATCH_PROGRAM_NAME)) {
		g_list_foreach (from->program_names, (GFunc) copy_param_specs, &match->program_names);
	}
	
	if (!(match->flags & MATCH_SHADOW))          match->shadow         = from->shadow;
	if (!(match->flags & MATCH_ARROW_DIRECTION)) match->arrow_direction= from->arrow_direction;
	if (!(match->flags & MATCH_ORIENTATION))     match->orientation    = from->orientation;
	if (!(match->flags & MATCH_GAP_SIDE))        match->gap_side       = from->gap_side;
	if (!(match->flags & MATCH_EXPANDER_STYLE))  match->expander_style = from->expander_style;
	if (!(match->flags & MATCH_WINDOW_EDGE))     match->window_edge    = from->window_edge;
	if (!(match->flags & MATCH_CONTINUE_SIDE))   match->continue_side  = from->continue_side;
	
	item = g_list_first (from->property_list);
	while (item != NULL) {
		list_item = (gvalue_list_item*) item->data;
		
		if (list_item->array != NULL) {
			if (get_value_array (match, list_item->property, FALSE) == NULL) {
				/* needs to be copied over */
				new_item = g_new (gvalue_list_item, 1);
				new_item->property = g_strdup (list_item->property);
				new_item->array = g_value_array_copy (list_item->array);
				match->property_list = g_list_append (match->property_list, (gpointer) new_item);
			}
		}
		
		item = g_list_next (item);
	}
	
	match->flags = match->flags | from->flags;
}

static gboolean
match_to_pattern_spec_list (GList * list, gchar * pattern)
{
	gchar * reverse;
	GList * item;
	gint length;
	if (pattern == NULL)
		return TRUE;
	if (list == NULL)
		return TRUE;
	
	reverse = g_strreverse (g_strdup (pattern));
	length  = strlen (pattern);
	item = list;
	
	while (item != NULL) {
		if (g_pattern_match (((eXperiencePatternSpec*) item->data)->pspec, length, pattern, reverse)) {
			g_free (reverse);
			return TRUE;
		}
		item = g_list_next (item);
	}
	g_free (reverse);
	return FALSE;
}

static void
get_property (eXperienceMatchTemp * to, gchar * property, GValue * property_value)
{
	/* I created this function, so that I can override some properties
	 * with values from the engine/another widget ... well it doesn't work
	 * for comboboxes ... */
	g_object_get_property (to->widget, property, property_value);
}

gboolean
experience_match (eXperienceMatch * match, eXperienceMatchTemp * to)
{
	eXperienceMatchFlags flags;
	GValue property_value, transformed_value, * list_value;
	GObjectClass * widget_class;
	GParamSpec * param_spec;
	GType property_type, list_type;
	GList * item;
	gvalue_list_item * list_item;
	gboolean matched, compared;
	gint i;
	
	g_return_val_if_fail (match != NULL, FALSE);
	g_return_val_if_fail (to    != NULL, FALSE);
	
	flags = match->flags & to->flags;
	
	if (flags & MATCH_STATE)
		if (!(match->state & to->state)) return FALSE;
	if (flags & MATCH_FUNCTION)
		if (!(match->functions & to->function)) return FALSE;
	if (flags & MATCH_SHADOW)
		if (!(match->shadow & to->shadow)) return FALSE;
	if (flags & MATCH_ARROW_DIRECTION)
		if (!(match->arrow_direction & to->arrow_direction)) return FALSE;
	if (flags & MATCH_ORIENTATION)
		if (!(match->orientation & to->orientation)) return FALSE;
	if (flags & MATCH_GAP_SIDE)
		if (!(match->gap_side & to->gap_side)) return FALSE;
	if (flags & MATCH_EXPANDER_STYLE)
		if (!(match->expander_style & to->expander_style)) return FALSE;
	if (flags & MATCH_WINDOW_EDGE)
		if (!(match->window_edge & to->window_edge)) return FALSE;
	if (flags & MATCH_TEXT_DIRECTION)
		if (!(match->text_directions & to->text_direction)) return FALSE;
	if (flags & MATCH_CONTINUE_SIDE) {
		if (!(match->continue_side & to->continue_side)) return FALSE;
	}
	
	/* now to the more expensive ones */
	if (flags & MATCH_DETAIL)
		if (!match_to_pattern_spec_list (match->details, to->detail))
			return FALSE;
	if (match->flags & MATCH_PROGRAM_NAME)
		if (!match_to_pattern_spec_list (match->program_names, g_get_prgname()))
			return FALSE;
	
	/*-------*/
	if (to->widget != NULL) {
		item = g_list_first (match->property_list);
		
		while (item != NULL) {
			matched = FALSE;
			compared = FALSE;
			
			list_item = (gvalue_list_item*) item->data;
			
			widget_class = G_OBJECT_GET_CLASS (to->widget);
			param_spec = g_object_class_find_property (widget_class, list_item->property);
			
			if ((param_spec != NULL) && (list_item->array != NULL)) {
				memset (&property_value, 0, sizeof(GValue));
				g_value_init (&property_value, param_spec->value_type);
				
				get_property (to, list_item->property, &property_value);
				
				property_type = G_VALUE_TYPE (&property_value);
				
				for (i = 0; i < list_item->array->n_values; i++) {
					list_value = g_value_array_get_nth (list_item->array, i);
					
					list_type = G_VALUE_TYPE (list_value);
					
					memset (&transformed_value, 0, sizeof(GValue));
					g_value_init (&transformed_value, list_type);
					
					if (g_value_transform (&property_value, &transformed_value)) {
						compared = TRUE;
						/* this can be transformed */
						switch (list_type) {
							case G_TYPE_STRING:
								if (g_str_equal (g_value_get_string (&transformed_value), g_value_get_string (list_value))) matched = TRUE;
								break;
							case G_TYPE_BOOLEAN:
								if (g_value_get_boolean (&transformed_value) == g_value_get_boolean (list_value)) matched = TRUE;
								break;
							case G_TYPE_FLOAT:
								if (g_value_get_float (&transformed_value) == g_value_get_float (list_value)) matched = TRUE;
								break;
							default:
								g_assert_not_reached ();
								break;
						}
					}
					g_value_unset (&transformed_value);
				}
				g_value_unset (&property_value);
				
				if ((matched == FALSE) && (compared == TRUE)) return FALSE;
			}
			
			item = g_list_next (item);
		}
	}
	/*-------*/
	return TRUE;
}
