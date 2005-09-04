/* arch-tag: f0552231-42b8-4fd2-8cc7-130c94c56132 */


/*  eXperience GTK engine: rcstyle.c
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


#include "style.h"
#include "rcstyle.h"
#include <gdk/gdk.h>
#include <glib.h>
#include "parser.h"
#include "group.h"
#include "experience.h"
#include "image_loading.h"

static void      experience_rc_style_init         (eXperienceRcStyle      *style);
static void      experience_rc_style_class_init   (eXperienceRcStyleClass *klass);
static GtkStyle *experience_rc_style_create_style (GtkRcStyle         *rc_style);

static GtkRcStyleClass *parent_class;

static parsestate * experience_pst = NULL;
static gchar      * theme_name = NULL;

GType experience_type_rc_style = 0;

void
experience_rc_style_register_type (GTypeModule *module)
{
	if (!experience_type_rc_style) {
		static const GTypeInfo object_info =
		{
			sizeof (eXperienceRcStyleClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) experience_rc_style_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (eXperienceRcStyle),
			0,              /* n_preallocs */
			(GInstanceInitFunc) experience_rc_style_init,
			NULL
		};
		
		experience_type_rc_style = g_type_module_register_type (module,
		                         GTK_TYPE_RC_STYLE,
		                         "eXperienceRcStyle",
		                         &object_info, 0);
	}
}

static void
experience_rc_style_init (eXperienceRcStyle *style)
{
	gint i;
	
	style->group_names = NULL;
	style->groups = NULL;
	style->got_groups = FALSE;
	
	for (i = 0; i < 5; i++) {
		experience_filter_init (&style->icon_filter[i], FILTER_OPACITY | FILTER_SATURATION | FILTER_PIXELATE | FILTER_BRIGHTNESS);
	}
	
	style->icon_filter[GTK_STATE_INSENSITIVE].opacity = 0.4;
}

static void
unref_group (gpointer data, gpointer dummy)
{
	 experience_group_unref (data);
}

static void
experience_rc_style_finalize (GObject *object)
{
	gint i;
	eXperienceRcStyle * style = EXPERIENCE_RC_STYLE(object);
	GObjectClass    *parent_g_object_class = G_OBJECT_CLASS (parent_class);
	
	experience_rc_style_load_groups (style);
	
	if (style->groups != NULL) {
		g_list_foreach (style->groups, unref_group, NULL);
		g_list_free (style->groups);
	}
	
	if (style->group_names != NULL) {
		g_list_foreach (style->group_names, (GFunc) g_free, NULL);
		g_list_free (style->group_names);
	}
	
	for (i = 0; i < 5; i++) {
		experience_filter_finalize (&style->icon_filter[i]);
	}
	
	if (parent_g_object_class->finalize != NULL) {
		parent_g_object_class->finalize((GObject*) style);
	}
}

static void
experience_rc_style_merge (GtkRcStyle *dest, GtkRcStyle *src)
{
	gint i;
	gboolean add;
	GList * item, * other_item;
	
	if (EXPERIENCE_IS_RC_STYLE (src)) {
		eXperienceRcStyle *experience_dest = EXPERIENCE_RC_STYLE (dest);
		eXperienceRcStyle *experience_src = EXPERIENCE_RC_STYLE (src);

		item = g_list_first (experience_src->group_names);
		while (item != NULL) {
			add = TRUE;
			other_item = g_list_first (experience_dest->group_names);
			while ((other_item != NULL) && add) {
				if (g_str_equal (item->data, other_item->data)) add = FALSE;
				
				other_item = g_list_next (other_item);
			}
			
			if (add) experience_dest->group_names = g_list_append (experience_dest->group_names, g_strdup (item->data));
			item = g_list_next (item);
		}
		
		for (i = 0; i < 5; i++) {
			experience_filter_inherit_from (&experience_dest->icon_filter[i], &experience_src->icon_filter[i]);
		}
	}
	
	parent_class->merge (dest, src);
}

static guint
experience_rc_style_parse (GtkRcStyle   *rc_style,
                       GtkSettings  *settings,
                       GScanner     *scanner)
{
	eXperienceRcStyle *experience_rc_style = EXPERIENCE_RC_STYLE (rc_style);
	gchar * current_name;
	
	g_object_get (G_OBJECT (settings),
	              "gtk-theme-name", &current_name,
	              NULL);
	
	if (experience_pst != NULL) {
		if ((theme_name != NULL) || (experience_engine_state == RUNNING)) {
			if ((experience_engine_state == RUNNING) || ((theme_name != NULL) && !g_str_equal (theme_name, current_name))) {
				experience_cleanup_everything ();
			}
		} else {
			g_assert_not_reached ();
		}
	}
	
	
	if (experience_groups == NULL) {
		experience_groups = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, experience_group_unref_ptr);
	}
	
	if (theme_name != NULL) g_free (theme_name);
	theme_name = current_name;
	
	experience_engine_state = PARSING;
	
	if (experience_pst == NULL) {
		experience_pst = g_new0(parsestate, 1);
		
		parsestate_init(experience_pst, experience_groups, scanner);
	}
	experience_pst->expected_token = G_TOKEN_NONE;
	experience_pst->scanner = scanner;
	experience_pst->rcstyle = experience_rc_style;
	experience_pst->settings = settings;

	experience_read_gtkrcstyle(experience_pst);
	
	return experience_pst->expected_token;
}

