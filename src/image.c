/*  eXperience GTK engine: image.c
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

#include <string.h>
#include "experience.h"
#include "image.h"
#include "utils.h"
#include "filter.h"
#include "image_loading.h"

#define CHECK_AND_SET_OPTION(image, option, message) { \
	if (image->options_set & option) { \
			g_printerr ("Error in image #%i in group %s: %s\n", image->drawable.number, image->drawable.group_name, message); \
			return; \
	} else image->options_set |= option; \
}

void
experience_image_set_file (eXperienceImage * image, gchar * filename)
{
	g_assert (image != NULL);
	g_assert (image->drawable.class == experience_image_class);
	
	CHECK_AND_SET_OPTION (image, OPTION_FILE, "Tried to set the filename of an image more than once!");
	
	image->file = filename;
}

void
experience_image_set_border (eXperienceImage * image, guint left, guint right, guint top, guint bottom)
{
	g_assert ( image != NULL);
	g_assert (image->drawable.class == experience_image_class);
	
	CHECK_AND_SET_OPTION (image, OPTION_BORDER, "Tried to set the border more than once!");
	
	image->border.top    = top;
	image->border.bottom = bottom;
	image->border.left   = left;
	image->border.right  = right;
}

void
experience_image_set_draw_components (eXperienceImage * image, eXperienceComponents draw_components)
{
	g_assert (image != NULL);
	g_assert (image->drawable.class == experience_image_class);
	
	CHECK_AND_SET_OPTION (image, OPTION_DRAW_COMPONENTS, "Tried to set draw_components more than once!");
	
	image->draw_components = draw_components;
}

void
experience_image_set_interp_type (eXperienceImage * image, GdkInterpType interp_type)
{
	g_assert (image != NULL);
	g_assert (image->drawable.class == experience_image_class);
	
	CHECK_AND_SET_OPTION (image, OPTION_INTERP_TYPE, "Tried to set interpolation_type more than once!");
	
	image->interp_type = interp_type;
}


/*###################################*/
/* class defs */

static eXperienceDrawable *
create (void)
{
	eXperienceImage * new_image = g_new0 (eXperienceImage, 1);
	
	new_image->draw_components = COMPONENT_ALL;
	
	new_image->interp_type = CAIRO_FILTER_GOOD;
	
	experience_filter_init (&new_image->drawable.filter, FILTER_ALL);
	
	return (eXperienceDrawable*) new_image;
}

static void
destroy (eXperienceDrawable * drawable)
{
	eXperienceImage * image = (eXperienceImage*) drawable;
	g_assert (drawable != NULL);
	g_assert (drawable->class == experience_image_class);
	
	g_free (image->file);
	g_free (image);
}

static void
inherit_from_drawable (eXperienceDrawable * drawable, eXperienceDrawable * from)
{
	eXperienceImage * image = (eXperienceImage*) drawable;
	eXperienceImage * from_image = (eXperienceImage*) from;
	
	g_assert (drawable != NULL);
	g_assert (from  != NULL);
	g_assert (drawable->class == experience_image_class);
	
	if (image->file == NULL) image->file = g_strdup(from_image->file);
	
	if (!(image->options_set & OPTION_BORDER))
		image->border = from_image->border;
	
	if (!(image->options_set & OPTION_DRAW_COMPONENTS))
		image->draw_components = from_image->draw_components;
	
	if (!(image->options_set & OPTION_INTERP_TYPE))
		image->interp_type = from_image->interp_type;
		
	image->options_set |= from_image->options_set;
}

static void
apply_group_settings (eXperienceDrawable * drawable, eXperienceGroup * group)
{
	return;}

enum {
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTH_EAST,
	NORTH_WEST,
	SOUTH_EAST,
	SOUTH_WEST,
	CENTER,
} eXperienceAreas;

static const eXperienceComponents convert[9] = {
	COMPONENT_NORTH,
	COMPONENT_SOUTH,
	COMPONENT_EAST,
	COMPONENT_WEST,
	COMPONENT_NORTH_EAST,
	COMPONENT_NORTH_WEST,
	COMPONENT_SOUTH_EAST,
	COMPONENT_SOUTH_WEST,
	COMPONENT_CENTER,
};

typedef struct {
	cairo_t * cr;
	eXperienceRawImage * source;
	eXperienceBorder px_border;
	GdkRectangle src_area[9];
	gint scaled_width[9], scaled_height[9];
} tmp_drawing_data;

