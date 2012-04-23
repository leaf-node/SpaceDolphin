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


// draw every shape in the link list connected to objroot
void drawshapes(SDL_Surface * screen, SDL_Surface * sdlbuff, cairo_t * cr,
		struct objnode *objroot, long simtime)
{
    struct objnode *objx = objroot;

    cairoerase(cr);
    while ((objx = objx->next) != NULL) {
	switch (objx->kind) {
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
#if SHOWFPS
    drawfps(cr, simtime);
#endif

    SDL_BlitSurface(sdlbuff, NULL, screen, NULL);
    SDL_Flip(screen);

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

    cairo_set_line_width(cr, 0.25);
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

    cairo_set_line_width(cr, 0.25);
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

    cairo_set_line_width(cr, 0.25);
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
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, 160, 0);
    cairo_line_to(cr, 160, 120);
    cairo_line_to(cr, 0, 120);
    cairo_close_path(cr);
    cairo_fill(cr);
}


// creates a screen to draw on
void graphicsinit(SDL_Surface ** screen, SDL_Surface ** sdlbuff,
		  cairo_surface_t ** surface, cairo_t ** cr)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

    *screen = SDL_SetVideoMode(640, 480, 32, 0x0);
    *sdlbuff = SDL_CreateRGBSurface(0, 640, 480, 32,
				    0x00FF0000, 0x0000FF00, 0x000000FF,
				    0x0);
    if (*screen == NULL) {
	printf("Unable to set video mode: %s\n", SDL_GetError());
	exit(1);
    }

    *surface =
	cairo_image_surface_create_for_data((*sdlbuff)->pixels,
					    CAIRO_FORMAT_RGB24,
					    (*sdlbuff)->w, (*sdlbuff)->h,
					    (*sdlbuff)->pitch);

    *cr = cairo_create(*surface);

    cairo_scale(*cr, 4.0, -4.0);
    //cairo_translate(*cr, 79.875, -119.875);
    cairo_translate(*cr, -0.125, -119.875);

}

