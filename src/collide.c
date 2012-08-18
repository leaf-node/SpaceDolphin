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
// chcolor changes the color of C_LARGE objects touched by the C_SHIP.
int chcolor (cpArbiter *arb, cpSpace *space, void *data)
{
    struct objnode *obja, *objb, *objx;
    cpShape *sa, *sb;

    cpArbiterGetShapes(arb, &sa, &sb);

    obja = objb = NULL;
    for (objx = (struct objnode *) data; objx != NULL; \
	    objx = objx->next)
    {
	if (objx->s == sa)
	    obja = objx;
	if (objx->s == sb)
	    objb = objx;
    }

    if (obja == NULL || objb == NULL) {
	fprintf(stderr, "*** warning: couldn't find object during " \
		"collision callback!\n");
	return true;
    }

    // ship hits large object
    if (obja->s->collision_type == C_SHIP \
	    && objb->s->collision_type == C_LARGE) {
	if (obja->player == P_ONE) {
	    objb->c1 = setcolor(0.5, 0, 1, 1);
	    objb->cstatus = P_ONE;
	}
	else if (obja->player == P_TWO) {
	    objb->c1 = setcolor(.75, 0.5, 0, 1);
	    objb->cstatus = P_TWO;
	}
    }
    // large object hits small object
    else if (obja->s->collision_type == C_LARGE \
	    && objb->s->collision_type == C_SMALL) {
	if (obja->cstatus == P_ONE) {
	    objb->c1 = setcolor(0, 0, 1, 1);
	    objb->cstatus = P_ONE;
	}
	else if (obja->cstatus == P_TWO) {
	    objb->c1 = setcolor(1, 0, 0, 1);
	    objb->cstatus = P_TWO;
	}
    }
    // small object hits ship
    else if (obja->s->collision_type == C_SMALL \
	    && objb->s->collision_type == C_SHIP) {
	if (obja->cstatus == P_ONE \
		&& objb->player == P_TWO) {
	    // insert player two health reduction here
	    obja->c1 = setcolor(0, 0, 0, 0.625);
	    obja->c2 = setcolor(1, 0, 0, 0.625);
	}
	else if (obja->cstatus == P_TWO \
		&& objb->player == P_ONE) {
	    // insert player one health reduction here
	    obja->c1 = setcolor(0, 0, 0, 0.625);
	    obja->c2 = setcolor(1, 0, 0, 0.625);
	}
    }
    // this shouldn't happen
    else {
	fprintf(stderr, "*** warning: unknown collision type\n");
	return true;
    }

    return true;
}

