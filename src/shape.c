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

#define BHOLE           1	// the isloated layer for black holes
#define NONBH          ~1	// layer so things don't collide with black holes

enum groups { FLOATG = 1 };


struct objnode *makenode(struct objnode *objx);
void insertobj(cpSpace * space, struct objnode *objx);
cpVect randv(struct objnode *objlast, cpVect p1, cpVect p2, cpFloat width);
bool nearobjs(struct objnode *objlast, cpVect rvect, cpFloat width);
struct color_rgba setcolor(float r, float g, float b, float a);

struct objnode *makecirc(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, cpFloat radius, cpVect pos);
struct objnode *makepoly(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, int nverts, cpVect * verts,
			 cpVect pos);
struct objnode *makeline(struct objnode *objx, cpSpace * space, bool statb,
			 cpVect v1, cpVect v2);

struct objnode *makefloat(struct objnode *objx, cpSpace * space,
			  cpFloat mass, cpFloat radius, cpVect pos);
struct objnode *makebhole(struct objnode *objx, cpSpace * space,
			  cpFloat mass, cpFloat radius, cpVect pos);
struct objnode *makerect(struct objnode *objx, cpSpace * space,
			 cpFloat mass, cpFloat width, cpVect pos);
struct objnode *maketria(struct objnode *objx, cpSpace * space,
			 cpFloat mass, cpFloat len, cpFloat width,
			 cpVect pos);


// initialize a space and then add a whole bunch of shapes and boundaries
cpSpace *makeshapes(struct objnode *objx, struct objnode **vehicle)
{
    int i;
    cpSpace *space = cpSpaceNew();

    cpVect gravity = cpv(0, VGRAV);
    cpSpaceSetGravity(space, gravity);


/* boundaries for the game... */
    cpVect fwedgeverts[4] = {cpv(-1,-1), cpv(-1,11), cpv(60,11), cpv(108,-1)};
    objx = makepoly(objx, space, true, 1, 4, fwedgeverts, cpvzero);
    objx->c1 = setcolor(0, 0, 0, 0); //transparent body
    objx->c2 = setcolor(1, 0, 0, 1);

    objx = makeline(objx, space, true, cpv(99, 1), cpv(160, 1));
    objx = makeline(objx, space, true, cpv(0, 119), cpv(160, 119));
    objx = makeline(objx, space, true, cpv(1, 120), cpv(1, 10));
    objx = makeline(objx, space, true, cpv(159, 120), cpv(159, 0));


/* deterministically placed objects... */
    objx = makecirc(objx, space, false, 1.0, 10, cpv(120, 57));
    objx = makecirc(objx, space, false, 0.36, 3, cpv(120, 45));

    // the "vehicle" is the object that is controlled by the keyboard
    objx = *vehicle = maketria(objx, space, 1.33, 20, 8, cpv(40, 20));
    objx->c1 = setcolor(0.5, 0, 1, 1);
    objx->c2 = setcolor(  1, 0, 0, 1);

    objx = makebhole(objx, space, 1, 5, cpv(100, 70));
    cpShapeSetLayers(objx->s, BHOLE);
    objx = makebhole(objx, space, 1, 5, cpv(40, 80));
    cpShapeSetLayers(objx->s, BHOLE);

    objx = makerect(objx, space, 0.25, 8, cpv(80, 20));


/* randomly placed objects... */
    srandom(curns());
    for (i = 0; i < 7; i++) {
	objx = makecirc(objx, space, false, 0.5, 5, randfit(objx, 5));
	objx = makefloat(objx, space, 0.08, 2.0, randfit(objx, 2));
	objx = makefloat(objx, space, 0.08, 2.0, randfit(objx, 2));
    }

    return space;
}

// makes a circle, adds it to the space and then returns a handle
struct objnode *makecirc(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, cpFloat radius, cpVect pos)
{
    objx = makenode(objx);
    objx->kind = S_CIRC;

    if (statb) {
	objx->s = cpCircleShapeNew(space->staticBody, radius, cpvzero);
	objx->b = space->staticBody;
    }
    else {
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	objx->b = cpBodyNew(mass, moment);
	objx->s = cpCircleShapeNew(objx->b, radius, cpvzero);
    }

    insertobj(space, objx);
    if (!statb)
	cpBodySetPos(objx->b, pos);

