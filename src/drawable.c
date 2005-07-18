/* arch-tag: 5cb5730e-30fe-4714-a9aa-8ef4af2cb251 */

/*  eXperience GTK engine: drawable.c
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
#include <glib/gprintf.h>
#include "drawable.h"

#define CHECK_AND_SET_OPTION(drawable, FLAG, message) { \
	if (drawable->private->options_set & FLAG) { \
		g_printerr ("Error in %s #%i in group \"%s\": %s\n", drawable->class->object_type, drawable->number, drawable->group_name, message); \
		return; \
	} else drawable->private->options_set |= FLAG; \
}

typedef enum {
	OPTION_DONT_INHERIT = 1 << 0,
	OPTION_PADDING      = 1 << 1,
	OPTION_DONT_DRAW    = 1 << 2,
	OPTION_INNER_PADDING = 1 << 3,
	OPTION_WIDTH        = 1 << 4,
	OPTION_HEIGHT       = 1 << 5,
	OPTION_YPOS         = 1 << 6,
	OPTION_XPOS         = 1 << 7,
	OPTION_REPEAT       = 1 << 8,
	OPTION_DRAW_ENTIRE_ONLY = 1 << 9,
	OPTION_DONT_CLIP    = 1 << 10,
	OPTION_ROUNDING     = 1 << 11,
} eXperienceDrawableOptions;

struct _eXperienceDrawablePrivate {
	guint refcount;
	
	eXperienceDrawableOptions options_set;
	
	gboolean dont_inherit;
	gboolean dont_draw;
	
	eXperiencePadding padding;
	
	gboolean group_settings_applyed;

	eXperienceBorder repeat;
	
	eXperienceRoundingMethod rounding;
	
	eXperiencePadding inner_padding;
	
	eXperiencePercent width;
	eXperiencePercent height;
	
	eXperiencePercent xpos;
	eXperiencePercent ypos;
	
	gboolean draw_entire_only;
	gboolean dont_clip;
};

eXperienceDrawable *
experience_drawable_create (eXperienceDrawableClass * class, gint number, gchar * group_name)
{
	eXperienceDrawable * drawable;
	gchar buffer[500]; /* 500 _should_ be enough. */
	
	g_assert (class != NULL);
	g_assert (class->create != NULL);
	
	drawable = class->create ();
	g_return_val_if_fail (drawable != NULL, NULL);
	
	drawable->private = g_new0 (eXperienceDrawablePrivate, 1);
	
	drawable->private->refcount = 1;

	drawable->private->rounding = ROUND_NORMAL;
	
	drawable->private->width. widget = 1;
	drawable->private->height.widget = 1;
	
	drawable->private->repeat.left   = 1;
	drawable->private->repeat.right  = 1;
	drawable->private->repeat.top    = 1;
	drawable->private->repeat.bottom = 1;
	
	drawable->class = class;
	
	drawable->group_name = group_name;
	
	drawable->number = number;
	
	g_sprintf (buffer, "%s #%i in group \"%s\"", class->object_type, number, group_name);
	experience_filter_set_info_string (&drawable->filter, buffer);
	
	return drawable;
}


eXperienceDrawable *
experience_drawable_duplicate (eXperienceDrawable * drawable, gchar * group_name)
{
	eXperienceDrawable * new_drawable;
	
	g_assert (drawable != NULL);
	
	new_drawable = experience_drawable_create (drawable->class, drawable->number, group_name);
	
	experience_drawable_inherit_from (new_drawable, drawable);
	
	return new_drawable;
}

void
experience_drawable_ref (eXperienceDrawable * drawable)
{
	g_assert (drawable != NULL);
	
	drawable->private->refcount++;
}

void
experience_drawable_unref (eXperienceDrawable * drawable)
{
	g_assert (drawable != NULL);
	
	drawable->private->refcount--;
	
	if (drawable->private->refcount == 0) {
		/* destroy object */
		
		experience_filter_finalize (&drawable->filter);
		
		g_free (drawable->private);
		
		drawable->class->destroy (drawable);
	}
}

