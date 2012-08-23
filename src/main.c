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


struct objnode objroot[1] = {
    {S_NONE, 0, NULL, NULL, false, NULL, P_NONE, COLOR_NONE, {0, 0}, NULL, NULL}
};

bool someonelost(struct objnode *objroot);
void cleanup(SDL_Surface * sdlbuf, cairo_surface_t * surface, struct objnode
	     *objroot, cpSpace * space);

int main(void)
{
    SDL_Surface *screen, *sdlbuff;
    cairo_surface_t *surface;
    cairo_t *cr;
    long simtime, acc;
    bool play;

    cpSpace *space;

    graphicsinit(&screen, &sdlbuff, &surface, &cr);
    initcolors();
    space = makeshapes(objroot);

    play = true;
    simtime = 0, acc = 0;
    while (play == true) {

	// sleep and set the amount of time to be simulated
	simtime = timebal();

	// draw stuff to the screen, including fps
	drawshapes(screen, sdlbuff, cr, objroot, simtime);

	// control movment of the ships
	play = interact(space, objroot, &screen);

	// simulate the amount of real time since the last simulation.
	for (acc += simtime; acc > DT; acc -= DT)
	    cpSpaceStep(space, (double) DT / 1e9);

	// test if someone has lost, and print out winner's name
	if (someonelost(objroot)) {
	    showwinner(screen, sdlbuff, cr, objroot, simtime);
	    break;
	}
    }

    cleanup(sdlbuff, surface, objroot, space);

    return 0;
}

// returns true if someone lost the game, i.e. someone else won.
bool someonelost(struct objnode *objroot)
{
    struct objnode *player1, *player2;
    int hp1, hp2;

    player1 = findplayer(objroot, P_ONE);
    player2 = findplayer(objroot, P_TWO);

    hp1 = player1->pinfo->hp;
    hp2 = player2->pinfo->hp;

    if (hp1 <= 0 || hp2 <= 0)
	return true;
    else
	return false;

}

// free up some memory
void cleanup(SDL_Surface * sdlbuff, cairo_surface_t * surface, struct objnode
	     *objroot, cpSpace * space)
{
    SDL_FreeSurface(sdlbuff);
    cairo_surface_destroy(surface);
    rmobjs(objroot);
    cpSpaceFree(space);
    freecolorentries();
    hdestroy();

}
