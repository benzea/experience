/*  eXperience GTK engine: parsetree.c
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
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "image.h"
#include "parsetree.h"
#include "group.h"
#include "utils.h"

void
parse_begin_group (parsestate* pst, gchar* name, gchar* inherit, gboolean load)
{
	gint i;
	gchar * group_name = name;
	
	g_assert (pst != NULL);
	g_assert (pst->groups != NULL);
	g_assert (pst->current_group == NULL);
	g_assert (experience_engine_state == PARSING);
	
	if (!load) {
		if (group_name != NULL) {
			if (g_hash_table_lookup(pst->groups, group_name)) {
				experience_warning("The group name \"%s\" is used more than once!\n", name);
				g_free(group_name);
				group_name = NULL;
			}
		}
		
		while (group_name == NULL) {
			group_name = g_strdup ("RESERVED_XXXXXXXX");
			for (i = 9; i < 17; i++) {
				group_name[i] = (char) g_random_int_range ((gint)'A', (gint)'Z'+1);
			}
			
			if (g_hash_table_lookup (pst->groups, group_name)) {
				g_free (group_name);
				group_name = NULL;
			}
		}
		
		
		pst->current_group = experience_group_create (group_name, inherit);
		pst->rcstyle->group_names = g_list_append (pst->rcstyle->group_names, g_strdup (group_name));
	} else {
		g_assert (name != NULL);
		pst->current_group = g_hash_table_lookup (pst->groups, name);
	}
	
	pst->current_filter = &pst->current_group->filter;
	
	if (!load) {
		if (!g_hash_table_lookup (pst->groups, group_name)) {
			g_hash_table_insert (pst->groups, g_strdup(group_name), pst->current_group);
		} else {
			experience_warning ("Group \"%s\" is more than once!\n", group_name);
		}
	}
	g_free(group_name);
	if (inherit != NULL)
		g_free(inherit);
}

void
parse_end_group (parsestate* pst)
{
	g_assert (pst != NULL);
	
	pst->current_group = NULL;

	pst->current_filter = NULL;
}


void
parse_begin_drawable (parsestate* pst, gint image_number, eXperienceDrawableClass * class)
{
	g_assert (pst != NULL);
	g_assert (pst->current_group != NULL);
	
	pst->current_drawable = experience_group_add_drawable (pst->current_group, image_number, class);
	
	pst->current_filter = &pst->current_drawable->filter;
}

void
parse_end_drawable (parsestate* pst)
{
	g_assert (pst != NULL);
	
	experience_drawable_unref (pst->current_drawable);
	
	pst->current_drawable = NULL;
	pst->current_filter = &pst->current_group->filter;
}

void
parse_begin_icons (parsestate* pst)
{
	g_assert (pst != NULL);
	pst->current_filter = NULL;
}

void
parse_end_icons (parsestate* pst)
{
	g_assert (pst != NULL);
	pst->current_filter = NULL;
}

void
parse_begin_icons_state (parsestate* pst, GtkStateType state)
{
	g_assert (pst != NULL);
	pst->current_filter = &pst->rcstyle->icon_filter[state];
}

void
parse_end_icons_state (parsestate* pst)
{
	g_assert (pst != NULL);
	pst->current_filter = NULL;
}

void
parsestate_init (parsestate * pst, GHashTable * experience_groups, GScanner* scanner)
{
	g_assert (pst != NULL);
	g_assert (pst->scanner == NULL);
	g_assert (pst->groups == NULL);
	g_assert (scanner != NULL);
	
	pst->scanner = scanner;
	
	pst->groups = experience_groups;
	
	pst->gap_group[0] = NULL;
	pst->gap_group[1] = NULL;
	pst->gap_group[2] = NULL;
	
	pst->gap_image[0] = NULL;
	pst->gap_image[1] = NULL;
	pst->gap_image[2] = NULL;
	pst->should_end = 0;
}

void
parsestate_destroy (parsestate* pst)
{
	g_assert (pst != NULL);
	
	g_free (pst);
}


