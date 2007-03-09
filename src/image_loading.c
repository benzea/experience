/*  eXperience GTK engine: image_loading.c
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

#include "image_loading.h"
#include "utils.h"

GHashTable * image_cache = NULL;

static guint
hash_image (eXperienceCacheImage * image)
{
	guint result;
	
	g_return_val_if_fail (image != NULL, 0);
	g_return_val_if_fail (image->file != NULL, 0);
	
	result = g_str_hash(image->file);
	result = experience_filter_hash(&image->filter, result);
	
	return result;
}

static eXperienceRawImage *
load_image (eXperienceCacheImage * image)
{
	GError *err = NULL;
	GdkPixbuf *filtered = NULL, *orig_file;
	eXperienceRawImage * result = NULL;
	
	orig_file  = gdk_pixbuf_new_from_file (image->file, &err);
	
	if (!orig_file) {
		g_warning ("eXperience engine: Cannot load pixmap file %s: %s\n",
		           image->file, err->message);
		g_error_free (err);
	} else {
		filtered = gdk_pixbuf_add_alpha (orig_file, FALSE, 0, 0, 0);
		g_object_unref (orig_file);
		
		filtered = experience_apply_filters(filtered, &image->filter);
	}
	
	if (filtered) {
		result = experience_raw_image_create (filtered);
		g_object_unref (filtered);
	}
	
	return result;
}

static gboolean
image_equal (eXperienceCacheImage * image1, eXperienceCacheImage * image2)
{
	if (!g_str_equal (image1->file, image2->file)) return FALSE;
	
	return experience_filter_equal (&image1->filter, &image2->filter);
}

static eXperienceCacheImage *
duplicate_image (eXperienceCacheImage * src)
{
	eXperienceCacheImage * dest = g_new0 (eXperienceCacheImage, 1);
	dest->file   = g_strdup(src->file);
	experience_filter_init (&dest->filter, FILTER_NONE);
	experience_filter_copy (&dest->filter, &src->filter);
	
	return dest;
}

static void
experience_cache_image_destroy (eXperienceCacheImage * image)
{
	g_free (image->file);
	g_free (image);
}

eXperienceRawImage *
experience_get_raw_image (eXperienceCacheImage * image, GtkStyle * style)
{
	eXperienceRawImage * result;
	
	g_return_val_if_fail (image != NULL, NULL);
	if (image->file == NULL) {
		return NULL;
	}
	
	if (image->filter.recolor_mode != RECOLOR_NONE) {
		retrive_recolor_colors (&image->filter, style);
	}
	
	if (image_cache == NULL) {
		image_cache = g_hash_table_new_full ((GHashFunc)  hash_image,
		                                     (GEqualFunc) image_equal,
		                                     (GDestroyNotify) experience_cache_image_destroy,
		                                     (GDestroyNotify) experience_raw_image_destroy);
	}
	
	/* first do a lookup. */
	result = g_hash_table_lookup (image_cache, image);
	if (result == NULL) {
		result = load_image (image);
		
		/* try to load it again next time ... */
		if (result != NULL) {
			g_hash_table_insert (image_cache, duplicate_image (image), result);
		} else {
			return NULL;
		}
	}
	
	return result;
}

void
experience_image_cache_destroy (void)
{
	if (image_cache != NULL) {
		g_hash_table_destroy (image_cache);
		image_cache = NULL;
	}
}
