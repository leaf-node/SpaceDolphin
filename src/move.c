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
    bool up;
    bool down;
    bool left;
    bool right;
    bool cw;
    cpFloat cwt;
    bool ccw;
    cpFloat ccwt;
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
	      struct objnode *vehicle, SDL_Surface ** screen)
{
    SDL_Event event;
    bool togglefsm = false;
    static struct movement direct[1] =
	{ {false, false, false, false, false, 0, false, 0} };
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
	    case SDLK_f:
		togglefsm = true;
		break;
	    case SDLK_q:
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

    if (togglefsm == true)
	*screen = togglefullscreen();

}

// calculates and adds forces triggered by holding the movement keys down
// it also has to stop the forces applied from the prev call to this function
void blastengines(struct objnode *vehicle, struct movement *direct)
{
    cpFloat force = 0, tforce = 0;
    static struct forces prevf = { {0, 0}, {0, 0} };
    struct forces newf;
    cpVect rotv = cpBodyGetRot(vehicle->b);
    cpFloat angvel;
    long dt;
    struct timespec now;
    static struct timespec markt;

    // these test cases are to enforce soft limits on the rate of movement
    if (direct->up && !direct->down) {
	if (cpvunrotate(cpBodyGetVel(vehicle->b), rotv).y < MAXVEL)
	    force = FORCE;
    } else if (!direct->up && direct->down) {
	if (cpvunrotate(cpBodyGetVel(vehicle->b), rotv).y > -MAXVEL)
	    force = -FORCE;
    }

    curtime(&now);
    dt = convtns(tdiff(now, markt));
    markt = now;

    tforce = 0;
    angvel = cpBodyGetAngVel(vehicle->b);
    if (direct->ccw) {
	direct->ccwt += dt;
	direct->ccwt =
	    (direct->ccwt > TORQRAMPT) ? TORQRAMPT : direct->ccwt;

	if (angvel < MAXANGVEL)
	    tforce += TFORCE * sqrtl(direct->ccwt / TORQRAMPT);
    } else if (direct->ccwt > 0) {
	direct->ccwt -= dt;
	direct->ccwt = (direct->ccwt < 0) ? 0 : direct->ccwt;
    }

    if (direct->cw) {
	direct->cwt += dt;
	direct->cwt = (direct->cwt > TORQRAMPT) ? TORQRAMPT : direct->cwt;

	if (angvel > -MAXANGVEL)
	    tforce += -TFORCE * sqrtl(direct->cwt / TORQRAMPT);
    } else if (direct->cwt > 0) {
	direct->cwt -= dt;
	direct->cwt = (direct->cwt < 0) ? 0 : direct->cwt;
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
    cpVect bpos, hpos, unitv, gvect;
    cpFloat hmass, g, distsq;

    objx = objroot;
    while ((objx = objx->next) != NULL) {
	if (objx->bhole == true && objx->b != body) {
	    hole = objx->b;
	    bpos = cpBodyGetPos(body);
	    hpos = cpBodyGetPos(hole);

	    distsq = cpvdistsq(hpos, bpos);
	    distsq = (distsq == 0) ? 1e-50 : distsq;	// don't divide by zero

	    hmass = cpBodyGetMass(hole);
	    g = hmass * BGRAV * (1 / distsq);
	    g = (distsq < RDSQ) ? g * -REPFS : g;   // shoot close balls away

	    unitv = cpvmult(cpvsub(hpos, bpos), (1 / sqrt(distsq)));
	    gvect = cpvmult(unitv, g);

	    cpBodyUpdateVelocity(body, gvect, damping, dt);
	}
    }
}
