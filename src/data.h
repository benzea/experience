/* arch-tag: b2afa3aa-e38f-436a-9192-eb0153ab7df6 */

/*  eXperience GTK engine: data.h
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

#ifndef __DATA_H
#define __DATA_H

#include "filter.h"

typedef struct {
	guint left;
	guint right;
	guint top;
	guint bottom;
} eXperienceBorder;

typedef struct {
	gint left;
	gint right;
	gint top;
	gint bottom;
} eXperiencePadding;

typedef struct {
	gint width;
	gint height;
} eXperienceSize;

#endif /* __DATA_H */
