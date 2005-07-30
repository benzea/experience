/* arch-tag: 133e1038-9ad8-4b2c-9b01-85800a39b637 */

/*  eXperience GTK engine: filter.c
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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "experience.h"
#include "drawable.h"
#include "utils.h"

#define filter_warning(filter, msg) { \
	if (filter->info_string != NULL) { \
		experience_warning ("In filter for %s: %s\n", filter->info_string, msg); \
	} else { \
		experience_warning ("In unkown filter: %s\n", msg); \
	} \
} \

#define CHECK_AND_SET(FILTER, msg_cant_set, msg_already_set) { \
	if (filter->defined & FILTER) { \
		filter_warning (filter, msg_already_set); \
		return; \
	} else { \
		if (!(filter->allowed & FILTER)) { \
			filter_warning (filter, msg_cant_set); \
		} else { \
			filter->defined |= FILTER; \
		} \
	} \
}

static guint
full_hash_g_color (GdkColor * color, guint init)
{
	return hash_mem (&color->red, 3 * sizeof(guint16), init);
}

static guint
hash_g_color (GdkColor * color)
{
	return full_hash_g_color (color, 0);
}

static gboolean
equal_g_color (GdkColor * color1, GdkColor * color2)
{
	g_assert (color1 != NULL);
	g_assert (color2 != NULL);
	
	if (color1->red   != color2->red)   return FALSE;
	if (color1->green != color2->green) return FALSE;
	if (color1->blue  != color2->blue)  return FALSE;
	
	return TRUE;
}

void
experience_filter_init (eXperienceFilter * filter, eXperienceFilterList allowed_filters)
{
	g_assert (filter != NULL);
	
	filter->defined = FILTER_NONE;
	filter->saturation = 1;
	filter->pixelate   = FALSE;
	filter->opacity    = 1;
	filter->brightness = 0;
	
	filter->recolor_mode = RECOLOR_NONE;
	filter->rotation = ROTATE_NONE;
	filter->allowed = allowed_filters;
	
	filter->recolor_colors = g_hash_table_new_full ((GHashFunc) hash_g_color, (GEqualFunc) equal_g_color, g_free, g_free);
	
	filter->info_string = NULL;
}

void
experience_filter_set_info_string (eXperienceFilter * filter, gchar * info_string)
{
	g_assert (filter != NULL);
	
	if (filter->info_string != NULL) {
		g_free (filter->info_string);
	}
	filter->info_string = g_strdup (info_string);
}

static void
hash_recolor_color (GdkColor * key, eXperienceDynamicColor * color, guint * init)
{
	g_assert (color != NULL);
	
	*init = full_hash_g_color (&color->output_color, *init);
	
	*init = hash_mem (&color->source, 1, *init);
}

guint
experience_filter_hash (eXperienceFilter * filter, guint hash_value)
{
	g_assert (filter != NULL);
	
	hash_value = hash_mem (&filter->saturation, sizeof(gfloat), hash_value);
	hash_value = hash_mem (&filter->brightness, sizeof(gfloat), hash_value);
	hash_value = hash_mem (&filter->opacity,    sizeof(gfloat), hash_value);
	hash_value = hash_mem (&filter->pixelate,   sizeof(gboolean), hash_value);
	
	hash_value = hash_mem (&filter->rotation,   sizeof(filter->rotation), hash_value);
	hash_value = hash_mem (&filter->mirror,     sizeof(filter->mirror), hash_value);
	
	hash_value = hash_mem (&filter->recolor_mode, sizeof(eXperienceRecolorMode), hash_value);
	
	g_hash_table_foreach (filter->recolor_colors, (GHFunc) hash_recolor_color, &hash_value);
	
	return hash_value;
}

struct _recolor_equal {
	gboolean result;
	GHashTable * other;
};
typedef struct _recolor_equal recolor_equal;

static void
equal_recolor_color (GdkColor * key, eXperienceDynamicColor * color1, recolor_equal * data)
{
	eXperienceDynamicColor * color2;
	g_assert (color1 != NULL);
	
	color2 = g_hash_table_lookup (data->other, key);
	
	if (color2 == NULL) {
		data->result = FALSE;
	} else {
		/* THIS REQUIRES output_color TO BE SET! */
		if (!equal_g_color (&color1->output_color, &color2->output_color)) data->result = FALSE;
	}
}

