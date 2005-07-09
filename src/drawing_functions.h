/* arch-tag: 7ee87a9d-c66e-4763-bf65-79af2a482088 */

/*  eXperience GTK engine: drawing_functions.h
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

#ifndef __DRAWING_FUNCTIONS_H
#define __DRAWING_FUNCTIONS_H
#include <gtk/gtk.h>
#include <gdk/gdk.h>

void experience_draw_hline      (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x1,
                             gint             x2,
                             gint             y);
void experience_draw_vline      (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             y1,
                             gint             y2,
                             gint             x);
void experience_draw_shadow     (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_arrow      (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             GtkArrowType     arrow_type,
                             gboolean         fill,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_box        (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_flat_box   (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_check      (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_option     (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_tab        (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_shadow_gap (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height,
                             GtkPositionType  gap_side,
                             gint             gap_x,
                             gint             gap_width);
void experience_draw_box_gap    (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height,
                             GtkPositionType  gap_side,
                             gint             gap_x,
                             gint             gap_width);
void experience_draw_extension  (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height,
                             GtkPositionType  gap_side);
void experience_draw_focus      (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height);
void experience_draw_slider     (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height,
                             GtkOrientation   orientation);
void experience_draw_handle     (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GtkShadowType    shadow_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             gint             width,
                             gint             height,
                             GtkOrientation   orientation);
void experience_draw_expander   (GtkStyle        *style,
                             GdkWindow       *window,
                             GtkStateType     state_type,
                             GdkRectangle    *area,
                             GtkWidget       *widget,
                             const gchar     *detail,
                             gint             x,
                             gint             y,
                             GtkExpanderStyle expander_style);
void experience_draw_resize_grip (GtkStyle       *style,
                              GdkWindow      *window,
                              GtkStateType    state_type,
                              GdkRectangle   *area,
                              GtkWidget      *widget,
                              const gchar    *detail,
                              GdkWindowEdge   edge,
                              gint            x,
                              gint            y,
                              gint            width,
                              gint            height);

GdkPixbuf * experience_render_icon (GtkStyle * style,
                                const GtkIconSource * source,
                                GtkTextDirection direction,
                                GtkStateType state,
                                GtkIconSize size,
                                GtkWidget * widget,
                                const gchar * detail);
#endif /* __DRAWING_FUNCTIONS_H */
