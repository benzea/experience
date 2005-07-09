/* arch-tag: c13f1b84-7678-4508-8cf7-dc8f49b32f45 */

/*  eXperience GTK engine: style.c
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
#include <gtk/gtk.h>
#include "style.h"
#include "drawing_functions.h"

static void
experience_style_class_init (eXperienceStyleClass * klass)
{
	GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);
	
	experience_style_parent_class = g_type_class_peek_parent (klass);
	
	style_class->render_icon      = experience_render_icon;
	style_class->draw_hline       = experience_draw_hline;
	style_class->draw_vline       = experience_draw_vline;
	style_class->draw_shadow      = experience_draw_shadow;
	style_class->draw_arrow       = experience_draw_arrow;
	style_class->draw_box         = experience_draw_box;
	style_class->draw_flat_box    = experience_draw_flat_box;
	style_class->draw_check       = experience_draw_check;
	style_class->draw_option      = experience_draw_option;
	style_class->draw_tab         = experience_draw_tab;
	style_class->draw_shadow_gap  = experience_draw_shadow_gap;
	style_class->draw_box_gap     = experience_draw_box_gap;
	style_class->draw_extension   = experience_draw_extension;
	style_class->draw_focus       = experience_draw_focus;
	style_class->draw_slider      = experience_draw_slider;
	style_class->draw_handle      = experience_draw_handle;
	style_class->draw_expander    = experience_draw_expander;
	style_class->draw_resize_grip = experience_draw_resize_grip;
}

GType experience_type_style = 0;

void
experience_style_register_type (GTypeModule * module)
{
	if (!experience_type_style) {
		static const GTypeInfo object_info =
		{
			sizeof (eXperienceStyleClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) experience_style_class_init,
			NULL,			/* class_finalize */
			NULL,			/* class_data */
			sizeof (eXperienceStyle),
			0,				/* n_preallocs */
			(GInstanceInitFunc) NULL,
		};
		
		experience_type_style = g_type_module_register_type (module,
		                                                     GTK_TYPE_STYLE,
		                                                     "eXperienceStyle",
		                                                     &object_info, 0);
	}
}
