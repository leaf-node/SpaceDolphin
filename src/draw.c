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

void drawcirc(cairo_t * cr, struct objnode *objx);
void drawlseg(cairo_t * cr, struct objnode *objx);
void drawpoly(cairo_t * cr, struct objnode *objx);
void drawfps(cairo_t * cr, long simtime);
void cairoerase(cairo_t * cr);
SDL_Surface *setupSDLscreen(void);


// draw every shape in the link list connected to objroot
long drawshapes(SDL_Surface * screen, SDL_Surface * sdlbuff, cairo_t * cr,
		struct objnode *objroot)
{
    struct objnode *objx = objroot;
    long simtime;

    cairoerase(cr);
    while ((objx = objx->next) != NULL) {
	switch (objx->geom) {
	case S_CIRC:
	    drawcirc(cr, objx);
	    break;
	case S_LSEG:
	    drawlseg(cr, objx);
	    break;
	case S_POLY:
	    drawpoly(cr, objx);
	    break;
	}
    }
    simtime = timebal();

#if SHOWFPS
    drawfps(cr, simtime);
#endif

    SDL_BlitSurface(sdlbuff, NULL, screen, NULL);
    SDL_Flip(screen);

    return simtime;
}

// draws a circle on the cairo surface
void drawcirc(cairo_t * cr, struct objnode *objx)
{
    cpFloat radius;
    cpVect pos, radiusv;

    pos = cpBodyGetPos(objx->b);
    radius = cpCircleShapeGetRadius(objx->s);
    radiusv = cpvmult(cpvforangle(cpBodyGetAngle(objx->b)),
		      cpCircleShapeGetRadius(objx->s));

    cairo_arc(cr, pos.x, pos.y, radius, 0, 2 * M_PI);

    cairo_set_source_rgba(cr, objx->c1.r, objx->c1.g, objx->c1.b,
			  objx->c1.a);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, objx->c2.r, objx->c2.g, objx->c2.b,
			  objx->c2.a);
    cairo_stroke(cr);

    cairo_move_to(cr, pos.x, pos.y);
    cairo_rel_line_to(cr, radiusv.x, radiusv.y);
    cairo_stroke(cr);

}

// draws a line segment
void drawlseg(cairo_t * cr, struct objnode *objx)
{
    cpVect enda, endb;

    enda = cpSegmentShapeGetA(objx->s);
    endb = cpSegmentShapeGetB(objx->s);

    cairo_move_to(cr, enda.x, enda.y);
    cairo_line_to(cr, endb.x, endb.y);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, objx->c1.r, objx->c1.g, objx->c1.b,
			  objx->c1.a);
    cairo_stroke(cr);

}

// draws a polygon
void drawpoly(cairo_t * cr, struct objnode *objx)
{
    int i, numv;
    cpVect pos, rotv, vert;

    pos = cpBodyGetPos(objx->b);
    rotv = cpBodyGetRot(objx->b);
    numv = cpPolyShapeGetNumVerts(objx->s);

    for (i = 0; i < numv; i++) {
	vert =
	    cpvadd(pos, cpvrotate(cpPolyShapeGetVert(objx->s, i), rotv));

	if (i == 0)
	    cairo_move_to(cr, vert.x, vert.y);
	else
	    cairo_line_to(cr, vert.x, vert.y);
    }

    cairo_close_path(cr);

    cairo_set_source_rgba(cr, objx->c1.r, objx->c1.g, objx->c1.b,
			  objx->c1.a);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, objx->c2.r, objx->c2.g, objx->c2.b,
			  objx->c2.a);
    cairo_stroke(cr);

}

// draws frames per second on surface (converts from simulation time)
void drawfps(cairo_t * cr, long simtime)
{
    int fps;
    double simrate;
    //cairo_text_extents_t te;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_font_size(cr, 3.5);

    framerate(simtime, &simrate, &fps);

    if (fps >= 0) {
	char s[100];
	sprintf(s, "Framerate: %3d fps", fps);
	//   cairo_text_extents(cr, s, &te);
	cairo_scale(cr, 1.0, -1.0);
	cairo_move_to(cr, 3, -114);
	cairo_show_text(cr, s);
	sprintf(s, "Simulation rate: %1.2lfx", simrate);
	cairo_move_to(cr, 3, -110);
	cairo_show_text(cr, s);
	cairo_scale(cr, 1.0, -1.0);
    }

}

// erases the surface used by cairo
void cairoerase(cairo_t * cr)
{
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

}


// creates a screen and buffers to draw on
void graphicsinit(SDL_Surface ** screen, SDL_Surface ** sdlbuff,
		  cairo_surface_t ** surface, cairo_t ** cr)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
	exit(1);
    }
    SDL_ShowCursor(SDL_DISABLE);
    atexit(SDL_Quit);

    *screen = setupSDLscreen();

    *sdlbuff = SDL_CreateRGBSurface(0, XMAX * SCALEF, YMAX * SCALEF, 32,
				    0x00FF0000, 0x0000FF00, 0x000000FF,
				    0x0);
    *surface =
	cairo_image_surface_create_for_data((*sdlbuff)->pixels,
					    CAIRO_FORMAT_RGB24,
					    (*sdlbuff)->w, (*sdlbuff)->h,
					    (*sdlbuff)->pitch);

    *cr = cairo_create(*surface);

    cairo_translate(*cr, -0.5, -0.5);	// align to pixel center
    cairo_scale(*cr, SCALEF, -SCALEF);	// scale + flip image over x axis
    cairo_translate(*cr, XMIN, -YMAX);	// shift image vertically

}

bool fullscreen = FULLSCREEN;

// setup SDL screen stuff
SDL_Surface *setupSDLscreen(void)
{
    SDL_Surface *screen;

    if (fullscreen == true)
	screen = SDL_SetVideoMode(XMAX * SCALEF, YMAX * SCALEF, 32,
				  SDL_FULLSCREEN | SDL_HWSURFACE |
				  SDL_DOUBLEBUF);
    else
	screen = SDL_SetVideoMode(XMAX * SCALEF, YMAX * SCALEF, 32, 0x0);

    if (screen == NULL) {
	fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
	exit(1);
    }
#if DEBUG == true
    if (screen->flags & SDL_FULLSCREEN)
	printf("fullscreen, ");
    else
	printf("windowed, ");
    if (screen->flags & SDL_HWSURFACE)
	printf("uses hardware surface.\n");
    else
	printf("doesn't use hardware surface.\n");
#endif

    return screen;
}

// toggle fullscreen mode on and off
SDL_Surface *togglefullscreen(void)
{
    SDL_Surface *screen;

    fullscreen = !fullscreen;
    screen = setupSDLscreen();

    return screen;
}
