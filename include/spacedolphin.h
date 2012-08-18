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

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL/SDL.h>
#include <cairo/cairo.h>
#include <chipmunk.h>

enum player { P_NONE, P_ONE, P_TWO, P_THREE, P_FOUR };
enum shape { S_NONE, S_LSEG, S_CIRC, S_POLY };
enum collide { C_NONE, C_SHIP, C_COLOR };

struct color_rgba {
    float r, g, b, a;
};

// struct of force and torque vectors
struct forces {
    cpVect force;
    cpVect tforce;
};

// remembers which key presses/forces are active for a player
struct movement {
    struct forces prevf;
    struct timespec markt;
    bool up;
    bool down;
    bool left;
    bool right;
    bool cw;
    cpFloat cwt;
    bool ccw;
    cpFloat ccwt;
};

// linked list node to keep track of objects for drawing and querying
struct objnode {
    int player;
    struct movement *pmove;
    int geom;
    int bhole;
    cpBody *b;
    cpShape *s;
    cpFloat width;
    struct color_rgba c1;
    struct color_rgba c2;
    int cstatus;
    struct timespec lasthit;
    struct objnode *prev;
    struct objnode *next;
};

#define DEBUG	    false
#define SHOWFPS	    true	// show the fps if true
#define FULLSCREEN  false	// is fullscreen mode the default?

#define DURATION INFINITY	// max length of game in nanoseconds

#define DT       5e5		// 5e5 nanoseconds: physics engine time step size
#define MAXFPS   60		// max frames per second
#define MINFT    ((long) 1e9 / MAXFPS)	// min frame time
#define MINFPS   20		// min frames per second
#define MAXFT    ((long) 1e9 / MINFPS)	// max frame time
#define MINIDLEP 5		// minimum % of cpu to leave idle
#define NITER    MAXFPS		// n of frames to average, to calc actual fps

#define FORCE       200.0	// force of rocket's jetpack
#define TFORCE      200.0	// proportional to torque of rocket
#define RLEN          4.0	// length of radius at which TFORCE is applied
#define MAXVEL	    200.0	// soft limit for velocity
#define MAXANGVEL     8.0	// soft limit for angular velocity
#define TORQRAMPT     1e9	// ns over which torque ramps up while turning

//#define VGRAV     -50.0       // upwards gravity (so negative means down)
#define VGRAV	      0.0	// upwards gravity (so negative means down)
#define BGRAV     64000.0	// gravity towards any black holes
#define RDSQ	     10.0	// squared distance from BH to apply rep. force
#define REPFS	    400.0	// scaled strength of replellant force

#define XMAX        160.0
#define XMIN          0.0
#define YMAX        120.0
#define YMIN          0.0
#define XYBUF        20.0

#define SCALEF	      4.0
#define LPIXW	      1.0

#define PI   3.1415926535

// draw.c
void graphicsinit(SDL_Surface ** screen, SDL_Surface ** sdlbuff,
		  cairo_surface_t ** surface, cairo_t ** cr);
long drawshapes(SDL_Surface * screen, SDL_Surface * sdlbuff, cairo_t * cr,
		struct objnode *objroot);
SDL_Surface *togglefullscreen(void);

// move.c
void interact(cpSpace * space, struct objnode *objroot,
	      SDL_Surface ** screen);
void dontfall(cpBody * body, cpVect gravity, cpFloat damping, cpFloat dt);
void orbit(cpBody * body, cpVect gravity, cpFloat damping, cpFloat dt);

// time.c
void framerate(long simtime, double *simrate, int *fps);
long timebal(void);
void waitns(long ns);
void convttp(long ns, struct timespec *tp);
long convtns(struct timespec tp);
void curtime(struct timespec *tp);
struct timespec tdiff(struct timespec tp0, struct timespec tp1);

// shape.c
cpSpace *makeshapes(struct objnode *objx);
struct color_rgba setcolor(float r, float g, float b, float a);
void rmobj(struct objnode *objx);
void rmobjs(struct objnode *objroot);

// collide.c
int chcolor (cpArbiter *arb, cpSpace *space, void *data);

