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

enum { OFF, UP, DOWN, LEFT, RIGHT, R_LEFT, R_RIGHT };

// remembers which key presses/forces are active
struct movement {
    cpFloat up;
    cpFloat down;
    cpFloat left;
    cpFloat right;
    cpFloat cw;
    cpFloat ccw;
};

// forces from previous calculation
struct forces {
    cpVect force;
    cpVect tforce;
};


void blastengines(struct objnode *vehicle, struct movement *direct);
void applyforces(struct objnode *vehicle, struct forces f);


// query for button press change, then call applyforces()
void interact(cpSpace * space, struct objnode *objroot,
	      struct objnode *vehicle)
{
    SDL_Event event;

    static struct movement direct[1] = { {0, 0, 0, 0, 0, 0} };
    while (SDL_PollEvent(&event)) {
	switch (event.type) {
	case SDL_KEYDOWN:
	    switch (event.key.keysym.sym) {
	    case SDLK_UP:
		direct->up = true;
		break;
	    case SDLK_DOWN:
		direct->down = true;
		break;
	    case SDLK_LEFT:
		direct->ccw = true;
		break;
	    case SDLK_RIGHT:
		direct->cw = true;
		break;
	    case SDLK_ESCAPE:
		rmobjs(objroot);
		cpSpaceFree(space);
		exit(0);
		break;
	    default:
		break;
	    }
	    break;
	case SDL_KEYUP:
	    switch (event.key.keysym.sym) {
	    case SDLK_UP:
		direct->up = false;
		break;
	    case SDLK_DOWN:
		direct->down = false;
		break;
	    case SDLK_LEFT:
		direct->ccw = false;
		break;
	    case SDLK_RIGHT:
		direct->cw = false;
		break;
	    default:
		break;
	    }
	    break;
	case SDL_QUIT:
	    rmobjs(objroot);
	    cpSpaceFree(space);
	    exit(0);
	    break;
	default:
	    break;
	}
    }

    blastengines(vehicle, direct);

}

// calculates and adds forces triggered by holding the movement keys down
// it also has to stop the forces applied from the prev call to this function
void blastengines(struct objnode *vehicle, struct movement *direct)
{
    cpFloat force = 0, tforce = 0;
    static struct forces prevf = { {0, 0}, {0, 0} };
    struct forces newf;
    cpVect rotv = cpBodyGetRot(vehicle->b);

    // these test cases are to enforce soft limits on the rate of movement
    if (direct->up && !direct->down) {
	if (cpvunrotate(cpBodyGetVel(vehicle->b), rotv).y < MAXVEL)
	    force = FORCE;
    } else if (!direct->up && direct->down) {
	if (cpvunrotate(cpBodyGetVel(vehicle->b), rotv).y > -MAXVEL)
	    force = -FORCE;
    }

    if (direct->ccw && !direct->cw) {
	if (cpBodyGetAngVel(vehicle->b) < MAXANGVEL)
	    tforce = TFORCE;
    } else if (!direct->ccw && direct->cw) {
	if (cpBodyGetAngVel(vehicle->b) > -MAXANGVEL)
	    tforce = -TFORCE;
    }

    newf.force = cpvrotate(cpv(0, force), rotv);
    newf.tforce = cpv(0, tforce);

    applyforces(vehicle, prevf);
    applyforces(vehicle, newf);

    prevf.force = cpvneg(newf.force);
    prevf.tforce = cpvneg(newf.tforce);
}

// apply a lasting external force to the center of gravity
// apply lasting forces perpendicluar to vectors from the c.o.g.
// (in other words, push and rotate the object)
// these need to be subtracted later to stop their effect
void applyforces(struct objnode *vehicle, struct forces f)
{
    cpBodyApplyForce(vehicle->b, f.force, cpvzero);
    cpBodyApplyForce(vehicle->b, f.tforce, cpv(RLEN, 0));
    cpBodyApplyForce(vehicle->b, cpvneg(f.tforce), cpv(-RLEN, 0));
}

// make no changes to a body's movement
void dontfall(cpBody * body, cpVect gravity, cpFloat damping, cpFloat dt)
{
    ;
}

// set gravity to point towards black holes
void orbit(cpBody * body, cpVect gravity, cpFloat damping, cpFloat dt)
{
    extern struct objnode objroot[];
    struct objnode *objx;
    struct cpBody *hole;
    cpVect bpos, hpos, g = cpvzero;
    cpFloat hmass, distsq;

    objx = objroot;
    while ((objx = objx->next) != NULL) {
	if (objx->bhole == true && objx->b != body) {
	    hole = objx->b;
	    bpos = cpBodyGetPos(body);
	    hpos = cpBodyGetPos(hole);
	    distsq = cpvdistsq(hpos, bpos) ? cpvdistsq(hpos, bpos) : 1e-50;
	    hmass = cpBodyGetMass(hole);
	    g = cpvmult(cpvsub(hpos, bpos), hmass * BGRAV / distsq);
	    cpBodyUpdateVelocity(body, g, damping, dt);
	}
    }
}