/*#############*/

void
experience_drawable_set_dont_inherit (eXperienceDrawable * drawable, gboolean dont_inherit)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_DONT_INHERIT, "dont_inherit set more than once!");
	
	drawable->private->dont_inherit = dont_inherit;
}

void
experience_drawable_set_dont_draw (eXperienceDrawable * drawable, gboolean dont_draw)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_DONT_DRAW, "dont_draw set more than once!");
	
	drawable->private->dont_draw = dont_draw;
}

void
experience_drawable_set_padding (eXperienceDrawable * drawable, gint left, gint right, gint top, gint bottom)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_PADDING, "Padding set more than once!");
	
	drawable->private->padding.left   = left;
	drawable->private->padding.right  = right;
	drawable->private->padding.top    = top;
	drawable->private->padding.bottom = bottom;
}

void
experience_drawable_set_width (eXperienceDrawable * drawable, eXperiencePercent width)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_WIDTH, "Tried to set the width of an drawable more than once!");
	
	drawable->private->width = width;
}

void
experience_drawable_set_height (eXperienceDrawable * drawable, eXperiencePercent height)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_HEIGHT, "Tried to set the height of an drawable more than once!");
	
	drawable->private->height = height;
}

void
experience_drawable_set_pos (eXperienceDrawable * drawable, eXperiencePercent pos, eXperienceOrientation orientation)
{
	g_assert (drawable != NULL);
	
	if (orientation & ORIENTATION_HORIZONTAL) {
		CHECK_AND_SET_OPTION (drawable, OPTION_XPOS, "Tried to set the positioning more than once!");
		
		drawable->private->xpos = pos;
	}
	
	if (orientation & ORIENTATION_VERTICAL) {
		CHECK_AND_SET_OPTION (drawable, OPTION_YPOS, "Tried to set the positioning more than once!");
		
		drawable->private->ypos = pos;
	}
}

void
experience_drawable_set_repeat (eXperienceDrawable * drawable, guint left, guint right, guint top, guint bottom)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_REPEAT, "Tried to set the repeat more than once!");
	
	drawable->private->repeat.top    = top;
	drawable->private->repeat.bottom = bottom;
	drawable->private->repeat.left   = left;
	drawable->private->repeat.right  = right;
}

void
experience_drawable_set_inner_padding (eXperienceDrawable * drawable, guint left, guint right, guint top, guint bottom)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_INNER_PADDING, "Tried to set inner_padding more than once!");
	
	drawable->private->inner_padding.top    = top;
	drawable->private->inner_padding.bottom = bottom;
	drawable->private->inner_padding.left   = left;
	drawable->private->inner_padding.right  = right;
}

void
experience_drawable_set_rounding (eXperienceDrawable * drawable, eXperienceRoundingMethod rounding)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_ROUNDING, "Tried to set the rounding method more than once!");
	
	drawable->private->rounding = rounding;
}

void
experience_drawable_set_draw_entire_only (eXperienceDrawable * drawable, gboolean draw_entire_only)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_DRAW_ENTIRE_ONLY, "Tried to set draw_entire_only more than once!");
	
	drawable->private->draw_entire_only = draw_entire_only;
}

void
experience_drawable_set_dont_clip (eXperienceDrawable * drawable, gboolean dont_clip)
{
	g_assert (drawable != NULL);
	
	CHECK_AND_SET_OPTION (drawable, OPTION_DONT_CLIP, "Tried to set dont_clip more than once!");
	
	drawable->private->dont_clip = dont_clip;
}



/*-------------*/

