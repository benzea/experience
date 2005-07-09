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

static inline guint32
get_color_value (GdkColor * color, guint8 alpha)
{
	guint32 result = alpha;
	
	/* First remove the lower 8 bits, and after that move it at the correct place in the guint24. */
	result += (color->red   >> 8) << 24;
	result += (color->green >> 8) << 16;
	result += (color->blue  >> 8) <<  8;
	
	return result;
}

static gboolean
draw_begin (eXperienceDrawable * drawable, GtkStyle * style, gpointer * tmp_data, gint * width, gint * height, gboolean * fail)
{
	eXperienceFill * fill = (eXperienceFill*) drawable;
	
	g_assert (drawable  != NULL);
	g_assert (style     != NULL);
	g_assert (width     != NULL);
	g_assert (height    != NULL);
	g_assert (fail      != NULL);
	
	*width  = 0;
	*height = 0;
	
	if (!fill->color_set) return FALSE;
	
	experience_dynamic_color_update (&fill->color, style);
	
	return TRUE;
}

typedef struct {
	eXperienceFill * fill;
	GdkPixbuf * pixbuf;
	gint width;
	gint height;
} get_image_info;

static GdkPixbuf *
get_image_from_info (gpointer info_ptr)
{
	GdkColor color;
	get_image_info * info = info_ptr;
	guint32 pixel;
	
	if (info->pixbuf != NULL) return info->pixbuf;
	
	info->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, info->width, info->height);
	
	color = info->fill->color.output_color;
	
	experience_apply_filter_to_color (&color, &info->fill->drawable.filter);
	
	pixel = get_color_value (&color, info->fill->drawable.filter.opacity * 0xFF);
	
	gdk_pixbuf_fill (info->pixbuf, pixel);
	
	return info->pixbuf;
}

static gboolean
draw (eXperienceDrawable * drawable, gpointer tmp_data, GdkPixbuf * dest, GdkRectangle * dest_area, GdkRectangle * clip_area, GdkRegion * dirty_region)
{
	eXperienceFill * fill = (eXperienceFill*) drawable;
	GdkPixbuf * sub_pixbuf;
	guint32 pixel;
	GdkColor color;
	GdkRectangle real_dest_area = {0,0,0,0};
	
	g_assert (drawable   != NULL);
	g_assert (dest_area  != NULL);
	g_assert (drawable->class == experience_fill_class);
	
	if (drawable->filter.opacity == 1.0) {
		real_dest_area.width  = gdk_pixbuf_get_width  (dest);
		real_dest_area.height = gdk_pixbuf_get_height (dest);
	
		gdk_rectangle_intersect (dest_area, &real_dest_area, &real_dest_area);
		gdk_rectangle_intersect (clip_area, &real_dest_area, &real_dest_area);
	
		if ((real_dest_area.width > 0) && (real_dest_area.height > 0)) {
			/* opacity is 1. Just fill the region. */
			sub_pixbuf = gdk_pixbuf_new_subpixbuf (dest, real_dest_area.x, real_dest_area.y, real_dest_area.width, real_dest_area.height);
			
			/* update dirty area */
			if (dirty_region != NULL)
				gdk_region_union_with_rect (dirty_region, &real_dest_area);
			
			color = fill->color.output_color;
			
			experience_apply_filter_to_color (&color, &drawable->filter);
			
			pixel = get_color_value (&color, 0xFF);
			
			gdk_pixbuf_fill (sub_pixbuf, pixel);
			
			g_object_unref ((GObject*) sub_pixbuf);
		}
	} else {
		if (drawable->filter.opacity > 0) {
			/* opacity is between 0 and 1. We need to create a temporary pixmap, and composite it. */
			get_image_info * paint_data = tmp_data;
			
			if (paint_data == NULL) {
				paint_data = g_new0 (get_image_info, 1);
				paint_data->fill = fill;
				paint_data->width  = dest_area->width;
				paint_data->height = dest_area->height;
			}
			
			experience_pixbuf_composite (dest, dest_area, clip_area, dirty_region, get_image_from_info, paint_data);
		}
	}
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

static gboolean
draw_end (eXperienceDrawable * drawable, gpointer tmp_data)
{
	get_image_info * paint_data = tmp_data;
	
	if (paint_data != NULL) {
		if (paint_data->pixbuf != NULL)
			g_object_unref (paint_data);
		
		g_free (paint_data);
	}
	return TRUE;
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
	_experience_fill_class.create     = create;
	_experience_fill_class.destroy    = destroy;
	_experience_fill_class.draw_begin = draw_begin;
	_experience_fill_class.draw       = draw;
	_experience_fill_class.draw_end   = draw_end;
	_experience_fill_class.inherit_from_drawable = inherit_from_drawable;
	_experience_fill_class.apply_group_settings  = apply_group_settings;
	
	experience_fill_class = &_experience_fill_class;
}

