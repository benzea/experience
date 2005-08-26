/* arch-tag: 458e5939-3e2f-4398-83f5-3e6fc230e6e2 */

/*  eXperience GTK engine: fill.c
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

#include "fill.h"

void
experience_fill_set_color (eXperienceFill * fill, eXperienceDynamicColor color)
{
	g_assert (fill != NULL);
	g_assert (fill->drawable.class == experience_fill_class);
	
	if (!fill->color_set) {
		fill->color_set = TRUE;
		fill->color = color;
	} else g_printerr ("Color set more than once in fill %i in group %s!\n", fill->drawable.number, fill->drawable.group_name);
}

/*#######*/
static eXperienceDrawable *
create (void)
{
	eXperienceFill * new_fill;
	new_fill = g_new0 (eXperienceFill, 1);
	
	experience_filter_init (&new_fill->drawable.filter, FILTER_SATURATION | FILTER_BRIGHTNESS | FILTER_OPACITY);
	
	return (eXperienceDrawable*) new_fill;
}

static void
destroy (eXperienceDrawable * drawable)
{
	eXperienceFill * fill = (eXperienceFill* )drawable;
	
	g_assert (drawable != NULL);
	
	g_free (fill);
}

typedef struct {
	eXperienceFill * fill;
	GdkPixbuf * pixbuf;
	gint width;
	gint height;
} get_image_info;

static gboolean
draw (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	eXperienceFill * fill = (eXperienceFill*) drawable;
	GdkColor color;
	
	g_assert (drawable   != NULL);
	g_assert (dest_size  != NULL);
	g_assert (drawable->class == experience_fill_class);
	
	/* get the color */
	experience_dynamic_color_update (&fill->color, style);
	color = fill->color.output_color;
	experience_apply_filter_to_color (&color, &drawable->filter);
	
	/* rectangle */
	cairo_rectangle (cr, 0, 0, dest_size->width, dest_size->height);
	
	cairo_set_source_rgba (cr, color.red   / ((double) G_MAXUINT16),
	                           color.green / ((double) G_MAXUINT16),
	                           color.blue  / ((double) G_MAXUINT16),
	                           drawable->filter.opacity);
	
	cairo_fill (cr);
	
	return TRUE;
}

static void
inherit_from_drawable (eXperienceDrawable * drawable, eXperienceDrawable * from)
{
	eXperienceFill * fill = (eXperienceFill*) drawable;
	eXperienceFill * fill_from = (eXperienceFill*) from;
	
	g_assert (drawable != NULL);
	g_assert (from != NULL);
	g_assert (drawable->class == experience_fill_class);
	g_assert (from->class == experience_fill_class);
	
	if (!fill->color_set) {
		fill->color_set = fill_from->color_set;
		fill->color = fill_from->color;
	}
}

static void
apply_group_settings (eXperienceDrawable * drawable, eXperienceGroup * group)
{
	return;
}

static eXperienceDrawableClass _experience_fill_class;

void
experience_fill_init_class (void)
{
	_experience_fill_class.object_type = "fill";
	_experience_fill_class.create      = create;
	_experience_fill_class.destroy     = destroy;
	_experience_fill_class.get_info    = NULL;
	_experience_fill_class.draw        = draw;
	_experience_fill_class.inherit_from_drawable = inherit_from_drawable;
	_experience_fill_class.apply_group_settings  = apply_group_settings;
	
	experience_fill_class = &_experience_fill_class;
}

