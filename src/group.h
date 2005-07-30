/* arch-tag: 901e1b3c-0f88-49cb-8aea-0ee2dce5e8a5 */

/*  eXperience GTK engine: group.h
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

#ifndef __GROUP_H
#define __GROUP_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "match.h"
#include "data.h"

typedef enum {
	SETTINGS_LINE_WIDTH  = 1 << 0,
	SETTINGS_GAP_SIZE    = 1 << 1,
	SETTINGS_DONT_CLIP   = 1 << 2,
	SETTINGS_CLEAR_AREA  = 1 << 3,
	SETTINGS_PADDING     = 1 << 4,
} eXperienceGroupSettingsList;

typedef struct _eXperienceGroup eXperienceGroup;

struct _eXperienceGroup {
	eXperienceGroupSettingsList settings_set;
	
	eXperienceFilter filter;
	eXperienceMatch match;
	
	guint line_width;
	guint gap_size;
	
	gboolean dont_clip;
	gboolean clear_area;
	eXperiencePadding padding;
	
	gboolean circular_dependency;
	gboolean circular_dependency_check;
	
	/* private */
	
	gchar * name;
	gchar * parent_name;
	eXperienceGroup * parent;
	
	guint _refcount;
	
	GList * drawables;
};

#include "drawable.h"

eXperienceGroup * experience_group_create (gchar * name, gchar * inherit);

eXperienceDrawable * experience_group_add_drawable (eXperienceGroup * group, guint number, eXperienceDrawableClass * class);

void experience_group_set_line_width (eXperienceGroup * group, guint width);
void experience_group_set_gap_size (eXperienceGroup * group, guint size);
void experience_group_set_dont_clip (eXperienceGroup * group, gboolean dont_clip);
void experience_group_set_padding (eXperienceGroup * group, gint left, gint right, guint top, gint bottom);
void experience_group_set_clear_area (eXperienceGroup * group, gboolean clear_area);

void experience_group_apply_inheritance (eXperienceGroup * group);
void experience_group_cleanup (eXperienceGroup * group);

eXperienceGroup * experience_group_deep_copy (eXperienceGroup * group, gchar * prepend_name);

void experience_group_ref (eXperienceGroup * group);
void experience_group_ref_or_load (eXperienceGroup * group);
gboolean experience_group_unref (eXperienceGroup * group);

gboolean experience_group_draw (eXperienceGroup * group, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style);

void experience_group_unref_ptr (void * groups);

#endif /* __GROUP_H */
