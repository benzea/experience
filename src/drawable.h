/* arch-tag: ca7e2882-6c14-431f-8274-c5cd0f6883d0 */

/*  eXperience GTK engine: drawable.h
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

#ifndef __DRAWABLE_H
#define __DRAWABLE_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "data.h"
#include "filter.h"
#include "utils.h"

typedef struct _eXperienceDrawable eXperienceDrawable;
typedef struct _eXperienceDrawableClass eXperienceDrawableClass;

/* this is a cheap trick, to get the circular dependency working */
#include "group.h"

struct _eXperienceDrawableClass {
	gchar * object_type;
	
	eXperienceDrawable * (*create) (void);
	
	void (*inherit_from_drawable) (eXperienceDrawable * drawable, eXperienceDrawable * from);
	
	void (*apply_group_settings) (eXperienceDrawable * drawable, eXperienceGroup * group);
	
	void (*destroy) (eXperienceDrawable * drawable);
	
	void (*get_info) (eXperienceDrawable * drawable, GtkStyle * style, eXperienceSize * size);
	
	gboolean (*draw) (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style);
};

typedef struct _eXperienceDrawablePrivate eXperienceDrawablePrivate;

struct _eXperienceDrawable {
	eXperienceDrawableClass * class;
	
	gint number;
	
	eXperienceFilter filter;
	
	gchar * group_name;
	
	eXperienceDrawablePrivate * private;
};

eXperienceDrawable * experience_drawable_create (eXperienceDrawableClass * class, gint number, gchar * group_name);
eXperienceDrawable * experience_drawable_duplicate (eXperienceDrawable * drawable, gchar * group_name);

void experience_drawable_ref (eXperienceDrawable * drawable);
void experience_drawable_unref (eXperienceDrawable * drawable);

void experience_drawable_set_dont_inherit (eXperienceDrawable * drawable, gboolean dont_inherit);
void experience_drawable_set_dont_draw (eXperienceDrawable * drawable, gboolean dont_draw);
void experience_drawable_set_padding (eXperienceDrawable * drawable, gint left, gint right, gint top, gint bottom);

void experience_drawable_set_width (eXperienceDrawable * drawable, eXperiencePercent width);
void experience_drawable_set_height (eXperienceDrawable * drawable, eXperiencePercent height);
void experience_drawable_set_pos (eXperienceDrawable * drawable, eXperiencePercent pos, eXperienceOrientation orientation);

void experience_drawable_set_repeat (eXperienceDrawable * drawable, guint left, guint right, guint top, guint bottom);
void experience_drawable_set_inner_padding (eXperienceDrawable * drawable, guint left, guint right, guint top, guint bottom);
void experience_drawable_set_rounding (eXperienceDrawable * drawable, eXperienceRoundingMethod rounding);
void experience_drawable_set_draw_entire_only (eXperienceDrawable * drawable, gboolean draw_entire_only);
void experience_drawable_set_dont_clip (eXperienceDrawable * drawable, gboolean dont_clip);


void experience_drawable_apply_group_settings (eXperienceDrawable * drawable, eXperienceGroup * group);
void experience_drawable_inherit_from (eXperienceDrawable * drawable, eXperienceDrawable * from);
gboolean experience_drawable_draw (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style);

#endif /* __DRAWABLE_H */
