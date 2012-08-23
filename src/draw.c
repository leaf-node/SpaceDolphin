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
void drawhpmeters(cairo_t *cr, int hpmax, int hp1, int hp2);
void drawhpmeter(cairo_t *cr, struct colorset *colors, double hpratio,
		 int direction);
void showwinnertext(cairo_t *cr, char *name);
void centermessage(cairo_t *cr, char *str, struct color_rgba *color);
void drawstringxy(cairo_t *cr, char *str, struct color_rgba *color,
		  double x, double y);
void drawtoscreen(SDL_Surface *screen, SDL_Surface *sdlbuff);
void cairoerase(cairo_t * cr);
SDL_Surface *setupSDLscreen(void);


// draw every shape in the link list connected to objroot
void drawshapes(SDL_Surface * screen, SDL_Surface * sdlbuff, cairo_t * cr,
		struct objnode *objroot, long simtime)
{
    struct objnode *objx = objroot, *player1, *player2;

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

#if SHOWFPS
    drawfps(cr, simtime);
#endif

    player1 = findplayer(objroot, P_ONE);
    player2 = findplayer(objroot, P_TWO);

    drawhpmeters(cr, HPSTART, player1->pinfo->hp, player2->pinfo->hp);

    drawtoscreen(screen, sdlbuff);

}

// draws a circle on the cairo surface
void drawcirc(cairo_t * cr, struct objnode *objx)
{
    cpFloat radius;
    cpVect pos, radiusv;
    struct colorset *colors;

    pos = cpBodyGetPos(objx->b);
    radius = cpCircleShapeGetRadius(objx->s);
    radiusv = cpvmult(cpvforangle(cpBodyGetAngle(objx->b)),
		      cpCircleShapeGetRadius(objx->s));
    colors = findcolors(objx->colortype, objx->ownedby);

    cairo_arc(cr, pos.x, pos.y, radius, 0, 2 * M_PI);

    cairo_set_source_rgba(cr, colors->c1.r, colors->c1.g,
			  colors->c1.b, colors->c1.a);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, colors->c2.r, colors->c2.g,
			  colors->c2.b, colors->c2.a);
    cairo_stroke(cr);

    cairo_move_to(cr, pos.x, pos.y);
    cairo_rel_line_to(cr, radiusv.x, radiusv.y);
    cairo_stroke(cr);

}

// draws a line segment
void drawlseg(cairo_t * cr, struct objnode *objx)
{
    cpVect enda, endb;
    struct colorset *colors;

    enda = cpSegmentShapeGetA(objx->s);
    endb = cpSegmentShapeGetB(objx->s);
    colors = findcolors(objx->colortype, objx->ownedby);

    cairo_move_to(cr, enda.x, enda.y);
    cairo_line_to(cr, endb.x, endb.y);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, colors->c1.r, colors->c1.g,
			  colors->c1.b, colors->c1.a);
    cairo_stroke(cr);

}

// draws a polygon
void drawpoly(cairo_t * cr, struct objnode *objx)
{
    int i, numv;
    cpVect pos, rotv, vert;
    struct colorset *colors;

    pos = cpBodyGetPos(objx->b);
    rotv = cpBodyGetRot(objx->b);
    numv = cpPolyShapeGetNumVerts(objx->s);
    colors = findcolors(objx->colortype, objx->ownedby);

    for (i = 0; i < numv; i++) {
	vert =
	    cpvadd(pos, cpvrotate(cpPolyShapeGetVert(objx->s, i), rotv));

	if (i == 0)
	    cairo_move_to(cr, vert.x, vert.y);
	else
	    cairo_line_to(cr, vert.x, vert.y);
    }

    cairo_close_path(cr);

    cairo_set_source_rgba(cr, colors->c1.r, colors->c1.g,
			  colors->c1.b, colors->c1.a);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, colors->c2.r, colors->c2.g,
			  colors->c2.b, colors->c2.a);
    cairo_stroke(cr);

}

// draws frames per second and simulation time on surface
void drawfps(cairo_t * cr, long simtime)
{
    int fps;
    double simrate;
    struct color_rgba color;

    framerate(simtime, &simrate, &fps);

    color.r = 1; color.b = 1;
    color.g = 1; color.a = 1;

    if (fps >= 0) {
	char s[100];

	sprintf(s, "Framerate: %3d fps", fps);
	drawstringxy(cr, s, &color, 3, YMAX - HPBUF - 6);

	sprintf(s, "Simulation rate: %1.2lfx", simrate);
	drawstringxy(cr, s, &color, 3, YMAX - HPBUF - 10);
    }

}

