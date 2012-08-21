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
    { S_NONE, 0, NULL, NULL, false, NULL, P_NONE, COLOR_NONE, NULL, NULL }
};

int main(void)
{
    SDL_Surface *screen, *sdlbuff;
    cairo_surface_t *surface;
    cairo_t *cr;
    long simtime, acc;

    cpSpace *space;

    graphicsinit(&screen, &sdlbuff, &surface, &cr);
    initcolors();
    space = makeshapes(objroot);


    simtime = 0, acc = 0;
    for (;;) {

	simtime = drawshapes(screen, sdlbuff, cr, objroot);

	// control movment of the ships
	interact(space, objroot, &screen);

	// simulate the amount of real time since the last simulation.
	for (acc += simtime; acc > DT; acc -= DT)
	    cpSpaceStep(space, (double) DT / 1e9);

    }

    // clean up
    rmobjs(objroot);
    cpSpaceFree(space);
    cairo_surface_destroy(surface);
    hdestroy();
    freeentries();
    SDL_FreeSurface(sdlbuff);

    return 0;
}