void
experience_drawable_apply_group_settings (eXperienceDrawable * drawable, eXperienceGroup * group)
{
	eXperiencePadding padding_tmp;
	eXperiencePadding inner_padding_tmp;
	eXperienceBorder  repeat_tmp;
	eXperiencePercent percent_tmp;
	
	g_assert (drawable != NULL);
	g_assert (group    != NULL);
	
	if (group->filter.mirror & ORIENTATION_HORIZONTAL) {
		padding_tmp = drawable->private->padding;
		
		drawable->private->padding.right = padding_tmp.left;
		drawable->private->padding.left  = padding_tmp.right;
		
		inner_padding_tmp = drawable->private->inner_padding;
		
		drawable->private->inner_padding.right  = inner_padding_tmp.left;
		drawable->private->inner_padding.left   = inner_padding_tmp.right;
		
		repeat_tmp = drawable->private->repeat;
		
		drawable->private->repeat.right  = repeat_tmp.left;
		drawable->private->repeat.left   = repeat_tmp.right;
		
		drawable->private->xpos.pixel  = -drawable->private->xpos.pixel;
		drawable->private->xpos.widget = -drawable->private->xpos.widget;
		drawable->private->xpos.object = -drawable->private->xpos.object;
	}
	
	if (group->filter.mirror & ORIENTATION_VERTICAL) {
		padding_tmp = drawable->private->padding;
		
		drawable->private->padding.top    = padding_tmp.bottom;
		drawable->private->padding.bottom = padding_tmp.top;
		
		inner_padding_tmp = drawable->private->inner_padding;
		
		drawable->private->inner_padding.top    = inner_padding_tmp.bottom;
		drawable->private->inner_padding.bottom = inner_padding_tmp.top;
		
		repeat_tmp = drawable->private->repeat;
		
		drawable->private->repeat.top    = repeat_tmp.bottom;
		drawable->private->repeat.bottom = repeat_tmp.top;
		
		drawable->private->ypos.pixel  = -drawable->private->ypos.pixel;
		drawable->private->ypos.widget = -drawable->private->ypos.widget;
		drawable->private->ypos.object = -drawable->private->ypos.object;
	}
	
	switch (group->filter.rotation) {
		case ROTATE_CW:
			padding_tmp = drawable->private->padding;
			
			drawable->private->padding.top    = padding_tmp.left;
			drawable->private->padding.right  = padding_tmp.top;
			drawable->private->padding.bottom = padding_tmp.right;
			drawable->private->padding.left   = padding_tmp.bottom;
			
			inner_padding_tmp = drawable->private->inner_padding;
			
			drawable->private->inner_padding.top    = inner_padding_tmp.left;
			drawable->private->inner_padding.right  = inner_padding_tmp.top;
			drawable->private->inner_padding.bottom = inner_padding_tmp.right;
			drawable->private->inner_padding.left   = inner_padding_tmp.bottom;
			
			repeat_tmp = drawable->private->repeat;
			
			drawable->private->repeat.top    = repeat_tmp.left;
			drawable->private->repeat.right  = repeat_tmp.top;
			drawable->private->repeat.bottom = repeat_tmp.right;
			drawable->private->repeat.left   = repeat_tmp.bottom;
			
			percent_tmp      = drawable->private->xpos;
			drawable->private->xpos.pixel    = -drawable->private->ypos.pixel;
			drawable->private->xpos.widget   = -drawable->private->ypos.widget;
			drawable->private->xpos.object   = -drawable->private->ypos.object;
			drawable->private->ypos          = percent_tmp;
			
			percent_tmp    = drawable->private->width;
			drawable->private->width         = drawable->private->height;
			drawable->private->height        = percent_tmp;
			break;
		case ROTATE_CCW:
			padding_tmp = drawable->private->padding;
			
			drawable->private->padding.top    = padding_tmp.right;
			drawable->private->padding.right  = padding_tmp.bottom;
			drawable->private->padding.bottom = padding_tmp.left;
			drawable->private->padding.left   = padding_tmp.top;
			
			inner_padding_tmp = drawable->private->inner_padding;
			
			drawable->private->inner_padding.top    = inner_padding_tmp.right;
			drawable->private->inner_padding.right  = inner_padding_tmp.bottom;
			drawable->private->inner_padding.bottom = inner_padding_tmp.left;
			drawable->private->inner_padding.left   = inner_padding_tmp.top;
			
			repeat_tmp = drawable->private->repeat;
			
			drawable->private->repeat.top    = repeat_tmp.right;
			drawable->private->repeat.right  = repeat_tmp.bottom;
			drawable->private->repeat.bottom = repeat_tmp.left;
			drawable->private->repeat.left   = repeat_tmp.top;			
			
			percent_tmp      = drawable->private->xpos;
			drawable->private->xpos          = drawable->private->ypos;
			drawable->private->ypos.pixel    = -percent_tmp.pixel;
			drawable->private->ypos.widget   = -percent_tmp.widget;
			drawable->private->ypos.object   = -percent_tmp.object;
			
			percent_tmp    = drawable->private->width;
			drawable->private->width         = drawable->private->height;
			drawable->private->height        = percent_tmp;
			break;
		case ROTATE_AROUND:
			padding_tmp = drawable->private->padding;
			
			drawable->private->padding.top    = padding_tmp.bottom;
			drawable->private->padding.right  = padding_tmp.left;
			drawable->private->padding.bottom = padding_tmp.top;
			drawable->private->padding.left   = padding_tmp.right;
			
			inner_padding_tmp = drawable->private->inner_padding;
			
			drawable->private->inner_padding.top    = inner_padding_tmp.bottom;
			drawable->private->inner_padding.right  = inner_padding_tmp.left;
			drawable->private->inner_padding.bottom = inner_padding_tmp.top;
			drawable->private->inner_padding.left   = inner_padding_tmp.right;
			
			repeat_tmp = drawable->private->repeat;
			
			drawable->private->repeat.top    = repeat_tmp.bottom;
			drawable->private->repeat.right  = repeat_tmp.left;
			drawable->private->repeat.bottom = repeat_tmp.top;
			drawable->private->repeat.left   = repeat_tmp.right;
			
			drawable->private->xpos.pixel    = -drawable->private->xpos.pixel;
			drawable->private->xpos.widget   = -drawable->private->xpos.widget;
			drawable->private->xpos.object   = -drawable->private->xpos.object;
			drawable->private->ypos.pixel    = -drawable->private->ypos.pixel;
			drawable->private->ypos.widget   = -drawable->private->ypos.widget;
			drawable->private->ypos.object   = -drawable->private->ypos.object;
			break;
		case ROTATE_NONE:
			break;
	}
	
	experience_filter_apply_group_filter (&drawable->filter, &group->filter);
	
	drawable->class->apply_group_settings (drawable, group);
}

