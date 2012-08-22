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

#define BHOLE	    1		// the isloated layer for black holes
#define NONBH	    1 << 1	// layer so things don't collide with black holes

enum groups { FLOATG = 1 };


struct objnode *makenode(struct objnode *objx);
void initplayer(struct objnode *player);
void insertobj(cpSpace * space, struct objnode *objx);
cpVect randfit(struct objnode *objx, cpFloat r);
cpFloat randrange(cpFloat min, cpFloat max);
bool nearobjs(struct objnode *objlast, cpVect randv, cpFloat radius);
void giverandspin(struct objnode *objx);

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
struct objnode *makeplayer(struct objnode *objx, cpSpace * space,
			   int playernum);
struct objnode *maketria(struct objnode *objx, cpSpace * space,
			 cpFloat mass, cpFloat len, cpFloat width,
			 cpVect pos);


// initialize a space and then add a whole bunch of shapes and boundaries
cpSpace *makeshapes(struct objnode *objroot)
{
    int i;
    struct timespec time;
    cpSpace *space;
    struct objnode *objx;

    space = cpSpaceNew();
    cpVect gravity = cpv(0, VGRAV);
    cpSpaceSetGravity(space, gravity);

    objx = objroot;

/* boundaries for the game... */
    objx = makeline(objx, space, true, cpv(XMIN, YMIN + 1),
		    cpv(XMAX, YMIN + 1));
    objx = makeline(objx, space, true, cpv(XMIN, YMAX - 1),
		    cpv(XMAX, YMAX - 1));

    objx = makeline(objx, space, true, cpv(XMIN + 1, YMAX),
		    cpv(XMIN + 1, YMIN));
    objx = makeline(objx, space, true, cpv(XMAX - 1, YMAX),
		    cpv(XMAX - 1, YMIN));

    // needed before random intial velocities, placement and rotation
    curtime(&time);
    srandom(time.tv_nsec);

/* deterministically placed objects... */
    objx = makebhole(objx, space, 0.25, 5, cpv(100, 70));
    objx = makebhole(objx, space, 0.25, 5, cpv(40, 80));

    objx = makecirc(objx, space, false, 0.5, 10, cpv(120, 57));
    giverandspin(objx);
    objx = makecirc(objx, space, false, 0.15, 3, cpv(120, 45));
    giverandspin(objx);

    objx = makerect(objx, space, 0.25, 10, cpv(80, 20));
    giverandspin(objx);


/* randomly placed objects... */

    // create player one
    objx = makeplayer(objx, space, P_ONE);
    giverandspin(objx);

    // create player two
    objx = makeplayer(objx, space, P_TWO);
    giverandspin(objx);

    for (i = 0; i < 7; i++) {
	objx = makecirc(objx, space, false, 0.25, 5, randfit(objx, 5));
	giverandspin(objx);
	objx = makefloat(objx, space, 0.08, 2.0, randfit(objx, 2));
	giverandspin(objx);
	objx = makefloat(objx, space, 0.08, 2.0, randfit(objx, 2));
	giverandspin(objx);
    }

    cpSpaceAddCollisionHandler(space, C_SHIP, C_LARGE, NULL, *chcolor,
			       NULL, NULL, objroot);
    cpSpaceAddCollisionHandler(space, C_LARGE, C_SMALL, NULL, *chcolor,
			       NULL, NULL, objroot);
    cpSpaceAddCollisionHandler(space, C_SMALL, C_SHIP, NULL, *chcolor,
			       NULL, NULL, objroot);

    return space;
}

// makes a circle, adds it to the space and then returns a handle
struct objnode *makecirc(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, cpFloat radius, cpVect pos)
{
    objx = makenode(objx);
    objx->geom = S_CIRC;

    if (statb) {
	objx->s = cpCircleShapeNew(space->staticBody, radius, cpvzero);
	objx->b = space->staticBody;
    } else {
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	objx->b = cpBodyNew(mass, moment);
	objx->s = cpCircleShapeNew(objx->b, radius, cpvzero);
    }

