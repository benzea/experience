/*  eXperience GTK engine: main.c
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

#include <stdlib.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include "experience.h"
#include "style.h"
#include "rcstyle.h"
#include "image.h"
#include "fill.h"
#include "group_drawable.h"

G_MODULE_EXPORT void theme_init (GTypeModule *module);
G_MODULE_EXPORT void theme_exit (void);
G_MODULE_EXPORT GtkRcStyle * theme_create_rc_style (void);
G_MODULE_EXPORT const gchar* g_module_check_init (GModule *module);

static gboolean
decode_bool (gchar * value)
{
	gchar * copy;
	if (value) {
		copy = g_ascii_strdown (value, -1);
		
		if (g_str_equal (copy, "true")) {
			return TRUE;
		}
		
		free (copy);
	}
	return FALSE;
}

G_MODULE_EXPORT void
theme_init (GTypeModule *module)
{
	experience_g_quark = g_quark_from_string("experience_theme_engine");
	
	experience_groups = NULL;
	
	experience_rc_style_register_type (module);
	experience_style_register_type (module);
	
	/* widget path display */
	print_widget_path  = FALSE;
	warn_invalid_gtype = FALSE;
	
	print_widget_path  = decode_bool(getenv ("EXPERIENCE_PRINT_WIDGET_PATH"));
	warn_invalid_gtype = decode_bool(getenv ("EXPERIENCE_WARN_INVALID_GTYPE"));
	
	experience_image_init_class ();
	experience_fill_init_class ();
	experience_group_drawable_init_class ();
}

G_MODULE_EXPORT void
theme_exit (void)
{
	experience_cleanup_everything ();
}

G_MODULE_EXPORT GtkRcStyle *
theme_create_rc_style (void)
{
	return GTK_RC_STYLE (g_object_new (EXPERIENCE_TYPE_RC_STYLE, NULL));
}

/* The following function will be called by GTK+ when the module
 * is loaded and checks to see if we are compatible with the
 * version of GTK+ that loads us.
 */
const gchar*
g_module_check_init (GModule *module)
{
	return gtk_check_version (GTK_MAJOR_VERSION,
	                          GTK_MINOR_VERSION,
	                          GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}