static void
calculate_scaled_info (eXperienceImage * image, tmp_drawing_data * paint_data, eXperienceSize * dest_size)
{
	gint img_width, img_height;
	
	img_width  = paint_data->source->width;
	img_height = paint_data->source->height;
	
	paint_data->px_border = image->border;
	
	/* make border smaller if it is to big. */
	if ((paint_data->px_border.left + paint_data->px_border.right) >= dest_size->width) {
		paint_data->px_border.left  = dest_size->width / 2;
		paint_data->px_border.right = (dest_size->width - 1) / 2;
	}
	if ((paint_data->px_border.top + paint_data->px_border.bottom) >= dest_size->height) {
		paint_data->px_border.top    = dest_size->height / 2;
		paint_data->px_border.bottom = (dest_size->height - 1) / 2;
	}
	
	/*############*/
	paint_data->src_area[NORTH_WEST].x      = 0;
	paint_data->src_area[NORTH_WEST].y      = 0;
	paint_data->src_area[NORTH_WEST].width  = image->border.left;
	paint_data->src_area[NORTH_WEST].height = image->border.top;
	paint_data->scaled_width[NORTH_WEST]    = paint_data->px_border.left;
	paint_data->scaled_height[NORTH_WEST]   = paint_data->px_border.top;
	
	paint_data->src_area[NORTH].x      = image->border.left;
	paint_data->src_area[NORTH].y      = 0;
	paint_data->src_area[NORTH].width  = img_width - image->border.left - image->border.right;
	paint_data->src_area[NORTH].height = image->border.top;
	paint_data->scaled_width[NORTH]    = dest_size->width - paint_data->px_border.left - paint_data->px_border.right;
	paint_data->scaled_height[NORTH]   = paint_data->px_border.top;
	
	paint_data->src_area[NORTH_EAST].x      = img_width - image->border.right;
	paint_data->src_area[NORTH_EAST].y      = 0;
	paint_data->src_area[NORTH_EAST].width  = image->border.right;
	paint_data->src_area[NORTH_EAST].height = image->border.top;
	paint_data->scaled_width[NORTH_EAST]    = paint_data->px_border.right;
	paint_data->scaled_height[NORTH_EAST]   = paint_data->px_border.top;
	
	/*--*/
	
	paint_data->src_area[WEST].x      = 0;
	paint_data->src_area[WEST].y      = image->border.top;
	paint_data->src_area[WEST].width  = image->border.left;
	paint_data->src_area[WEST].height = img_height - image->border.top - image->border.bottom;
	paint_data->scaled_width[WEST]    = paint_data->px_border.left;
	paint_data->scaled_height[WEST]   = dest_size->height - paint_data->px_border.top - paint_data->px_border.bottom;
	
	paint_data->src_area[CENTER].x      = image->border.left;
	paint_data->src_area[CENTER].y      = image->border.top;
	paint_data->src_area[CENTER].width  = img_width - image->border.left - image->border.right;
	paint_data->src_area[CENTER].height = img_height - image->border.top - image->border.bottom;
	paint_data->scaled_width[CENTER]    = dest_size->width  - paint_data->px_border.left - paint_data->px_border.right;
	paint_data->scaled_height[CENTER]   = dest_size->height - paint_data->px_border.top  - paint_data->px_border.bottom;

	paint_data->src_area[EAST].x      = img_width - image->border.right;
	paint_data->src_area[EAST].y      = image->border.top;
	paint_data->src_area[EAST].width  = image->border.right;
	paint_data->src_area[EAST].height = img_height - image->border.top - image->border.bottom;
	paint_data->scaled_width[EAST]    = paint_data->px_border.right;
	paint_data->scaled_height[EAST]   = dest_size->height - paint_data->px_border.top - paint_data->px_border.bottom;
	
	/*--*/
	
	paint_data->src_area[SOUTH_WEST].x      = 0;
	paint_data->src_area[SOUTH_WEST].y      = img_height - image->border.bottom;
	paint_data->src_area[SOUTH_WEST].width  = image->border.left;
	paint_data->src_area[SOUTH_WEST].height = image->border.bottom;
	paint_data->scaled_width[SOUTH_WEST]    = paint_data->px_border.left;
	paint_data->scaled_height[SOUTH_WEST]   = paint_data->px_border.bottom;
	
	paint_data->src_area[SOUTH].x      = image->border.left;
	paint_data->src_area[SOUTH].y      = img_height - image->border.bottom;
	paint_data->src_area[SOUTH].width  = img_width - image->border.left - image->border.right;
	paint_data->src_area[SOUTH].height = image->border.bottom;
	paint_data->scaled_width[SOUTH]    = dest_size->width - paint_data->px_border.left - paint_data->px_border.right;
	paint_data->scaled_height[SOUTH]   = paint_data->px_border.bottom;

	paint_data->src_area[SOUTH_EAST].x      = img_width - image->border.right;
	paint_data->src_area[SOUTH_EAST].y      = img_height - image->border.bottom;
	paint_data->src_area[SOUTH_EAST].width  = image->border.right;
	paint_data->src_area[SOUTH_EAST].height = image->border.bottom;
	paint_data->scaled_width[SOUTH_EAST]    = paint_data->px_border.right;
	paint_data->scaled_height[SOUTH_EAST]   = paint_data->px_border.bottom;
	
	/*#############*/
}