static void
experience_rc_style_class_init (eXperienceRcStyleClass *klass)
{
	GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);
	GObjectClass    *g_object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	rc_style_class->merge = experience_rc_style_merge;
	rc_style_class->create_style = experience_rc_style_create_style;
	rc_style_class->parse        = experience_rc_style_parse;
	
	g_object_class->finalize     = experience_rc_style_finalize;
}

/* Create an empty style suitable to this RC style
 */
static GtkStyle *
experience_rc_style_create_style (GtkRcStyle *rc_style)
{
	return GTK_STYLE (g_object_new (EXPERIENCE_TYPE_STYLE, NULL));
}

static gint
group_walk_up (eXperienceGroup * group, gchar * group_name)
{
	gint result;
	
	if (!group) return 1;
	if (group->parent == NULL) return 1;
	
	if (group->circular_dependency) {
		if (group_name) experience_warning ("%s < ", group_name);
		return -1;
	}
	
	if (group->circular_dependency_check) {
		if (group_name) experience_warning ("%s < ", group_name);
		group->circular_dependency = TRUE;
		return 0;
	}
	
	group->circular_dependency_check = TRUE;
	
	result = group_walk_up (group->parent, group->parent_name);
	
	if (result == 1) {
		experience_group_apply_inheritance (group);
	} else {
		if (group_name)
			if (!parser_error) g_printerr ("%s < ", group_name);
		group->circular_dependency = TRUE;
	}
	return result;
}

static gboolean
group_walk_inheritance (gchar * group_name, eXperienceGroup * group, GHashTable * dest)
{
	gint walk_result;
	
	if (group->parent != NULL) {
		/* group already seen in a circular dependency? if so, dump it */
		if (group->circular_dependency_check) return FALSE;
		
		walk_result = group_walk_up (group, NULL);
		
		if (walk_result == 0) {
			if (!parser_error) g_printerr ("%s: circular dependency!\n", group_name);
			return FALSE;
		} else if (walk_result == -1) {
			if (!parser_error) g_printerr ("%s: depends on a group inside a circular dependency!\n", group_name);
			return FALSE;
		}
	}
	
	g_hash_table_insert (dest, group_name, group);
	
	return TRUE;
}

static void
group_make_inheritance_pointers (gchar * group_name, eXperienceGroup * group, GHashTable * source)
{
	/* if the group doesn't want to live on ... delete it!  */
	
	if (!group->parent_name) return;

	group->parent = g_hash_table_lookup (source, group->parent_name);
	if (group->parent == NULL) {
  		experience_warning("Group %s depends on a non existant group (%s). Ignoring dependency!\n", group_name, group->parent_name);
	}
}

static void
cleanup_group (gchar * group_name, eXperienceGroup * group, gpointer dummy)
{
	experience_group_cleanup (group);
}

static void
cleanup_groups(void)
{
	GHashTable * new_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, experience_group_unref_ptr);
	GHashTable * old_hash_table = experience_groups;
	
	experience_groups = new_hash_table;
	g_hash_table_foreach (old_hash_table, (GHFunc) group_make_inheritance_pointers, old_hash_table);
	g_hash_table_foreach_steal (old_hash_table, (GHRFunc) group_walk_inheritance, new_hash_table);
	
	g_hash_table_foreach (experience_groups, (GHFunc) cleanup_group, NULL);
	
	g_hash_table_destroy(old_hash_table);
	
	experience_engine_state = RUNNING;
}

static void
experience_rc_style_load_group (gchar * group_name, eXperienceRcStyle * rcstyle)
{
	eXperienceGroup * group = g_hash_table_lookup(experience_groups, group_name);
	
	if (group != NULL) {
		experience_group_ref (group);
		rcstyle->groups = g_list_append (rcstyle->groups, group);
	}
}

void
experience_rc_style_load_groups (eXperienceRcStyle * rcstyle)
{
	if (rcstyle->got_groups) return;
	
	if (experience_engine_state == PARSING) {
		cleanup_groups ();
	} else if (experience_engine_state == NONE) {
		g_assert_not_reached ();
	}
	
	g_list_foreach(rcstyle->group_names, (GFunc) experience_rc_style_load_group, rcstyle);
	
	rcstyle->got_groups = TRUE;
}

void
experience_cleanup_everything (void)
{
	/* cleanup */
	if (experience_groups != NULL) {
		g_hash_table_destroy (experience_groups);
		experience_groups = NULL;
	}
	
	if (experience_pst != NULL) {
		g_free (experience_pst);
		experience_pst = NULL;
	}
	
	/* cleanup cache */
	experience_image_cache_destroy ();
	
	experience_engine_state = NONE;
}
