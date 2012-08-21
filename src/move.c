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

void blastengines(struct objnode *player);
void applyforces(struct objnode *player, struct forces f);
struct objnode *findplayer(struct objnode *objroot, int playernum);
void initthrust(struct objnode *player);


// query for button press change, then start moving the players' ships.
void interact(cpSpace * space, struct objnode *objroot,
	      SDL_Surface ** screen)
{
    SDL_Event event;
    bool togglefsm = false;
    struct objnode *player1, *player2;

    player1 = findplayer(objroot, P_ONE);

    player2 = findplayer(objroot, P_TWO);

    while (SDL_PollEvent(&event)) {
	switch (event.type) {
	case SDL_KEYDOWN:
	    switch (event.key.keysym.sym) {

	    case SDLK_UP:
		player1->pinfo->thrust.up = true;
		break;
	    case SDLK_DOWN:
		player1->pinfo->thrust.down = true;
		break;
	    case SDLK_LEFT:
		player1->pinfo->thrust.ccw = true;
		break;
	    case SDLK_RIGHT:
		player1->pinfo->thrust.cw = true;
		break;

	    case SDLK_w:
		player2->pinfo->thrust.up = true;
		break;
	    case SDLK_s:
		player2->pinfo->thrust.down = true;
		break;
	    case SDLK_a:
		player2->pinfo->thrust.ccw = true;
		break;
	    case SDLK_d:
		player2->pinfo->thrust.cw = true;
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
		player1->pinfo->thrust.up = false;
		break;
	    case SDLK_DOWN:
		player1->pinfo->thrust.down = false;
		break;
	    case SDLK_LEFT:
		player1->pinfo->thrust.ccw = false;
		break;
	    case SDLK_RIGHT:
		player1->pinfo->thrust.cw = false;
		break;

	    case SDLK_w:
		player2->pinfo->thrust.up = false;
		break;
	    case SDLK_s:
		player2->pinfo->thrust.down = false;
		break;
	    case SDLK_a:
		player2->pinfo->thrust.ccw = false;
		break;
	    case SDLK_d:
		player2->pinfo->thrust.cw = false;
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

    blastengines(player1);
    blastengines(player2);

    if (togglefsm == true)
	*screen = togglefullscreen();

}

// calculates and adds forces triggered by holding the movement keys down.
// it also has to stop the forces applied by the prev call to this function
void blastengines(struct objnode *player)
{
    struct movement *thrust;
    cpFloat force = 0, tforce = 0;
    struct forces newf;
    cpVect rotv = cpBodyGetRot(player->b);
    cpFloat angvel;
    long dt;
    struct timespec now;

    thrust = &player->pinfo->thrust;

    // these test cases are to enforce soft limits on the rate of movement
    if (thrust->up && !thrust->down) {
	if (cpvunrotate(cpBodyGetVel(player->b), rotv).y < MAXVEL)
	    force = FORCE;
    } else if (!thrust->up && thrust->down) {
	if (cpvunrotate(cpBodyGetVel(player->b), rotv).y > -MAXVEL)
	    force = -FORCE;
    }

    curtime(&now);
    dt = convtns(tdiff(now, thrust->markt));
    thrust->markt = now;

    tforce = 0;
    angvel = cpBodyGetAngVel(player->b);
    if (thrust->ccw) {
	thrust->ccwt += dt;
	thrust->ccwt =
	    (thrust->ccwt > TORQRAMPT) ? TORQRAMPT : thrust->ccwt;

	if (angvel < MAXANGVEL)
	    tforce += TFORCE * sqrtl(thrust->ccwt / TORQRAMPT);
    } else if (thrust->ccwt > 0) {
	thrust->ccwt -= dt;
	thrust->ccwt = (thrust->ccwt < 0) ? 0 : thrust->ccwt;
    }

    if (thrust->cw) {
	thrust->cwt += dt;
	thrust->cwt = (thrust->cwt > TORQRAMPT) ? TORQRAMPT : thrust->cwt;

	if (angvel > -MAXANGVEL)
	    tforce += -TFORCE * sqrtl(thrust->cwt / TORQRAMPT);
    } else if (thrust->cwt > 0) {
	thrust->cwt -= dt;
	thrust->cwt = (thrust->cwt < 0) ? 0 : thrust->cwt;
    }

    newf.force = cpvrotate(cpv(0, force), rotv);
    newf.tforce = cpv(0, tforce);

    applyforces(player, thrust->prevf);
    applyforces(player, newf);

    thrust->prevf.force = cpvneg(newf.force);
    thrust->prevf.tforce = cpvneg(newf.tforce);
}

// apply a lasting external force to the center of gravity.
// also apply lasting forces perpendicluar to vectors from the c.o.g.
// (in other words, push and rotate the object)
// these need to be subtracted later to stop their effect
void applyforces(struct objnode *player, struct forces f)
{
    cpBodyApplyForce(player->b, f.force, cpvzero);
    cpBodyApplyForce(player->b, f.tforce, cpv(RLEN, 0));
    cpBodyApplyForce(player->b, cpvneg(f.tforce), cpv(-RLEN, 0));
}

// make no changes to a body's movement
void dontfall(cpBody * body, cpVect gravity, cpFloat damping, cpFloat dt)
{
    ;
}

// set gravity (of previously chosen objects) to point towards black holes
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

// finds the specified player in the linked list of objects.
struct objnode *findplayer(struct objnode *objroot, int playernum)
{
    struct objnode *objx;

    for (objx = objroot; objx != NULL; objx = objx->next)
	if (objx->s != NULL && objx->s->collision_type == C_SHIP \
		&& objx->ownedby == playernum) {
	    if (objx->pinfo == NULL)
		initthrust(objx);
	    return objx;
	}

    fprintf(stderr, "*** Error, could not find player %d\n", playernum);
    exit(5);

}

// initializes playerinfo struct. starts with no thrust.
void initthrust(struct objnode *player)
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