void
experience_drawable_inherit_from (eXperienceDrawable * drawable, eXperienceDrawable * from)
{
	g_assert (drawable != NULL);
	g_assert (from     != NULL);
	
	/* nothing should be done, so return */
	if (drawable->private->dont_inherit) return;
	
	if (!(drawable->private->options_set & OPTION_PADDING))
		drawable->private->padding = from->private->padding;
	
	if (!(drawable->private->options_set & OPTION_DONT_DRAW))
		drawable->private->dont_draw = from->private->dont_draw;
	
	if (!(drawable->private->options_set & OPTION_ROUNDING))
		drawable->private->rounding = from->private->rounding;
	
	if (!(drawable->private->options_set & OPTION_INNER_PADDING))
		drawable->private->inner_padding = from->private->inner_padding;
	
	if (!(drawable->private->options_set & OPTION_REPEAT))
		drawable->private->repeat = from->private->repeat;
	
	if (!(drawable->private->options_set & OPTION_XPOS))
		drawable->private->xpos = from->private->xpos;
	
	if (!(drawable->private->options_set & OPTION_YPOS))
		drawable->private->ypos = from->private->ypos;
	
	if (!(drawable->private->options_set & OPTION_WIDTH))
		drawable->private->width = from->private->width;
	
	if (!(drawable->private->options_set & OPTION_HEIGHT))
		drawable->private->height = from->private->height;
	
	if (!(drawable->private->options_set & OPTION_DRAW_ENTIRE_ONLY))
		drawable->private->draw_entire_only = from->private->draw_entire_only;
	
	if (!(drawable->private->options_set & OPTION_DONT_CLIP))
		drawable->private->dont_clip = from->private->dont_clip;
	
	
	drawable->private->options_set |= from->private->options_set;
	
	/* always inherit filters */
	experience_filter_inherit_from (&drawable->filter, &from->filter);
	
	if (drawable->class == from->class) {
		/* inherit all other information, if the types are the same */
		drawable->class->inherit_from_drawable (drawable, from);
	}
}