    insertobj(space, objx);
    if (!statb)
	cpBodySetPos(objx->b, pos);

    objx->radius = radius;

    cpShapeSetLayers(objx->s, NONBH);

    cpBodySetAngVel(objx->b, 20);
    cpShapeSetFriction(objx->s, 0.7);
    cpShapeSetElasticity(objx->s, 0.7f);

    // defualts
    objx->colortype = COLOR_LARGE;
    objx->s->collision_type = C_LARGE;

    return objx;
}

// creates any sort of polygon...
struct objnode *makepoly(struct objnode *objx, cpSpace * space, bool statb,
			 cpFloat mass, int nverts, cpVect * verts,
			 cpVect pos)
{
    objx = makenode(objx);
    objx->geom = S_POLY;

    if (statb) {
	objx->s =
	    cpPolyShapeNew(space->staticBody, nverts, verts, cpvzero);
	objx->b = space->staticBody;
    } else {
	cpFloat moment = cpMomentForPoly(mass, nverts, verts, cpvzero);
	objx->b = cpBodyNew(mass, moment);
	objx->s = cpPolyShapeNew(objx->b, nverts, verts, cpvzero);
    }

    insertobj(space, objx);
    if (!statb)
	cpBodySetPos(objx->b, pos);

    int i;
    for (i = 0; i < nverts; i++)
	if (cpvlength(verts[i]) > objx->radius)
	    objx->radius = cpvlength(verts[i]);

    cpShapeSetLayers(objx->s, NONBH);

    cpShapeSetFriction(objx->s, 0.7f);
    cpShapeSetElasticity(objx->s, 0.4f);

    // defaults
    objx->colortype = COLOR_LARGE;
    objx->s->collision_type = C_LARGE;

    return objx;
}

// creates a static boundary...
struct objnode *makeline(struct objnode *objx, cpSpace * space,
			 bool statb, cpVect v1, cpVect v2)
{
    objx = makenode(objx);
    objx->geom = S_LSEG;

    if (statb) {
	objx->s = cpSegmentShapeNew(space->staticBody, v1, v2, 0);
	objx->b = space->staticBody;
    } else {
	fprintf(stderr,
		"Error, non-static line segments are not supported.\n");
	exit(3);
    }

    insertobj(space, objx);

    cpShapeSetFriction(objx->s, 1);
    cpShapeSetElasticity(objx->s, 0.7f);

    objx->colortype = COLOR_LINE;
    objx->s->collision_type = C_NONE;

    return objx;
}



// makes a circle unaffected by vertical gravity. attracted to bholes
struct objnode *makefloat(struct objnode *objx, cpSpace * space,
			  cpFloat mass, cpFloat radius, cpVect pos)
{
    objx = makecirc(objx, space, false, mass, radius, pos);

    objx->b->velocity_func = &orbit;

    // don't collide with black holes
    cpShapeSetGroup(objx->s, FLOATG);

    cpBodySetAngVel(objx->b, 2);

    objx->colortype = COLOR_SMALL;
    objx->s->collision_type = C_SMALL;

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

    // don't collide
    cpShapeSetGroup(objx->s, FLOATG);
    cpShapeSetLayers(objx->s, BHOLE);

    cpBodySetVelLimit(objx->b, 0);
    cpBodySetAngVel(objx->b, 10);

    objx->colortype = COLOR_BHOLE;

    return objx;
}

