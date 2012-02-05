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

// calculates the avg. fps and simulation speed once every NITER iterations
// returns a simrate and fps of -1 for the first NITER+1 iterations
void framerate(long simtime, double *simrate, int *fps)
{
    int j;
    static int i = 0;
    static long markt = 0, frmtime = -1;
    static long simsum = -1, frmsum = -1;
    static long simtimes[NITER], frmtimes[NITER];

    if (markt == 0)
	markt = curns();
    else {
	frmtime = curns() - markt;
	markt = curns();

	simtimes[i] = simtime;
	frmtimes[i++] = frmtime;

	if (i == NITER) {
	    simsum = frmsum = 0;
	    for (j = 0; j < NITER; j++) {
		simsum += simtimes[j];
		frmsum += frmtimes[j];
	    }
	    i = 0;
	}
    }

    *simrate = (simsum == -1) ? -1 : (double) simsum / frmsum;
    *fps = (frmsum == -1) ? -1 : (long) 1e9 *NITER / frmsum;

}

// return capped length of time for the last cycle through main()'s loop
// (this value is used to tell the physics engine how much time to simulate.)
// also slow things down to MAXFPS; leave at least 3% cpu idle
long timebal(long *markt)
{
    long waitt, simtime, minidle;

    simtime = curns() - *markt;
    *markt = curns();

    // avoid positive feedback loop causing scheduled simulation time to
    // escalate on slower machines
    simtime = (simtime > MINFT) ? MINFT : simtime;

    waitt = MAXFT - simtime / 2;	// aim for the desired fps or less
    minidle = simtime * MINIDLEP / 100;	// free a minimum of MINIDLEP% cpu
    waitt = (waitt <= minidle) ? minidle : waitt;
    waitns(waitt);

    return simtime;
}

// sleep for ns nanoseconds
void waitns(long ns)
{
    struct timespec t;
    if (ns <= 0)
	return;
    convtns(ns, &t);
    if (nanosleep(&t, NULL) != 0)
	printf("nanosleep error ***");
}

// convert nanoseconds into a struct used by nanosleep
void convtns(long ns, struct timespec *tp)
{
    tp->tv_sec = ns / (long) 1e9;
    tp->tv_nsec = ns % (long) 1e9;
}

// get current monotinic time in nanoseconds
long curns(void)
{
    struct timespec tp[1];

    clock_gettime(CLOCK_MONOTONIC, tp);
    return tp->tv_nsec + (long) 1e9 *tp->tv_sec;
}