#define BIG 10000

/* experience_drawable_draw
 * 
 * This function basically creates a new surface. On this surface all the
 * drawing in image.c will be done. After this, the surface will be drawn
 * tiled.
 * (This function needs to tile the image in some cases)
 * 
 * I don't think that how it currently works it is that good, but everything
 * seems to work fine.
 */

gboolean
experience_drawable_draw (eXperienceDrawable * drawable, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	gboolean result = TRUE;
	eXperienceSize repeat_distance;
	eXperienceSize drawable_size = {0, 0};
	cairo_surface_t * surface;
	cairo_pattern_t * pattern = NULL;
	eXperienceSize sub_cr_size;
	GdkRectangle dest_area, draw_entire_area;
	GdkPoint translation;
	cairo_t * sub_cr = NULL;
	
	eXperienceSize real_dest_size;
	
	/* no asserts/checks for now, nothing can really happen, because everything was checked before. */
	
	/* if this thing should not be drawn, get OUT */
	if (drawable->private->dont_draw) return TRUE;
	
	/* after this point, jump to "end" to return. */
	cairo_save (cr);
	
	/* get rid of the border */
	cairo_translate (cr, drawable->private->padding.left, drawable->private->padding.top);
	
	real_dest_size = *dest_size;
	real_dest_size.width  -= drawable->private->padding.left + drawable->private->padding.right;
	real_dest_size.height -= drawable->private->padding.top  + drawable->private->padding.bottom;

	/* get information */
	if (drawable->class->get_info != NULL) {
		drawable->class->get_info (drawable, style, &drawable_size);
	}
	
	/* calculate the repeat */
	repeat_distance.width  = drawable->private->width. widget * real_dest_size.width  + drawable->private->width. object * drawable_size.width  + drawable->private->width. pixel;
	repeat_distance.height = drawable->private->height.widget * real_dest_size.height + drawable->private->height.object * drawable_size.height + drawable->private->height.pixel;
	
	/* apply inner padding. save into padded_width, padded_height */
	sub_cr_size.width  = repeat_distance.width  - (drawable->private->inner_padding.left + drawable->private->inner_padding.right);
	sub_cr_size.height = repeat_distance.height - (drawable->private->inner_padding.top  + drawable->private->inner_padding.bottom);
	
	translation.x = experience_round(drawable->private->rounding, ((drawable->private->xpos.widget + 1.0) * (gfloat) real_dest_size.width  / 2.0) - ((drawable->private->xpos.widget + 1.0) * (gfloat) repeat_distance.width  / 2.0));
	translation.y = experience_round(drawable->private->rounding, ((drawable->private->ypos.widget + 1.0) * (gfloat) real_dest_size.height / 2.0) - ((drawable->private->ypos.widget + 1.0) * (gfloat) repeat_distance.height / 2.0));
	
	translation.x += drawable->private->xpos.pixel;
	translation.y += drawable->private->ypos.pixel;
	
	if (!drawable->private->dont_clip) { /* maybe do this by making the fill smaller */
			cairo_rectangle (cr, 0, 0, real_dest_size.width, real_dest_size.height);
			cairo_clip (cr);
	}
	
	cairo_translate (cr, translation.x, translation.y);
	
	
	{	/* calculate dest area */
		if (drawable->private->repeat.right == 0) {
			dest_area.width = BIG;
		} else {
			dest_area.width = (drawable->private->repeat.left + drawable->private->repeat.right - 1) * repeat_distance.width;
		}
		if (drawable->private->repeat.bottom == 0) {
			dest_area.height = BIG;
		} else {
			dest_area.height = (drawable->private->repeat.top + drawable->private->repeat.bottom - 1) * repeat_distance.height;
		}
		
		if (drawable->private->repeat.left == 0) {
			dest_area.x = -BIG;
			dest_area.width += BIG;
		} else {
			dest_area.x = -(drawable->private->repeat.left - 1) * repeat_distance.width;
		}
		if (drawable->private->repeat.top == 0) {
			dest_area.y = -BIG;
			dest_area.height += BIG;
		} else {
			dest_area.y = -(drawable->private->repeat.top - 1) * repeat_distance.height;
		}

		
		/* translate because of inner padding */
/*		dest_area.x += drawable->private->inner_padding.left;
		dest_area.y += drawable->private->inner_padding.top;*/
	}
	
	cairo_translate (cr, drawable->private->inner_padding.left, drawable->private->inner_padding.top);
	
	if (drawable->private->draw_entire_only) {
		draw_entire_area.x = -translation.x;
		draw_entire_area.y = -translation.y;
		draw_entire_area.width  = real_dest_size.width;
		draw_entire_area.height = real_dest_size.height;
		
		draw_entire_area.x += translation.x % repeat_distance.width;
		draw_entire_area.y += translation.y % repeat_distance.height;
		
		draw_entire_area.width  -= draw_entire_area.x;
		draw_entire_area.height -= draw_entire_area.y;
		
		draw_entire_area.width  -= draw_entire_area.width  % repeat_distance.width;
		draw_entire_area.height -= draw_entire_area.height % repeat_distance.height;
		
		gdk_rectangle_intersect (&dest_area, &draw_entire_area, &dest_area);
	}
	
	if ((drawable->private->repeat.left != 1) || (drawable->private->repeat.right != 1) || (drawable->private->repeat.top != 1) || (drawable->private->repeat.bottom != 1)) {
		/* create the cairo surface */
		surface = cairo_surface_create_similar (cairo_get_target (cr), CAIRO_CONTENT_COLOR_ALPHA, repeat_distance.width, repeat_distance.height);
		
		/* get the cairo context */
		sub_cr = cairo_create (surface);
		
		{ /* clip all drawing to dest_size */
			cairo_rectangle (sub_cr, 0, 0, sub_cr_size.width, sub_cr_size.height);
			cairo_clip (sub_cr);
		}
		
		/* create the cairo pattern */
		pattern = cairo_pattern_create_for_surface (surface);
		cairo_surface_destroy (surface);
		if (cairo_pattern_status (pattern) != CAIRO_STATUS_SUCCESS) {
			/* XXX: log error message */
			result = FALSE; /* whoops, fail */
			goto end;
		}
		
		cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
	
		/* draw */
		result = drawable->class->draw (drawable, sub_cr, &sub_cr_size, style);
		
		if (result) {
			/* draw the pattern */
			gdk_cairo_rectangle (cr, &dest_area);
			
			cairo_set_source (cr, pattern);
			
			cairo_fill (cr);
		}
	} else {
		cairo_save (cr);
		
		gdk_cairo_rectangle (cr, &dest_area);
		
		cairo_clip (cr);
		
		drawable->class->draw (drawable, cr, &sub_cr_size, style);
		cairo_restore (cr);
	}
	
end:
	if (sub_cr)
		cairo_destroy (sub_cr);
	if (pattern)
		cairo_pattern_destroy (pattern);
	
	cairo_restore (cr);
	
	return result;
}
