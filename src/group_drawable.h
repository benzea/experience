/* arch-tag: 9c995cf1-01d9-470b-a028-471209ab73f6 */

/*  eXperience GTK engine: group_drawable.h
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

#ifndef __GROUP_DRAWABLE_H
#define __GROUP_DRAWABLE_H

#include "drawable.h"
#include "group.h"
#include "dynamic_color.h"

eXperienceDrawableClass * experience_group_drawable_class;

typedef struct {
	eXperienceDrawable drawable;
	
	gboolean   group_name_set;
	gchar    * group_name;
	
	eXperienceGroup * group;
} eXperienceGroupDrawable;

void experience_group_drawable_set_group_name (eXperienceGroupDrawable * group_drawable, gchar * group_name);

void experience_group_drawable_init_class (void);

#endif /* _GROUP_DRAWABLE_H */