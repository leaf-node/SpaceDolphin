/*  Copyright 2011 Andrew Engelbrecht <sudoman@ninthfloor.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "spacedolphin.h"

// handle collision between objects previously chosen by
// cpSpaceAddCollisionHandler()
// chcolor changes the color of C_COLOR objects touched by the C_SHIP.
int chcolor (cpArbiter *arb, cpSpace *space, void *data)
{
    struct objnode *obja, *objb, *objx;
    cpShape *sa, *sb;
    struct timespec now, dt;

    cpArbiterGetShapes(arb, &sa, &sb);

    obja = objb = NULL;
    for (objx = (struct objnode *) data; objx->next != NULL; \
	    objx = objx->next)
    {
	if (objx->s == sa)
	    obja = objx;
	if (objx->s == sb)
	    objb = objx;
    }

    if (obja == NULL || objb == NULL) {
	fprintf(stderr, "*** warning: couldn't find object during" \
		"collision callback!\n");
	return true;
    }

    curtime(&now);
    dt = tdiff(now, objb->lasthit);

    if ((dt.tv_sec >= 1) || (convtns(dt) > 0.25 * 1e9))
    {
    	switch (objb->cstatus) {
	case 0:
	    objb->cstatus++;
	    objb->c1 = setcolor(0.85, 0, 0, 1);
	    break;
	case 1:
	    objb->cstatus++;
	    objb->c1 = setcolor(0, 0.85, 0, 1);
	    break;
	case 2:
	    objb->cstatus = 0;
	    objb->c1 = setcolor(0, 0, 1, 1);

	    break;
	}

	objb->lasthit = now;
    }

    return true;
}


