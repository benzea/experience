/* arch-tag: ef126cd3-9cee-42c8-9bea-9a35dda8db85 */

/*  eXperience GTK engine: filter.h
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

#ifndef __FILTER_H
#define __FILTER_H

#include <glib.h>
#include <gtk/gtk.h>
#include "utils.h"
#include "dynamic_color.h"

typedef enum {
	FILTER_NONE = 0,
	FILTER_SATURATION   = 1 << 0,
	FILTER_BRIGHTNESS   = 1 << 1,
	FILTER_OPACITY      = 1 << 2,
	FILTER_PIXELATE     = 1 << 3,
	FILTER_ROTATE       = 1 << 4,
	FILTER_MIRROR       = 1 << 5,
	FILTER_RECOLOR      = 1 << 6,
} eXperienceFilterList;

#define FILTER_ALL (FILTER_SATURATION | FILTER_BRIGHTNESS | FILTER_OPACITY | FILTER_PIXELATE | FILTER_ROTATE | FILTER_MIRROR | FILTER_RECOLOR)

typedef enum {
	RECOLOR_NONE,
	RECOLOR_SIMPLE_REPLACE,
	RECOLOR_RGB_GRADIENT
} eXperienceRecolorMode;

struct _filter {
	eXperienceFilterList allowed;
	eXperienceFilterList defined;

	gfloat saturation;
	gfloat brightness; 
	gfloat opacity;
	gboolean pixelate;
	
	eXperienceRoundingMethod rounding;
	
	eXperienceOrientation mirror;
	eXperienceRotate rotation;
	
	eXperienceRecolorMode recolor_mode;
	GHashTable * recolor_colors;
	
	gchar * info_string;
};
typedef struct _filter eXperienceFilter;


void experience_filter_init (eXperienceFilter * filter, eXperienceFilterList allowed_filters);
void experience_filter_set_info_string (eXperienceFilter * filter, gchar * info_string);
guint experience_filter_hash (eXperienceFilter * filter, guint hash_value);
gboolean experience_filter_equal (eXperienceFilter * filter1, eXperienceFilter * filter2);
void experience_filter_inherit_from (eXperienceFilter * filter, eXperienceFilter * from);
void experience_filter_apply_filter (eXperienceFilter * filter, eXperienceFilter * from_filter);
void experience_filter_apply_group_filter (eXperienceFilter * filter, eXperienceFilter * group_filter);

void experience_filter_set_saturation (eXperienceFilter * filter, gfloat value);
void experience_filter_set_opacity    (eXperienceFilter * filter, gfloat value);
void experience_filter_set_brightness (eXperienceFilter * filter, gfloat value);
void experience_filter_set_pixelate   (eXperienceFilter * filter, gboolean value);

void experience_filter_add_recolor_color (eXperienceFilter * filter, GdkColor color, eXperienceDynamicColor recolor_color);
void experience_filter_set_recolor_gradient (eXperienceFilter * filter, eXperienceDynamicColor red, eXperienceDynamicColor green, eXperienceDynamicColor blue);

void experience_filter_add_mirror (eXperienceFilter * filter, eXperienceOrientation mirror);
void experience_filter_set_rotation (eXperienceFilter * filter, eXperienceRotate rotation);

/*-------*/

GdkPixbuf * experience_apply_filters (GdkPixbuf * experience, eXperienceFilter * filter);

void experience_apply_filter_to_color (GdkColor * color, eXperienceFilter * filter);

void retrive_recolor_colors (eXperienceFilter * filter, GtkStyle * style);

GdkColor * experience_filter_get_recolor_color (eXperienceFilter * filter, GdkColor * color);

void experience_filter_finalize (eXperienceFilter * filter);
void experience_filter_copy (eXperienceFilter * dest, eXperienceFilter * source);

#endif /* __FILTER_H */