gboolean
experience_filter_equal (eXperienceFilter * filter1, eXperienceFilter * filter2)
{
	recolor_equal data;
	
	g_assert (filter1 != NULL);
	g_assert (filter2 != NULL);
	
	if (filter1->defined != filter2->defined) return FALSE;
	if (filter1->saturation      != filter2->saturation)      return FALSE;
	if (filter1->brightness      != filter2->brightness)      return FALSE;
	if (filter1->opacity         != filter2->opacity)         return FALSE;
	if (filter1->pixelate        != filter2->pixelate)        return FALSE;
	
	if (filter1->rotation        != filter2->rotation)        return FALSE;
	if (filter1->mirror          != filter2->mirror)          return FALSE;
	
	data.result = TRUE;
	data.other  = filter2->recolor_colors;
	
	g_hash_table_foreach (filter1->recolor_colors, (GHFunc) equal_recolor_color, &data);
	
	return data.result;
}

static void
copy_recolor_colors (GdkColor * color, eXperienceDynamicColor * recolor_color, GHashTable * dest)
{
	GdkColor * new_color;
	eXperienceDynamicColor * new_recolor_color;
	
	new_color = g_new (GdkColor, 1);
	new_recolor_color = g_new (eXperienceDynamicColor, 1);
	
	*new_color = *color;
	*new_recolor_color = *recolor_color;
	
	g_hash_table_insert (dest, new_color, new_recolor_color);
}

void
experience_filter_inherit_from (eXperienceFilter * filter, eXperienceFilter * from)
{
	eXperienceFilterList not_defined_filter;
	
	g_assert (filter != NULL);
	g_assert (from   != NULL);
	
	/* only inherit filters that are not defined, and allowed */
	not_defined_filter = (~filter->defined) & filter->allowed;
	
	if (not_defined_filter & FILTER_SATURATION)   filter->saturation   = from->saturation;
	if (not_defined_filter & FILTER_BRIGHTNESS)   filter->brightness   = from->brightness;
	if (not_defined_filter & FILTER_OPACITY)      filter->opacity      = from->opacity;
	if (not_defined_filter & FILTER_PIXELATE)     filter->pixelate     = from->pixelate;
	
	if (not_defined_filter & FILTER_ROTATE)       filter->rotation     = from->rotation;
	if (not_defined_filter & FILTER_MIRROR)       filter->mirror       = from->mirror;
	
	if ((filter->allowed & FILTER_RECOLOR) && (filter->recolor_mode == RECOLOR_NONE) && (from->recolor_mode != RECOLOR_NONE)) {
		filter->recolor_mode = from->recolor_mode;
		g_hash_table_foreach (from->recolor_colors, (GHFunc) copy_recolor_colors, filter->recolor_colors);
	}
	
	filter->defined = (filter->defined | from->defined) & filter->allowed;
}

void
experience_filter_apply_filter (eXperienceFilter * filter, eXperienceFilter * from_filter)
{
	eXperienceOrientation tmp_mirror;
	
	g_assert (filter != NULL);
	g_assert (from_filter != NULL);
	
	filter->saturation *= from_filter->saturation;
	filter->opacity    *= from_filter->opacity;
	filter->brightness += from_filter->brightness;
	
	if ((from_filter->rotation == ROTATE_CW) ||
	    (from_filter->rotation == ROTATE_CCW)) {
		tmp_mirror  = (filter->mirror & ORIENTATION_HORIZONTAL) ? ORIENTATION_VERTICAL   : 0;
		tmp_mirror |= (filter->mirror & ORIENTATION_VERTICAL)   ? ORIENTATION_HORIZONTAL : 0;
		filter->mirror = tmp_mirror;
	}
	filter->mirror = filter->mirror ^ from_filter->mirror;
	
	filter->rotation = (filter->rotation + from_filter->rotation) % 4;
}

void
experience_filter_apply_group_filter (eXperienceFilter * filter, eXperienceFilter * group_filter)
{
	g_assert (filter != NULL);
	g_assert (group_filter != NULL);
	
	filter->saturation   *= group_filter->saturation;
	filter->brightness   += group_filter->brightness;
}

void
experience_filter_set_saturation (eXperienceFilter * filter, gfloat value)
{
	g_assert (filter != NULL);
	
	CHECK_AND_SET (FILTER_SATURATION, "Saturation can't be used in this context!", "Tried to set the saturation more than once!");
	
	filter->saturation = value;
}

