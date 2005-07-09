/* arch-tag: a5e3690e-f3a6-4c3c-9b36-640dcd9f8aa5 */

/*  eXperience GTK engine: fill.h
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

#ifndef __FILL_H
#define __FILL_H

#include "drawable.h"
#include "dynamic_color.h"

eXperienceDrawableClass * experience_fill_class;

typedef struct {
	eXperienceDrawable drawable;
	
	gboolean color_set;
	eXperienceDynamicColor color;
} eXperienceFill;

void experience_fill_set_color (eXperienceFill * fill, eXperienceDynamicColor color);

void experience_fill_init_class (void);

#endif /* _FILL_H */