    objx->width = radius;

    cpShapeSetLayers(objx->s, NONBH);

    cpBodySetAngVel(objx->b, 20);
    cpShapeSetFriction(objx->s, 0.7);
    cpShapeSetElasticity(objx->s, 0.7f);
    objx->c1 = setcolor(0, 0, 1, 1);
    objx->c2 = setcolor(0.5, 0.5, 0.5, 1);

    return objx;
}

// creates any sort of polygon...
struct objnode *makepoly(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, int nverts, cpVect * verts,
			 cpVect pos)
{
    objx = makenode(objx);
    objx->kind = S_POLY;

    if (statb) {
	objx->s =
	    cpPolyShapeNew(space->staticBody, nverts, verts, cpvzero);
	objx->b = space->staticBody;
    }
    else {
	cpFloat moment = cpMomentForPoly(mass, nverts, verts, cpvzero);
	objx->b = cpBodyNew(mass, moment);
	objx->s = cpPolyShapeNew(objx->b, nverts, verts, cpvzero);
    }

    insertobj(space, objx);
    if (!statb)
	cpBodySetPos(objx->b, pos);

    int i;
    for (i = 0; i < nverts; i++)
	if (cpvlength(verts[i]) > objx->width)
	    objx->width = cpvlength(verts[i]);

    cpShapeSetLayers(objx->s, NONBH);

    cpShapeSetFriction(objx->s, 0.7f);
    cpShapeSetElasticity(objx->s, 0.4f);
    objx->c1 = setcolor(0, 0, 1, 1);
    objx->c2 = setcolor(0.5, 0.5, 0.5, 1);

    return objx;
}

// creates a static boundary...
struct objnode *makeline(struct objnode *objx, cpSpace * space,
			 bool statb, cpVect v1, cpVect v2)
{
    objx = makenode(objx);
    objx->kind = S_LSEG;

    if (statb) {
	objx->s = cpSegmentShapeNew(space->staticBody, v1, v2, 0);
	objx->b = space->staticBody;
    }
    else {
	fprintf(stderr,
		"Error, non-static line segments are not supported.\n");
	exit(3);
    }

    insertobj(space, objx);

    cpShapeSetFriction(objx->s, 1);
    cpShapeSetElasticity(objx->s, 0.7f);
    objx->c1 = setcolor(1, 0, 0, 1);
    objx->c2 = setcolor(0, 0, 0, 0);

    return objx;
}



// makes a circle unaffected by vertical gravity. attracted to bholes
struct objnode *makefloat(struct objnode *objx, cpSpace * space,
			  cpFloat mass, cpFloat radius, cpVect pos)
{
    objx = makecirc(objx, space, false, mass, radius, pos);

    objx->b->velocity_func = &orbit;
    cpShapeSetGroup(objx->s, FLOATG);

    cpBodySetAngVel(objx->b, 2);
    objx->c1 = setcolor(1, 0.5, 0, 0.625);
    objx->c2 = setcolor(1, 0, 0, 0.625);

    return objx;
}

// makes a black hole that does not move. it attracts "float" objects.
struct objnode *makebhole(struct objnode *objx, cpSpace * space,
			  cpFloat mass, cpFloat radius, cpVect pos)
{
    // black holes aren't 'static', because we want them to rotate
    objx = makecirc(objx, space, false, mass, radius, pos);

    // ...but they should not change their 0 velocity:
    objx->b->velocity_func = &dontfall;
    objx->bhole = true;
    cpShapeSetGroup(objx->s, FLOATG);

    cpBodySetVelLimit(objx->b, 0);
    cpBodySetAngVel(objx->b, 10);
    objx->c1 = setcolor(0.125, 0.125, 0.125, 0.5);
    objx->c2 = setcolor(0.5, 0.5, 0.5, 0.5);

    return objx;
}


// makes a triangle...
struct objnode *maketria(struct objnode *objx, cpSpace * space,
			 cpFloat mass, cpFloat len, cpFloat width,
			 cpVect pos)
{
    cpVect verts[3];

    verts[0] = cpv(0, 2.0 * len / 3.0);
    verts[1] = cpv(width / 2.0, -len / 3.0);
    verts[2] = cpv(-width / 2.0, -len / 3.0);

    cpShapeSetFriction(objx->s, 0.50);

    objx = makepoly(objx, space, false, mass, 3, verts, pos);

    return objx;
}

