/*  eXperience GTK engine: style.h
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

#include <gtk/gtkstyle.h>

typedef struct _eXperienceStyle eXperienceStyle;
typedef struct _eXperienceStyleClass eXperienceStyleClass;

GType experience_type_style;
GtkStyleClass *experience_style_parent_class;

#define EXPERIENCE_TYPE_STYLE              experience_type_style
#define EXPERIENCE_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), EXPERIENCE_TYPE_STYLE, eXperienceStyle))
#define EXPERIENCE_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), EXPERIENCE_TYPE_STYLE, eXperienceStyleClass))
#define EXPERIENCE_IS_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), EXPERIENCE_TYPE_STYLE))
#define EXPERIENCE_IS_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), EXPERIENCE_TYPE_STYLE))
#define EXPERIENCE_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), EXPERIENCE_TYPE_STYLE, eXperienceStyleClass))

struct _eXperienceStyle
{
  GtkStyle parent_instance;
};

struct _eXperienceStyleClass
{
  GtkStyleClass parent_class;
};

void experience_style_register_type (GTypeModule *module);