// displays text that states the winner
void showwinner(SDL_Surface * screen, SDL_Surface * sdlbuff, cairo_t * cr,
		struct objnode *objroot, long simtime)
{
    struct objnode *player1, *player2;
    int hp1, hp2;

    // draw the last frame first
    drawshapes(screen, sdlbuff, cr, objroot, simtime);

    player1 = findplayer(objroot, P_ONE);
    player2 = findplayer(objroot, P_TWO);

    hp1 = player1->pinfo->hp;
    hp2 = player2->pinfo->hp;

    if (hp1 <= 0 && hp2 <= 0)
	printf("the game was a tie!\n");
    else if (hp1 <= 0) {
	if (player2->pinfo->name != NULL)
	    showwinnertext(cr, player2->pinfo->name);
	else
	    showwinnertext(cr, "player 2");
    }
    else {
	if (player1->pinfo->name != NULL)
	    showwinnertext(cr, player1->pinfo->name);
	else
	    showwinnertext(cr, "player 1");
    }

    drawtoscreen(screen, sdlbuff);

    waitns(1e9 * 2);
}

// displays the winning player's name and that they won
void showwinnertext(cairo_t *cr, char *name)
{
    int slen;
    char *str;
    struct color_rgba color;

    color.r = 1; color.b = 1;
    color.g = 1; color.a = 1;

    slen = strlen(name) + strlen(" wins!");
    str = malloc(slen + 1);
    snprintf(str, slen + 1, "%s wins!", name);

    centermessage(cr, str, &color);

    free(str);
}

// displays centered text on the screen
void centermessage(cairo_t *cr, char *str, struct color_rgba *color)
{
    double width, height;
    cairo_text_extents_t extents;

    cairo_text_extents(cr, str, &extents);

    width = extents.width;
    height = extents.height;

    drawstringxy(cr, str, color, (XMAX - width) / 2, (YMAX - HPBUF - height) / 2);

}

// draws text at the specified x,y space coords. (not pixel coords)
void drawstringxy(cairo_t *cr, char *str, struct color_rgba *color,
		  double x, double y)
{
    cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);

    cairo_scale(cr, 1.0, -1.0);
    cairo_move_to(cr, x, -y);
    cairo_show_text(cr, str);
    cairo_scale(cr, 1.0, -1.0);
}

// draws both the hp meters
void drawhpmeters(cairo_t *cr, int hpmax, int hp1, int hp2)
{
    struct colorset *colors1, *colors2;

    colors1 = findcolors(COLOR_SHIP, P_ONE);
    colors2 = findcolors(COLOR_SHIP, P_TWO);

    drawhpmeter(cr, colors1, hp1 / (double) hpmax, -1);
    drawhpmeter(cr, colors2, hp2 / (double) hpmax, 1);

}

// draws a single hp meter
void drawhpmeter(cairo_t *cr, struct colorset *colors, double hpratio,
		 int direction)
{
    double xmid, xpos;

    if (hpratio < 0)
	hpratio = 0;

    xmid = ((double) XMAX) / 2;
    xpos = xmid + hpratio * (xmid - 2) * direction;

    cairo_move_to(cr, xmid, YMAX - 1);
    cairo_line_to(cr, xmid, YMAX - HPBUF);
    cairo_line_to(cr, xpos, YMAX - HPBUF);
    cairo_line_to(cr, xpos, YMAX - 1);

    cairo_close_path(cr);

    cairo_set_source_rgba(cr, colors->c1.r, colors->c1.g,
			  colors->c1.b, colors->c1.a);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, LPIXW / SCALEF);
    cairo_set_source_rgba(cr, colors->c2.r, colors->c2.g,
			  colors->c2.b, colors->c2.a);
    cairo_stroke(cr);

}

// blits and draws pixels to the screen
void drawtoscreen(SDL_Surface *screen, SDL_Surface *sdlbuff)
{
    SDL_BlitSurface(sdlbuff, NULL, screen, NULL);
    SDL_Flip(screen);
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
    cairo_translate(*cr, 0, -YMAX);	// shift image vertically
    cairo_set_font_size(*cr, FONTSIZE / (double) SCALEF);

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