// makes a rectangle...
struct objnode *makerect(struct objnode *objx, cpSpace * space,
			 cpFloat mass, cpFloat width, cpVect pos)
{
    cpFloat hw;
    cpVect verts[4];

    hw = width / 2;

    verts[0] = cpv(-hw, -hw);
    verts[1] = cpv(-hw, hw);
    verts[2] = cpv(hw, hw);
    verts[3] = cpv(hw, -hw);

    objx = makepoly(objx, space, false, mass, 4, verts, pos);

    return objx;
}

// returns a structure to store an object's color
struct color_rgba setcolor(float r, float g, float b, float a)
{
    struct color_rgba color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

// picks a random spot to place an object, then calls nearobjs() to test it.
cpVect randv(struct objnode * objlast, cpVect p1, cpVect p2, cpFloat width)
{
    int i;
    cpVect rvect;

    for (i = 0; i < 1000; i++) {
	rvect.x = p1.x + random() / (double) RAND_MAX *(p2.x - p1.x);
	rvect.y = p1.y + random() / (double) RAND_MAX *(p2.y - p1.y);
	if (nearobjs(objlast, rvect, width))
	    return rvect;
    }

    fprintf(stderr,
	    "Error: cannot find a random place to fit an object.\n");
    exit(4);

    return rvect;
}

// returns true if an object 'width' wide intersects with another object.
bool nearobjs(struct objnode * objlast, cpVect rvect, cpFloat width)
{
    cpFloat dist;
    struct objnode *objch = objlast;

    while ((objch = objch->prev)) {

	if (objch->kind == S_CIRC || objch->kind == S_POLY)
	    dist = cpvlength(cpvsub(rvect, cpBodyGetPos(objch->b)));
	else if (objch->kind == S_LSEG) {
	    cpVect enda, endb, segv, posv;
	    cpFloat angle;

	    enda = cpSegmentShapeGetA(objch->s);
	    endb = cpSegmentShapeGetB(objch->s);
	    if (cpvlength(cpvsub(rvect, enda)) <
		cpvlength(cpvsub(rvect, endb))) {
		segv = cpvsub(enda, endb);
		posv = cpvsub(rvect, endb);
	    } else {
		segv = cpvsub(endb, enda);
		posv = cpvsub(rvect, enda);
	    }
	    angle = cpvtoangle(posv) - cpvtoangle(segv);
	    if (fabs(angle) < PI / 2)
		dist = fabs(sin(angle) * cpvlength(posv));
	    else
		dist = cpvlength(posv);
	} else
	    dist = INFINITY;

	if (dist > objch->width + width)
	    continue;
	else
	    return false;
    }

    return true;
}

// inserts a handle for pointing to a new physical body/shape
struct objnode *makenode(struct objnode *objx)
{
    struct objnode *objnew;

    objnew = malloc(sizeof(struct objnode));
    objnew->next = objx->next;
    objnew->prev = objx;
    objx->next = objnew;

    objnew->kind = S_NONE;
    objnew->bhole = false;
    objnew->b = NULL;
    objnew->s = NULL;
    objnew->width = 0;
    objnew->c1 = setcolor(0, 0, 0, 1);
    objnew->c2 = setcolor(1, 1, 1, 1);

    return objnew;
}

// inserts object's body and shape into the space
void insertobj(cpSpace * space, struct objnode *objx)
{
    if (!cpBodyIsStatic(objx->b))
	cpSpaceAddBody(space, objx->b);
    if (objx->s != NULL)
	cpSpaceAddShape(space, objx->s);
}

// removes an object and frees the memory associated with it.
void rmobj(struct objnode *objx)
{
    if (objx->prev == NULL) {
	fprintf(stderr, "*** Error: attempted to remove root object.\n");
	exit(2);
    }

    cpShapeFree(objx->s);
    if (!cpBodyIsStatic(objx->b))
        cpBodyFree(objx->b);

    objx->prev->next = objx->next;
    if (objx->next)
	objx->next->prev = objx->prev;

    free(objx);
}

// removes every object after the current one
void rmobjs(struct objnode *objroot)
{
    while (objroot->next)
	rmobj(objroot->next);
}