static void
draw_image_part (tmp_drawing_data * paint_data, eXperienceImage * image, gint area, gint x_pos, gint y_pos)
{
	double scale_x, scale_y;
	cairo_surface_t * surface;
	cairo_pattern_t * pattern;
	cairo_matrix_t matrix;
	
	if ((image->draw_components & convert[area]) && ((paint_data->src_area[area].width > 0) && (paint_data->src_area[area].height > 0))) {
		cairo_save (paint_data->cr);
		
		scale_x = (double) paint_data->src_area[area].width  / (double) paint_data->scaled_width [area];
		scale_y = (double) paint_data->src_area[area].height / (double) paint_data->scaled_height[area];
		
		surface = experience_raw_image_get_surface (paint_data->source, &paint_data->src_area[area]);
		
		pattern = cairo_pattern_create_for_surface (surface);
		cairo_surface_destroy (surface);
		
		cairo_translate (paint_data->cr, x_pos, y_pos);
		
		cairo_set_source (paint_data->cr, pattern);
		
		cairo_matrix_init_scale (&matrix, scale_x, scale_y);
		cairo_pattern_set_matrix (pattern, &matrix);
		
		cairo_pattern_set_filter (pattern, image->interp_type);
		
		/* cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD); */
		cairo_rectangle (paint_data->cr, 0, 0, paint_data->scaled_width[area], paint_data->scaled_height[area]);
		cairo_fill (paint_data->cr);
		
		/* --- */
		cairo_pattern_destroy (pattern);
		
		cairo_restore (paint_data->cr);
	}
}

static void
get_info (eXperienceDrawable * drawable, GtkStyle * style, eXperienceSize * size)
{
	eXperienceImage * image = (eXperienceImage*) drawable;
	eXperienceCacheImage cache_image;
	eXperienceRawImage * source;
	
	g_assert (drawable  != NULL);
	g_assert (size != NULL);
	
	/* get source pixbuf */
	cache_image.file   = image->file;
	cache_image.filter = drawable->filter;
	
	source = experience_get_raw_image (&cache_image, style);
	
	if (source != NULL) {
		size->width  = source->width;
		size->height = source->height;
	}
}

/* This function gets called from experience_drawable_draw,
   currently it gets a new cairo surface. For details see experience_drawable_draw */
static gboolean
draw (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	eXperienceImage * image = (eXperienceImage*) drawable;
	eXperienceCacheImage cache_image;
	tmp_drawing_data paint_data;
	
	g_assert (drawable  != NULL);
	g_assert (dest_size != NULL);
	
	/* get source pixbuf */
	cache_image.file   = image->file;
	cache_image.filter = drawable->filter;
	
	paint_data.cr = cr;
	paint_data.source = experience_get_raw_image (&cache_image, style);
	
	if (paint_data.source == NULL) {
		return FALSE;
	}
	
	/* get the needed info */
	calculate_scaled_info (image, &paint_data, dest_size);
	
	/* and draw */
	draw_image_part (&paint_data, image, NORTH_WEST,
	                 0,
	                 0);
	draw_image_part (&paint_data, image, NORTH,
	                 (gint) paint_data.px_border.left,
	                 0);
	draw_image_part (&paint_data, image, NORTH_EAST,
	                 dest_size->width - (gint) paint_data.px_border.right,
	                 0);
	
	draw_image_part (&paint_data, image, WEST,
	                 0,
	                 (gint) paint_data.px_border.top);
	draw_image_part (&paint_data, image, CENTER,
	                 (gint) paint_data.px_border.left,
	                 (gint) paint_data.px_border.top);
	draw_image_part (&paint_data, image, EAST,
	                 (gint) dest_size->width - (gint) paint_data.px_border.right,
	                 (gint) paint_data.px_border.top);

	draw_image_part (&paint_data, image, SOUTH_WEST,
	                 0,
	                 (gint) dest_size->height - (gint) paint_data.px_border.bottom);
	draw_image_part (&paint_data, image, SOUTH,
	                 (gint) paint_data.px_border.left,
	                 (gint) dest_size->height - (gint) paint_data.px_border.bottom);
	draw_image_part (&paint_data, image, SOUTH_EAST,
	                 (gint) dest_size->width  - (gint) paint_data.px_border.right,
	                 (gint) dest_size->height - (gint) paint_data.px_border.bottom);

	return TRUE;
}

static eXperienceDrawableClass _experience_image_class;

void
experience_image_init_class (void)
{
	_experience_image_class.object_type = "image";
	_experience_image_class.create      = create;
	_experience_image_class.destroy     = destroy;
	_experience_image_class.get_info    = get_info;
	_experience_image_class.draw        = draw;
	_experience_image_class.inherit_from_drawable = inherit_from_drawable;
	_experience_image_class.apply_group_settings  = apply_group_settings;
	
	experience_image_class = &_experience_image_class;
}

