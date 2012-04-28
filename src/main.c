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

struct objnode objroot[1] =
    { {S_NONE, false, NULL, NULL, 0, {0, 0, 0, 0}, {0, 0, 0, 0}, NULL,
       NULL}
};


int main(void)
{
    SDL_Surface *screen, *sdlbuff;
    cairo_surface_t *surface;
    cairo_t *cr;

    cpSpace *space;
    struct objnode *vehicle;

    graphicsinit(&screen, &sdlbuff, &surface, &cr);
    space = makeshapes(objroot, &vehicle);


    long t = 0, simtime = 0, acc = 0;
    while (t < DURATION) {

	simtime = drawshapes(screen, sdlbuff, cr, objroot);

	interact(space, objroot, vehicle);

	for (acc += simtime; acc > DT; acc -= DT, t += DT)
	    cpSpaceStep(space, (double) DT / 1e9);

    }

    rmobjs(objroot);
    cpSpaceFree(space);
    cairo_surface_destroy(surface);

    return 0;
}