void
experience_filter_set_opacity    (eXperienceFilter * filter, gfloat value)
{
	g_assert (filter != NULL);
	
	CHECK_AND_SET (FILTER_OPACITY, "Opacity can't be used in this context!", "Tried to set the opacity more than once!");
	
	filter->opacity = CLAMP (value, 0, 1);
}

void
experience_filter_set_brightness (eXperienceFilter * filter, gfloat value)
{
	g_assert (filter != NULL);
	
	CHECK_AND_SET (FILTER_BRIGHTNESS, "Brightness can't be used in this context!", "Tried to set the brightness more than once!");
	
	filter->brightness = CLAMP (value, -1, 1);
}

void
experience_filter_set_pixelate   (eXperienceFilter * filter, gboolean value)
{
	g_assert (filter != NULL);
	
	CHECK_AND_SET (FILTER_PIXELATE, "Pixelate can't be used in this context!", "Tried to set pixelate more than once!");
	
	filter->pixelate = value;
}

void experience_filter_add_recolor_color (eXperienceFilter * filter, GdkColor color, eXperienceDynamicColor recolor_color)
{
	eXperienceDynamicColor * new_recolor_color;
	GdkColor * new_color;
	
	g_assert (filter != NULL);
	
	if (!(filter->allowed & FILTER_RECOLOR)) {
		experience_warning ("You can't use recoloring in this context!\n");
		return;
	}
	
	if (filter->recolor_mode == RECOLOR_RGB_GRADIENT) {
		experience_warning ("Tried to set a recolor color, but there is already a gradient to be recolored!\n");
		return;
	}
	
	if (g_hash_table_lookup (filter->recolor_colors, &color) != NULL) {
		experience_warning ("Tried to assigne multiple recolor colors to one color!\n");
		return;
	}
	
	filter->recolor_mode = RECOLOR_SIMPLE_REPLACE;
	new_recolor_color = g_new (eXperienceDynamicColor, 1);
	new_color = g_new (GdkColor, 1);
	
	*new_recolor_color = recolor_color;
	*new_color = color;
	
	g_hash_table_insert (filter->recolor_colors, new_color, new_recolor_color);
}

void experience_filter_set_recolor_gradient (eXperienceFilter * filter, eXperienceDynamicColor red, eXperienceDynamicColor green, eXperienceDynamicColor blue)
{
	eXperienceDynamicColor * new_recolor_color;
	GdkColor * new_color;
	
	g_assert (filter != NULL);
	
	if (!(filter->allowed & FILTER_RECOLOR)) {
		experience_warning ("You can't use recoloring in this context!\n");
		return;
	}
	
	if (filter->recolor_mode != RECOLOR_NONE) {
		experience_warning ("Tried to set a \"recolor_gradient\" but some other recoloring was already set!\n");
		return;
	}
	
	filter->recolor_mode = RECOLOR_RGB_GRADIENT;
	
	new_recolor_color = g_new (eXperienceDynamicColor, 1);
	new_color = g_new0 (GdkColor, 1);
	
	new_color->red     = GDK_COLOR_MAX;
	*new_recolor_color = red;
	
	g_hash_table_insert (filter->recolor_colors, new_color, new_recolor_color);
	
	
	new_recolor_color = g_new (eXperienceDynamicColor, 1);
	new_color = g_new0 (GdkColor, 1);
	
	new_color->green   = GDK_COLOR_MAX;
	*new_recolor_color = green;
	
	g_hash_table_insert (filter->recolor_colors, new_color, new_recolor_color);
	
	
	new_recolor_color = g_new (eXperienceDynamicColor, 1);
	new_color = g_new0 (GdkColor, 1);
	
	new_color->blue    = GDK_COLOR_MAX;
	*new_recolor_color = blue;
	
	g_hash_table_insert (filter->recolor_colors, new_color, new_recolor_color);
}

void experience_filter_add_mirror (eXperienceFilter * filter, eXperienceOrientation mirror)
{
	g_assert (filter != NULL);
	
	if (!(filter->allowed & FILTER_MIRROR)) {
		experience_warning ("You can't use mirroring in this context!\n");
		return;
	}
	
	filter->defined |= FILTER_MIRROR;
	filter->mirror = filter->mirror ^ mirror;
}

