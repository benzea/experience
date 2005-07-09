/* arch-tag: 0743f5ae-bbb1-4d30-aca2-750d80033982 */

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
	
	new_image->interp_type = GDK_INTERP_BILINEAR;
	
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
	eXperienceImage * image = (eXperienceImage*) drawable;
	eXperienceBorder border_tmp;
	
	g_assert (drawable != NULL);
	g_assert (drawable->class == experience_image_class);
	
	if (group->filter.mirror & ORIENTATION_HORIZONTAL) {
		border_tmp = image->border;
		
		image->border.right  = border_tmp.left;
		image->border.left   = border_tmp.right;
	}
	
	if (group->filter.mirror & ORIENTATION_VERTICAL) {
		border_tmp = image->border;
		
		image->border.top    = border_tmp.bottom;
		image->border.bottom = border_tmp.top;
	}
	
	switch (group->filter.rotation) {
		case ROTATE_CW:
			border_tmp = image->border;
			
			image->border.top    = border_tmp.left;
			image->border.right  = border_tmp.top;
			image->border.bottom = border_tmp.right;
			image->border.left   = border_tmp.bottom;
			break;
		case ROTATE_CCW:
			border_tmp = image->border;
			
			image->border.top    = border_tmp.right;
			image->border.right  = border_tmp.bottom;
			image->border.bottom = border_tmp.left;
			image->border.left   = border_tmp.top;
			break;
		case ROTATE_AROUND:
			border_tmp = image->border;
			
			image->border.top    = border_tmp.bottom;
			image->border.right  = border_tmp.left;
			image->border.bottom = border_tmp.top;
			image->border.left   = border_tmp.right;
			break;
		case ROTATE_NONE:
			break;
	}
}

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

typedef struct {
	GdkPixbuf * scaled_pixbuf[9], * loaded_pixbuf;
	GdkRectangle src_area[9];
	gint scaled_width[9], scaled_height[9];
	eXperienceBorder px_border;
	gboolean calculated_scaled_info;
} tmp_drawing_data;


static gboolean
draw_begin (eXperienceDrawable * drawable, GtkStyle * style, gpointer * tmp_data, gint * width, gint * height, gboolean * fail)
{
	eXperienceCacheImage cache_image;
	eXperienceImage * image = (eXperienceImage*) drawable;
	tmp_drawing_data * paint_data;
	
	g_assert (drawable != NULL);
	g_assert (style    != NULL);
	g_assert (width    != NULL);
	g_assert (height   != NULL);
	g_assert (fail     != NULL);
	
	cache_image.file   = image->file;
	cache_image.filter = drawable->filter;
	
	/* try to load the image ... */
	paint_data = g_new0 (tmp_drawing_data, 1);
	paint_data->loaded_pixbuf = experience_get_image_pixbuf (&cache_image, style);
	
	if (paint_data->loaded_pixbuf == NULL) {
		*fail = TRUE;
		g_free (paint_data);
		return FALSE;
	}
	
	*width  = gdk_pixbuf_get_width  (paint_data->loaded_pixbuf);
	*height = gdk_pixbuf_get_height (paint_data->loaded_pixbuf);
	
	/* make image border smaller if it doesn't fit on the image. */
	if (image->border.left + image->border.right >= *width) {
		g_printerr ("Image border (horizontal) of image #%i in group \"%s\" is too big!\n", drawable->number, drawable->group_name);
		image->border.left  =  *width / 2;
		image->border.right = (*width - 1) / 2;
	}
	if (image->border.top + image->border.bottom >= *height) {
		g_printerr ("Image border (vertical) of image #%i in group \"%s\" is too big!\n", drawable->number, drawable->group_name);
		image->border.top    =  *height / 2;
		image->border.bottom = (*height - 1) / 2;
	}
	
	paint_data->px_border = image->border;
	
	*tmp_data = (gpointer) paint_data;
	
	return TRUE;
}

