/*  eXperience GTK engine: rcstyle.h
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

#ifndef __RCSTYLE_H
#define __RCSTYLE_H

#include <gtk/gtk.h>
#include <gtk/gtkrc.h>
#include "filter.h"

typedef struct _eXperienceRcStyle eXperienceRcStyle;
typedef struct _eXperienceRcStyleClass eXperienceRcStyleClass;

GType experience_type_rc_style;

#define EXPERIENCE_TYPE_RC_STYLE              experience_type_rc_style
#define EXPERIENCE_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), EXPERIENCE_TYPE_RC_STYLE, eXperienceRcStyle))
#define EXPERIENCE_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), EXPERIENCE_TYPE_RC_STYLE, eXperienceRcStyleClass))
#define EXPERIENCE_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), EXPERIENCE_TYPE_RC_STYLE))
#define EXPERIENCE_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), EXPERIENCE_TYPE_RC_STYLE))
#define EXPERIENCE_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), EXPERIENCE_TYPE_RC_STYLE, eXperienceRcStyleClass))

struct _eXperienceRcStyle
{
	GtkRcStyle parent_instance;
	
	eXperienceFilter icon_filter[5];
	
	GList * group_names;
	GList * groups;
	gboolean got_groups;
};

struct _eXperienceRcStyleClass
{
	GtkRcStyleClass parent_class;
};

void experience_rc_style_register_type (GTypeModule *module);
void experience_rc_style_load_groups (eXperienceRcStyle * rcstyle);

void experience_cleanup_everything (void);

#endif /* __RCSTYLE_H */
