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
// cpSpaceAddCollisionHandler(). chcolor changes the the owner (and by extension
// the color) of C_LARGE objects touched by the C_SHIP, C_SMALL objects touched
// by C_LARGE objects, and C_SMALL objects that touch C_SHIP objects.
// it also handles hitpoint reduction.
int chcolor(cpArbiter * arb, cpSpace * space, void *data)
{
    struct objnode *obja, *objb, *objx;
    cpShape *sa, *sb;
    struct timespec now, dt;

    cpArbiterGetShapes(arb, &sa, &sb);

    obja = objb = NULL;
    for (objx = (struct objnode *) data; objx != NULL; objx = objx->next) {
	if (objx->s == sa)
	    obja = objx;
	if (objx->s == sb)
	    objb = objx;
    }

    if (obja == NULL || objb == NULL) {
	fprintf(stderr, "*** warning: couldn't find object during "
		"collision callback!\n");
	return true;
    }

    now = curtime();
    dt = tdiff(now, objb->lastchange);

    // ship hits large object
    if (obja->s->collision_type == C_SHIP
	&& objb->s->collision_type == C_LARGE) {

	if (!isbrief(dt) || (objb->lastchangeby == obja)) {
	    objb->ownedby = obja->ownedby;
	    objb->lastchange = now;
	    objb->lastchangeby = obja;
	}
    }
    // large object hits small object
    else if (obja->s->collision_type == C_LARGE
	     && objb->s->collision_type == C_SMALL) {

	if (!isbrief(dt) || (objb->lastchangeby == obja)) {
	    objb->ownedby = obja->ownedby;
	    objb->lastchange = now;
	    objb->lastchangeby = obja;
	}
    }
    // small object hits ship
    else if (obja->s->collision_type == C_SMALL
	     && objb->s->collision_type == C_SHIP) {

	if (obja->ownedby != objb->ownedby
	    && obja->ownedby != P_NONE) {
	    objb->pinfo->hp -= 1;
	    obja->ownedby = P_NONE;
	    // the ship can get hit multiple times in a row, but the small
	    // object should not cycle color quickly when stuck between a ship
	    // and a large object.
	    obja->lastchange = now;
	    obja->lastchangeby = objb;
	}
    }
    // this shouldn't happen
    else {
	fprintf(stderr, "*** warning: unknown collision type\n");
	return true;
    }

    return true;
}