const eXperienceComponents convert[9] = {
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

static void
calculate_scaled_info (eXperienceImage * image, tmp_drawing_data * paint_data, gint dst_width, gint dst_height)
{
	gint img_width, img_height;
	
	img_width  = gdk_pixbuf_get_width (paint_data->loaded_pixbuf);
	img_height = gdk_pixbuf_get_height (paint_data->loaded_pixbuf);
	
	/* make border smaller if it is to big. */
	if ((paint_data->px_border.left + paint_data->px_border.right) >= dst_width) {
		paint_data->px_border.left  = dst_width / 2;
		paint_data->px_border.right = (dst_width - 1) / 2;
	}
	if ((paint_data->px_border.top + paint_data->px_border.bottom) >= dst_height) {
		paint_data->px_border.top    = dst_height / 2;
		paint_data->px_border.bottom = (dst_height - 1) / 2;
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
	paint_data->scaled_width[NORTH]    = dst_width - paint_data->px_border.left - paint_data->px_border.right;
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
	paint_data->scaled_height[WEST]   = dst_height - paint_data->px_border.top - paint_data->px_border.bottom;
	
	paint_data->src_area[CENTER].x      = image->border.left;
	paint_data->src_area[CENTER].y      = image->border.top;
	paint_data->src_area[CENTER].width  = img_width - image->border.left - image->border.right;
	paint_data->src_area[CENTER].height = img_height - image->border.top - image->border.bottom;
	paint_data->scaled_width[CENTER]    = dst_width  - paint_data->px_border.left - paint_data->px_border.right;
	paint_data->scaled_height[CENTER]   = dst_height - paint_data->px_border.top  - paint_data->px_border.bottom;

	paint_data->src_area[EAST].x      = img_width - image->border.right;
	paint_data->src_area[EAST].y      = image->border.top;
	paint_data->src_area[EAST].width  = image->border.right;
	paint_data->src_area[EAST].height = img_height - image->border.top - image->border.bottom;
	paint_data->scaled_width[EAST]    = paint_data->px_border.right;
	paint_data->scaled_height[EAST]   = dst_height - paint_data->px_border.top - paint_data->px_border.bottom;
	
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
	paint_data->scaled_width[SOUTH]    = dst_width - paint_data->px_border.left - paint_data->px_border.right;
	paint_data->scaled_height[SOUTH]   = paint_data->px_border.bottom;

	paint_data->src_area[SOUTH_EAST].x      = img_width - image->border.right;
	paint_data->src_area[SOUTH_EAST].y      = img_height - image->border.bottom;
	paint_data->src_area[SOUTH_EAST].width  = image->border.right;
	paint_data->src_area[SOUTH_EAST].height = image->border.bottom;
	paint_data->scaled_width[SOUTH_EAST]    = paint_data->px_border.right;
	paint_data->scaled_height[SOUTH_EAST]   = paint_data->px_border.bottom;
	
	/*#############*/
	paint_data->calculated_scaled_info = TRUE;
}

typedef struct {
	tmp_drawing_data * paint_data;
	gint area;
	GdkInterpType interp_type;
} tmp_get_image_info;


static GdkPixbuf *
scale_image_part (gpointer info_ptr)
{
	GdkPixbuf * result;
	tmp_get_image_info * info = (tmp_get_image_info*) info_ptr;
	
	if (info->paint_data->scaled_pixbuf[info->area] != NULL) {
		return info->paint_data->scaled_pixbuf[info->area];
	}
	
/*	sub_pixbuf = gdk_pixbuf_new_subpixbuf (info->paint_data->loaded_pixbuf,
	                                       info->paint_data->src_area[info->area].x,
	                                       info->paint_data->src_area[info->area].y,
	                                       info->paint_data->src_area[info->area].width,
	                                       info->paint_data->src_area[info->area].height);
	
	result = experience_gdk_pixbuf_scale_simple_or_ref (sub_pixbuf,
	                                                    info->paint_data->scaled_width [info->area],
	                                                    info->paint_data->scaled_height[info->area],
	                                                    info->interp_type);
	*/
	
	result = experience_gdk_pixbuf_scale_or_ref (info->paint_data->loaded_pixbuf,
	                                             &info->paint_data->src_area[info->area],
  	                                             info->paint_data->scaled_width [info->area],
	                                             info->paint_data->scaled_height[info->area],
	                                             info->interp_type);
	
	info->paint_data->scaled_pixbuf[info->area] = result;
	
	return result;
}


static void
draw_image_part (eXperienceImage * image, tmp_drawing_data * paint_data, GdkPixbuf * dest, GdkRectangle * clip_area, GdkRegion * dirty_region, gint area, gint x_pos, gint y_pos)
{
	GdkRectangle dest_area;
	tmp_get_image_info get_image_info;
	
	if (image->draw_components & convert[area]) {
		dest_area.x = x_pos;
		dest_area.y = y_pos;
		dest_area.width  = paint_data->scaled_width [area];
		dest_area.height = paint_data->scaled_height[area];
		
		get_image_info.paint_data = paint_data;
		get_image_info.area = area;
		get_image_info.interp_type = image->interp_type;
			
		experience_pixbuf_composite (dest, &dest_area, clip_area, dirty_region, scale_image_part, &get_image_info);
	}
}

static gboolean
draw (eXperienceDrawable * drawable, gpointer tmp_data, GdkPixbuf * dest, GdkRectangle * dest_area, GdkRectangle * clip_area, GdkRegion * dirty_region)
{
	eXperienceImage * image = (eXperienceImage*) drawable;
	tmp_drawing_data * paint_data = (tmp_drawing_data*) tmp_data;
		
	g_assert (drawable != NULL);
	g_assert (paint_data != NULL);
	g_assert (dest     != NULL);
	
	if (!paint_data->calculated_scaled_info) {
		calculate_scaled_info (image, paint_data, dest_area->width, dest_area->height);
	}
	
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 NORTH_WEST, dest_area->x,
	                             dest_area->y);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 NORTH,      dest_area->x + (gint) paint_data->px_border.left,
	                             dest_area->y);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 NORTH_EAST, dest_area->x + dest_area->width - (gint) paint_data->px_border.right,
	                             dest_area->y);
		
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 WEST,   dest_area->x,
	                         dest_area->y + (gint) paint_data->px_border.top);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 CENTER, dest_area->x + (gint) paint_data->px_border.left,
	                         dest_area->y + (gint) paint_data->px_border.top);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 EAST,   dest_area->x + (gint) dest_area->width - (gint) paint_data->px_border.right,
	                         dest_area->y + (gint) paint_data->px_border.top);

	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 SOUTH_WEST, dest_area->x,
	                             dest_area->y + (gint) dest_area->height - (gint) paint_data->px_border.bottom);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 SOUTH,      dest_area->x + (gint) paint_data->px_border.left,
	                             dest_area->y + (gint) dest_area->height - (gint) paint_data->px_border.bottom);
	draw_image_part (image, paint_data, dest, clip_area, dirty_region,
	                 SOUTH_EAST, dest_area->x + (gint) dest_area->width  - (gint) paint_data->px_border.right,
	                             dest_area->y + (gint) dest_area->height - (gint) paint_data->px_border.bottom);

	return TRUE;
}

static gboolean
draw_end (eXperienceDrawable * drawable, gpointer tmp_data)
{
	gint i;
	tmp_drawing_data * paint_data = (tmp_drawing_data*) tmp_data;
	
	g_assert (paint_data != NULL);
	
	for (i = 0; i <= CENTER; i++) {
		if (paint_data->scaled_pixbuf[i] != NULL) {
			g_object_unref ((GObject*) paint_data->scaled_pixbuf[i]);
			paint_data->scaled_pixbuf[i] = NULL;
		}
	}
	
	g_free (paint_data);
	
	return TRUE;
}

static eXperienceDrawableClass _experience_image_class;

void
experience_image_init_class (void)
{
	_experience_image_class.object_type = "image";
	_experience_image_class.create     = create;
	_experience_image_class.destroy    = destroy;
	_experience_image_class.draw_begin = draw_begin;
	_experience_image_class.draw       = draw;
	_experience_image_class.draw_end   = draw_end;
	_experience_image_class.inherit_from_drawable = inherit_from_drawable;
	_experience_image_class.apply_group_settings  = apply_group_settings;
	
	experience_image_class = &_experience_image_class;
}

