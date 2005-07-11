/* arch-tag: 6750c768-cd2d-4659-bf04-8ba8ab506b64 */

#include <cairo.h>
#include "render.h"
#include "group.h"

static cairo_t *
experience_get_cairo_context (GdkWindow * window, GdkRectangle * object_area, GdkRectangle * area, gboolean dont_clip_to_object)
{
	cairo_t * cr;
	
	cr = gdk_cairo_create (window);
	
	/* set up clipping */
	if (dont_clip_to_object) {
		gdk_cairo_rectangle (cr, area);
	} else {
		gdk_cairo_rectangle (cr, object_area);
	}
	
	cairo_clip (cr);

	cairo_translate (cr, object_area->x, object_area->y);
	
	return cr;
}

gboolean
experience_render_group_to_cr (eXperienceGroup *group, cairo_t * cr, eXperienceSize * dest_size, GtkStyle * style)
{
	g_assert (group != NULL);
	g_assert (cr    != NULL);
	g_return_val_if_fail (dest_size != NULL, FALSE);
	g_return_val_if_fail (style     != NULL, FALSE);
	
	/* succeed silently */
	if ((dest_size->width <= 0) && (dest_size->height <= 0))
		return TRUE;
	
	/* render the group */
	return experience_group_draw (group, cr, dest_size, style);
}

gboolean
experience_render_group (eXperienceGroup *group, GdkWindow *window, GdkRectangle * object_area, GdkRectangle *area, GtkStyle * style)
{
	gboolean result;
	cairo_t * cr;
	GdkRectangle clip_area;
	eXperienceSize size;
	
	g_assert (group != NULL);
	g_assert (object_area != NULL);
	
	/* the following could happen because of an app bug */
	g_return_val_if_fail (window != NULL, FALSE);
	g_return_val_if_fail (style != NULL, FALSE);
	
	if ((object_area->width == -1) && (object_area->height == -1)) {
		gdk_window_get_size (window, &object_area->width, &object_area->height);
	} else if (object_area->width == -1)
		gdk_window_get_size (window, &object_area->width, NULL);
	else if (object_area->height == -1)
		gdk_window_get_size (window, NULL, &object_area->height);
	
	if (area == NULL) {
		clip_area = *object_area;
	} else {
		clip_area = *area;
	}
	
	cr = experience_get_cairo_context (window, object_area, &clip_area, group->dont_clip);
	
	if (cr == NULL) { /* XXX: is the cr == NULL check correct? */
		/* XXX: maybe print cairo error message. */
		return FALSE;
	}
	
	size.width = object_area->width;
	size.height = object_area->height;
	
	result = experience_render_group_to_cr (group, cr, &size, style);
	
	cairo_destroy (cr);
	
	return result;
}
