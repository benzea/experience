/*  eXperience GTK engine: group_drawable.c
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

#include "group_drawable.h"

void
experience_group_drawable_set_group_name (eXperienceGroupDrawable * group_drawable, gchar * group_name)
{
	g_assert (group_drawable != NULL);
	g_assert (group_drawable->drawable.class == experience_group_drawable_class);
	
	if (!group_drawable->group_name_set) {
		group_drawable->group_name_set = TRUE;
		group_drawable->group_name = group_name;
	} else g_printerr ("Color set more than once in group_drawable %i in group %s!\n", group_drawable->drawable.number, group_drawable->drawable.group_name);
}

/*#######*/
static eXperienceDrawable *
create (void)
{
	eXperienceGroupDrawable * new_group_drawable;
	new_group_drawable = g_new0 (eXperienceGroupDrawable, 1);
	
	experience_filter_init (&new_group_drawable->drawable.filter, FILTER_SATURATION | FILTER_BRIGHTNESS | FILTER_OPACITY | FILTER_ROTATE | FILTER_MIRROR);
	
	return (eXperienceDrawable*) new_group_drawable;
}

static void
destroy (eXperienceDrawable * drawable)
{
	eXperienceGroupDrawable * group_drawable = (eXperienceGroupDrawable* )drawable;
	
	g_assert (drawable != NULL);
	
	if (group_drawable->group_name)
		g_free (group_drawable->group_name);
	
	if (group_drawable->group)
		experience_group_unref (group_drawable->group);
	
	g_free (group_drawable);
}

static gboolean
draw (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	eXperienceGroupDrawable * group_drawable = (eXperienceGroupDrawable*) drawable;
	eXperienceGroup * orig_group;
	
	g_assert (drawable   != NULL);
	g_assert (dest_size  != NULL);
	g_assert (drawable->class == experience_group_drawable_class);
	
	if (group_drawable->group == NULL) {
		/* try to get the group */
		if (group_drawable->group_name == NULL) {
			experience_warning ("group name not set!\n");
			return FALSE;
		}
		
		orig_group = g_hash_table_lookup (experience_groups, group_drawable->group_name);
		
		if (orig_group == NULL) {
			experience_warning ("Could not get group \"%s\"\n", group_drawable->group_name);
			return FALSE;
		}
		
		group_drawable->group = experience_group_deep_copy (orig_group, drawable->group_name);
		
		if (group_drawable->group == NULL) {
			/* XXX: can't be reached currently, I think */
			experience_warning ("Could not create a copy of group \"%s\"\n", group_drawable->group_name);
			return FALSE;
		}
		
		experience_filter_apply_filter (&group_drawable->group->filter, &drawable->filter);
	}
	
	experience_group_draw (group_drawable->group, cr, dest_size, style);
	
	return TRUE;
}

static void
inherit_from_drawable (eXperienceDrawable * drawable, eXperienceDrawable * from)
{
	eXperienceGroupDrawable * group_drawable = (eXperienceGroupDrawable*) drawable;
	eXperienceGroupDrawable * group_drawable_from = (eXperienceGroupDrawable*) from;
	
	g_assert (drawable != NULL);
	g_assert (from != NULL);
	g_assert (drawable->class == experience_group_drawable_class);
	g_assert (from->class == experience_group_drawable_class);
	
	if (!group_drawable->group_name_set) {
		group_drawable->group_name_set = group_drawable_from->group_name_set;
		group_drawable->group_name = g_strdup (group_drawable_from->group_name);
	}
}

static void
apply_group_settings (eXperienceDrawable * drawable, eXperienceGroup * group)
{
	return;
}

static eXperienceDrawableClass _experience_group_drawable_class;

void
experience_group_drawable_init_class (void)
{
	_experience_group_drawable_class.object_type = "group";
	_experience_group_drawable_class.create      = create;
	_experience_group_drawable_class.destroy     = destroy;
	_experience_group_drawable_class.get_info    = NULL;
	_experience_group_drawable_class.draw        = draw;
	_experience_group_drawable_class.inherit_from_drawable = inherit_from_drawable;
	_experience_group_drawable_class.apply_group_settings  = apply_group_settings;
	
	experience_group_drawable_class = &_experience_group_drawable_class;
}

