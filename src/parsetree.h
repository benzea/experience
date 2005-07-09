/* arch-tag: b13e2cc1-2ada-45bf-9b1e-b043dd2440da */

/*  eXperience GTK engine: parsetree.h
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

#ifndef __PARSETREE_H
#define __PARSETREE_H

#include <glib.h>
#include <gdk/gdkcolor.h>
#include "experience.h"
#include "drawable.h"
#include "group.h"
#include "rcstyle.h"
#include "image.h"

struct _parsestate {
  GScanner * scanner;
  GtkSettings * settings;
  
  int should_end;

  GHashTable * groups;
  eXperienceGroup    * current_group;
  eXperienceDrawable * current_drawable;
  eXperienceFilter   * current_filter;

  eXperienceRcStyle * rcstyle;
  
  eXperienceGroup * gap_group[3];
  eXperienceImage * gap_image[3];
  
  gboolean show_warnings, warning_shown;
  
  guint expected_token;
};
typedef struct _parsestate parsestate;

void parse_begin_group (parsestate* pst, gchar* name, gchar* inherit, gboolean load);
void parse_end_group (parsestate* pst);

void parse_begin_drawable (parsestate* pst, gint image_number, eXperienceDrawableClass * class);
void parse_end_drawable (parsestate* pst);

void parse_begin_icons (parsestate* pst);
void parse_end_icons (parsestate* pst);

void parse_begin_icons_state (parsestate* pst, GtkStateType state);
void parse_end_icons_state (parsestate* pst);

void parsestate_init (parsestate * pst, GHashTable * experience_groups, GScanner* scanner);
void parsestate_destroy (parsestate* pst);

#endif /* __PARSETREE_H */
