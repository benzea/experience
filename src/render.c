/* arch-tag: 6750c768-cd2d-4659-bf04-8ba8ab506b64 */

#include "render.h"
#include "group.h"

static GdkPixbuf *
experience_render_create_pixbuf (GdkRectangle * object_area, GdkRectangle * area, gboolean dont_clip_to_object)
{
	GdkRectangle clip_area;
	GdkPixbuf * result;
	
	g_return_val_if_fail (object_area != NULL, NULL);
	g_return_val_if_fail (area != NULL, NULL);
	
	if (!dont_clip_to_object) {
		gdk_rectangle_intersect(area, object_area, &clip_area);
	} else {
		clip_area = *area;
	}
	
	if ((clip_area.width <= 0) || (clip_area.height <= 0)) {
		return NULL; /* Nothing has to be done. */
	}
	
	result = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, clip_area.width, clip_area.height);
	
	/* clear data */
	gdk_pixbuf_fill (result, 0x00000000);
	
	object_area->x -= clip_area.x;
	object_area->y -= clip_area.y;
	
	*area = clip_area;
	
	return result;
}

GdkPixbuf *
experience_render_group_to_new_pixbuf (eXperienceGroup *group, GdkRectangle *dest_area, GdkRectangle * object_area, GdkRegion * dirty_region, GtkStyle * style)
{
	GdkPixbuf * pixbuf;
	
	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (dest_area   != NULL, NULL);
	g_return_val_if_fail (object_area != NULL, NULL);
	
	/* no message needed here. */
	if ((object_area->width <= 0) && (object_area->height <= 0))
		return NULL;
	
	pixbuf = experience_render_create_pixbuf (object_area, dest_area, group->dont_clip);
	
	if (pixbuf == NULL)
		return NULL;
	
	/* Render all images onto tmp_pixbuf. */
	if (!experience_group_draw (group, pixbuf, object_area, dirty_region, style)) {
		g_object_unref(pixbuf);
		return NULL;
	}
	
	return pixbuf;
}

void
experience_render_pixbuf_to_window (GdkWindow * window, GdkPixbuf * pixbuf, GdkRectangle * area, GdkRegion * clip_region, gfloat opacity)
{
	GdkGC * tmp_gc = NULL;
	
	experience_change_pixbuf_opacity (pixbuf, opacity);
	
	tmp_gc = gdk_gc_new (window);
	
	if (clip_region != NULL) {
		gdk_gc_set_clip_region (tmp_gc, clip_region);
		gdk_gc_set_clip_origin (tmp_gc, area->x, area->y);
	}
	
	gdk_draw_pixbuf (window, tmp_gc, pixbuf,
	                 0, 0,
	                 area->x, area->y,
	                 area->width, area->height,
	                 GDK_RGB_DITHER_NORMAL, 0 ,0);
	
	gdk_gc_unref (tmp_gc);
}

gboolean
experience_render_group (eXperienceGroup *group, GdkWindow *window, GdkRectangle * object_area, GdkRectangle *area, GtkStyle * style)
{
	GdkPixbuf * pixbuf;
	GdkRectangle dest_area;
	GdkRegion * dirty_region;
	
	g_return_val_if_fail (group != NULL, FALSE);
	
	
	if ((object_area->width == -1) && (object_area->height == -1)) {
		gdk_window_get_size (window, &object_area->width, &object_area->height);
	} else if (object_area->width == -1)
		gdk_window_get_size (window, &object_area->width, NULL);
	else if (object_area->height == -1)
		gdk_window_get_size (window, NULL, &object_area->height);
	
	if (area == NULL) {
		dest_area = *object_area;
	} else {
		dest_area = *area;
	}
	
	dirty_region = gdk_region_new ();
	
	/* render into pixbuf */
	pixbuf = experience_render_group_to_new_pixbuf (group, &dest_area, object_area, dirty_region, style);
	if (pixbuf == NULL)
		return FALSE;
	
	dest_area.x = dest_area.x;
	dest_area.y = dest_area.y;
	dest_area.width  = gdk_pixbuf_get_width  (pixbuf);
	dest_area.height = gdk_pixbuf_get_height (pixbuf);
	
	experience_render_pixbuf_to_window (window, pixbuf, &dest_area, dirty_region, group->filter.opacity);
	
	g_object_unref (pixbuf);
	return TRUE;
}

void
experience_render_cleanup (void)
{
	/* ok, this is now empty, I'll just leave it in here for now. */
}