void experience_filter_set_rotation (eXperienceFilter * filter, eXperienceRotate rotation)
{
	g_assert (filter != NULL);
	
	if (!(filter->allowed & FILTER_ROTATE)) {
		experience_warning ("You can't use ration in this context!\n");
		return;
	}
	
	if (!(filter->defined & FILTER_ROTATE)) {
		filter->defined |= FILTER_ROTATE;
		
		filter->rotation = rotation;
	} else experience_warning("Tried to set rotation more than once!\n");
}

/*------------*/


static void
experience_recolor_pixbuf (GdkPixbuf * pixbuf, eXperienceFilter * filter)
{
	GdkColor in, * out;
	gint width, height, rowstride, n_channels;
	guchar *row, *pixel;
	gint x, y;
	
	g_return_if_fail (pixbuf != NULL);
	g_return_if_fail (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8); /* we can only handle one byte per sample */
	g_return_if_fail (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	g_return_if_fail ((n_channels == 3) || (n_channels == 4)); /*RGB || RGBA*/
	
	width  = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	row = gdk_pixbuf_get_pixels (pixbuf);
	
	for (y = 0; y < height; y++) {
		pixel = row;
		for (x = 0; x < width; x++) {
			in.red   = pixel[RED_OFFSET]   * (GDK_COLOR_MAX / 255);
			in.green = pixel[GREEN_OFFSET] * (GDK_COLOR_MAX / 255);
			in.blue  = pixel[BLUE_OFFSET]  * (GDK_COLOR_MAX / 255);
			
			out = experience_filter_get_recolor_color (filter, &in);
			
			if (out != NULL) {
				pixel[RED_OFFSET]   = out->red   / (GDK_COLOR_MAX / 255);
				pixel[GREEN_OFFSET] = out->green / (GDK_COLOR_MAX / 255);
				pixel[BLUE_OFFSET]  = out->blue  / (GDK_COLOR_MAX / 255);
			}
			
			pixel += n_channels;
		}
		row += rowstride;
	}
}


GdkPixbuf *
experience_apply_filters (GdkPixbuf * pixbuf, eXperienceFilter * filter)
{
	GdkPixbuf * target = pixbuf;
	
	g_assert(target != NULL);
	g_assert(filter != NULL);
	
	experience_recolor_pixbuf (target, filter);
	
	target = experience_mirror (target, filter->mirror);
	
	if (filter->rotation != ROTATE_NONE)
		target = experience_rotate (target, filter->rotation);
	
	target = experience_change_pixbuf_opacity (target, filter->opacity);
	
	experience_set_pixbuf_brightness     (target, filter->brightness);
	gdk_pixbuf_saturate_and_pixelate (target, target, filter->saturation, filter->pixelate);
	
	return target;
}

/* partialy taken from gdk (gdk_pixbuf_saturate_and_pixelate) */
#define SATURATE(v) ((1.0 - filter->saturation) * intensity + filter->saturation * (v))
void
experience_apply_filter_to_color (GdkColor * color, eXperienceFilter * filter)
{
	gfloat intensity;
	
	g_assert (color  != NULL);
	g_assert (filter != NULL);
	
	color->red   = CLAMP_COLOR (color->red   + filter->brightness * GDK_COLOR_MAX);
	color->green = CLAMP_COLOR (color->green + filter->brightness * GDK_COLOR_MAX);
	color->blue  = CLAMP_COLOR (color->blue  + filter->brightness * GDK_COLOR_MAX);
	
	intensity = color->red * 0.30 + color->green * 0.59 + color->blue * 0.11;
	
	color->red   = CLAMP_COLOR (SATURATE (color->red));
	color->green = CLAMP_COLOR (SATURATE (color->green));
	color->blue  = CLAMP_COLOR (SATURATE (color->blue));
}

static void
retrive_recolor_color (GdkColor * key, eXperienceDynamicColor * color, GtkStyle * style)
{
	experience_dynamic_color_update (color, style);
}

void
retrive_recolor_colors (eXperienceFilter * filter, GtkStyle * style)
{
	g_assert (filter != NULL);
	g_assert (style  != NULL);
	
	g_hash_table_foreach (filter->recolor_colors, (GHFunc) retrive_recolor_color, style);
}

static GdkColor *
get_recolor_color (eXperienceFilter * filter, GdkColor * color)
{
	eXperienceDynamicColor * new_color;
	
	g_assert (filter != NULL);
	g_assert (color  != NULL);
	
	new_color = g_hash_table_lookup (filter->recolor_colors, color);
	
	if (new_color == NULL) return NULL;
	
	/* Since this has to be the correct color (for hashing and comparing).
	   I can just use this, instead of doing a lookup. */
	return &new_color->output_color;
	
}

GdkColor *
experience_filter_get_recolor_color (eXperienceFilter * filter, GdkColor * color)
{
	GdkColor * tmp;
	GdkColor get_tmp = {0, 0, 0, 0};
	static GdkColor result; /* ok, I don't like this ... */
	
	g_assert (filter != NULL);
	g_assert (color  != NULL);
	
	if (filter->recolor_mode == RECOLOR_NONE) return NULL;
	
	if (filter->recolor_mode == RECOLOR_SIMPLE_REPLACE) {
		return get_recolor_color (filter, color);
	} else if (filter->recolor_mode == RECOLOR_RGB_GRADIENT) {
		get_tmp.red   = GDK_COLOR_MAX;
		get_tmp.green = 0;
		get_tmp.blue  = 0;
		tmp = get_recolor_color (filter, &get_tmp);
		
		if (tmp != NULL) {
			result.red   = CLAMP_COLOR (tmp->red   * ((gfloat)color->red / (gfloat)GDK_COLOR_MAX));
			result.green = CLAMP_COLOR (tmp->green * ((gfloat)color->red / (gfloat)GDK_COLOR_MAX));
			result.blue  = CLAMP_COLOR (tmp->blue  * ((gfloat)color->red / (gfloat)GDK_COLOR_MAX));
		} else {
			g_assert_not_reached ();
		}
		
		get_tmp.red   = 0;
		get_tmp.green = GDK_COLOR_MAX;
		tmp = get_recolor_color (filter, &get_tmp);

		if (tmp != NULL) {
			result.red   = CLAMP_COLOR (result.red   + tmp->red   * ((gfloat)color->green / (gfloat)GDK_COLOR_MAX));
			result.green = CLAMP_COLOR (result.green + tmp->green * ((gfloat)color->green / (gfloat)GDK_COLOR_MAX));
			result.blue  = CLAMP_COLOR (result.blue  + tmp->blue  * ((gfloat)color->green / (gfloat)GDK_COLOR_MAX));
		} else {
			g_assert_not_reached ();
		}
		
		get_tmp.green = 0;
		get_tmp.blue  = GDK_COLOR_MAX;
		tmp = get_recolor_color (filter, &get_tmp);
		
		if (tmp != NULL) {
			result.red   = CLAMP_COLOR (result.red   + tmp->red   * ((gfloat)color->blue / (gfloat)GDK_COLOR_MAX));
			result.green = CLAMP_COLOR (result.green + tmp->green * ((gfloat)color->blue / (gfloat)GDK_COLOR_MAX));
			result.blue  = CLAMP_COLOR (result.blue  + tmp->blue  * ((gfloat)color->blue / (gfloat)GDK_COLOR_MAX));
		} else {
			g_assert_not_reached ();
		}
		
		return &result;
	}	
	g_assert_not_reached ();
	
	return NULL; /* dummy return to prevent warning. */
}


void
experience_filter_finalize (eXperienceFilter * filter)
{
	g_assert (filter != NULL);
	g_assert (filter->recolor_colors != NULL);
	
	g_hash_table_destroy (filter->recolor_colors);
	filter->recolor_colors = NULL;
	
	if (filter->info_string != NULL)
		g_free (filter->info_string);
}

static void
copy_recolor_color (GdkColor * key, eXperienceDynamicColor * color, GHashTable * dest)
{
	GdkColor * new_key = g_new (GdkColor, 1);
	eXperienceDynamicColor * new_color = g_new (eXperienceDynamicColor, 1);
	
	*new_key = *key;
	*new_color = *color;
	
	g_hash_table_insert (dest, new_key, new_color);
}

void
experience_filter_copy (eXperienceFilter * dest, eXperienceFilter * source)
{
	g_assert (dest   != NULL);
	g_assert (source != NULL);
	g_assert (dest->  recolor_colors != NULL);
	g_assert (source->recolor_colors != NULL);
	
	dest->defined = source->defined;
	dest->saturation = source->saturation;
	dest->brightness = source->brightness;
	dest->opacity    = source->opacity;
	dest->pixelate   = source->pixelate;
	
	dest->mirror     = source->mirror;
	dest->rotation   = source->rotation;
	
	dest->recolor_mode = source->recolor_mode;
	
	g_hash_table_foreach (source->recolor_colors, (GHFunc) copy_recolor_color, dest->recolor_colors);
}