// make a ship for player number playernum
struct objnode *makeplayer(struct objnode *objx, cpSpace * space,
			   int playernum)
{
    objx = maketria(objx, space, 1.33, 20, 8, randfit(objx, 10));
    objx->ownedby = playernum;
    initplayer(objx);

    objx->colortype = COLOR_SHIP;
    objx->s->collision_type = C_SHIP;

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

// sets the object spinning
void giverandspin(struct objnode *objx)
{
    cpBodySetVel(objx->b, cpv(randrange(-60, 60), randrange(-60, 60)));
}

// picks a random spot to place an object, until nearobjs() returns false.
cpVect randfit(struct objnode *objlast, cpFloat r)
{
    int i;
    cpVect xymin, xymax;
    cpVect randv;

    xymin = cpv(XMIN + XYBUF, YMIN + XYBUF);
    xymax = cpv(XMAX - XYBUF, YMAX - XYBUF);

    for (i = 0; i < 1000; i++) {
	randv.x = randrange(xymin.x, xymax.x);
	randv.y = randrange(xymin.y, xymax.y);
	if (nearobjs(objlast, randv, r))
	    return randv;
    }

    fprintf(stderr,
	    "Error: cannot find a random place to fit an object.\n");
    exit(4);

    return randv;
}

// returns a random number between min and max
cpFloat randrange(cpFloat min, cpFloat max)
{
    return min + random() / (double) RAND_MAX *(max - min);
}

// returns true if an object with intersects with another object.
// (crudely calculated with max radius from center of gravity.)
bool nearobjs(struct objnode *objlast, cpVect randv, cpFloat radius)
{
    cpFloat dist;
    struct objnode *objch = objlast;

    while ((objch = objch->prev)) {

	if (objch->geom == S_CIRC || objch->geom == S_POLY)
	    dist = cpvlength(cpvsub(randv, cpBodyGetPos(objch->b)));
	else if (objch->geom == S_LSEG) {
	    cpVect enda, endb, segv, posv;
	    cpFloat angle;

	    enda = cpSegmentShapeGetA(objch->s);
	    endb = cpSegmentShapeGetB(objch->s);
	    if (cpvlength(cpvsub(randv, enda)) <
		cpvlength(cpvsub(randv, endb))) {
		segv = cpvsub(enda, endb);
		posv = cpvsub(randv, endb);
	    } else {
		segv = cpvsub(endb, enda);
		posv = cpvsub(randv, enda);
	    }
	    angle = cpvtoangle(posv) - cpvtoangle(segv);
	    if (fabs(angle) < PI / 2)
		dist = fabs(sin(angle) * cpvlength(posv));
	    else
		dist = cpvlength(posv);
	} else
	    dist = INFINITY;

	if (dist > objch->radius + radius)
	    continue;
	else
	    return false;
    }

    return true;
}

// inserts a struct for pointing to a new body/shape
struct objnode *makenode(struct objnode *objx)
{
    struct objnode *objnew;

    objnew = malloc(sizeof(struct objnode));

    objnew->geom = S_NONE;
    objnew->radius = 0;
    objnew->b = NULL;
    objnew->s = NULL;
    objnew->bhole = false;
    objnew->pinfo = NULL;
    objnew->ownedby = P_NONE;
    objnew->colortype = COLOR_NONE;
    curtime(&objnew->lastchange);

    objnew->next = objx->next;
    objnew->prev = objx;
    objx->next = objnew;

    return objnew;
}

// initializes playerinfo struct. starts with no thrust and default hp
void initplayer(struct objnode *player)
{
    player->pinfo = malloc(sizeof(struct playerinfo));

    player->pinfo->hp = HPSTART;

    player->pinfo->thrust.prevf.force = cpvzero;
    player->pinfo->thrust.prevf.tforce = cpvzero;
    player->pinfo->thrust.markt.tv_sec = 0;
    player->pinfo->thrust.markt.tv_nsec = 0;

    player->pinfo->thrust.up = false;
    player->pinfo->thrust.down = false;
    player->pinfo->thrust.left = false;
    player->pinfo->thrust.right = false;
    player->pinfo->thrust.cw = false;
    player->pinfo->thrust.cwt = 0;
    player->pinfo->thrust.ccw = false;
    player->pinfo->thrust.ccwt = 0;

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
    free(objx->pinfo);

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
