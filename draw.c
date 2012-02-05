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

void drawcirc(SDL_Surface * screen, struct objnode *objx);
void drawlseg(SDL_Surface * screen, struct objnode *objx);
void drawpoly(SDL_Surface * screen, struct objnode *objx);
void drawfps(SDL_Surface * screen, long simtime);


// draw every shape in the link list connected to objroot
void drawshapes(SDL_Surface * screen, struct objnode *objroot,
		long simtime)
{
    struct objnode *objx = objroot;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    while ((objx = objx->next)) {
	switch (objx->kind) {
	case S_CIRC:
	    drawcirc(screen, objx);
	    break;
	case S_LSEG:
	    drawlseg(screen, objx);
	    break;
	case S_POLY:
	    drawpoly(screen, objx);
	    break;
	}
    }
#if SHOWFPS
    drawfps(screen, simtime);
#endif

    SDL_Flip(screen);

}

// draws a circle on the screen
void drawcirc(SDL_Surface * screen, struct objnode *objx)
{
    cpVect pos, radi;

    pos = cpBodyGetPos(objx->b);
    filledCircleColor(screen, SX(pos.x), SY(pos.y),
		      SS(cpCircleShapeGetRadius(objx->s)), objx->c1);
    aacircleColor(screen, SX(pos.x), SY(pos.y),
		  SS(cpCircleShapeGetRadius(objx->s)), objx->c2);
    radi = cpvmult(cpvforangle(cpBodyGetAngle(objx->b)),
		   cpCircleShapeGetRadius(objx->s));
    aalineColor(screen,
		SX(pos.x), SY(pos.y), SX(radi.x + pos.x),
		SY(radi.y + pos.y), objx->c2);
}

// draws a line segment
void drawlseg(SDL_Surface * screen, struct objnode *objx)
{
    cpVect enda, endb;

    enda = cpSegmentShapeGetA(objx->s);
    endb = cpSegmentShapeGetB(objx->s);
    aalineColor(screen, SX(enda.x), SY(enda.y), SX(endb.x), SY(endb.y),
		objx->c1);
}

// draws a polygon
void drawpoly(SDL_Surface * screen, struct objnode *objx)
{
    int i, numv;
    cpVect pos, rotv, vert;
    Sint16 xverts[4], yverts[4];

    pos = cpBodyGetPos(objx->b);
    rotv = cpBodyGetRot(objx->b);
    numv = cpPolyShapeGetNumVerts(objx->s);
    for (i = 0; i < numv; i++) {
	vert =
	    cpvadd(pos, cpvrotate(cpPolyShapeGetVert(objx->s, i), rotv));
	xverts[i] = SX(vert.x);
	yverts[i] = SY(vert.y);
    }
    filledPolygonColor(screen, xverts, yverts, numv, objx->c1);
    aapolygonColor(screen, xverts, yverts, numv, objx->c2);
}

// draws frames per second on screen (converts from simulation time)
void drawfps(SDL_Surface * screen, long simtime)
{
    int fps;
    double simrate;

    framerate(simtime, &simrate, &fps);
    if (fps >= 0) {
	char s[100];
	sprintf(s, "Framerate: %3d fps", fps);
	stringColor(screen, 10, 10, s, 0xFFFFFFFF);
	sprintf(s, "Simulation rate: %1.2lfx", simrate);
	stringColor(screen, 10, 20, s, 0xFFFFFFFF);
    }
}

// creates a screen to draw on
void graphicsinit(SDL_Surface ** screen)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

    *screen = SDL_SetVideoMode(640, 480, 16, SDL_DOUBLEBUF);
    if (*screen == NULL) {
	printf("Unable to set video mode: %s\n", SDL_GetError());
	exit(1);
    }
}
