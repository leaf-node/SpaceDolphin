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
    static long frmtime = -1;
    static double simsum = -1, frmsum = -1;
    static long simtimes[NITER], frmtimes[NITER];
    struct timespec now;
    static struct timespec markt;

    curtime(&now);
    if (markt.tv_sec == 0 && markt.tv_nsec == 0)
	markt = now;
    else {
	frmtime = convtns(tdiff(now, markt));
	markt = now;

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
    *fps = (frmsum == -1) ? -1 : round((double) 1e9 * NITER / frmsum);

}

// return capped length of time for the last cycle through main()'s loop
// (this value is used to tell the physics engine how much time to simulate.)
// also slow things down to MAXFPS; leave at least MINIDLEP% cpu idle
// TODO: avoid overflow with multi-second time deltas on 32 bit systems.
long timebal(void)
{
    long calctime, waitt, truewaitt, minidle, totalt;
    static long waitdiff;
    struct timespec now;
    static struct timespec markt, marktbeforeidle;


    curtime(&now);
    if (markt.tv_sec == 0 && markt.tv_nsec == 0)
	markt = marktbeforeidle = now;

    calctime = convtns(tdiff(now, markt));
    marktbeforeidle = now;

    waitt = MINFT - calctime - waitdiff;
    // free a minimum of MINIDLEP% cpu
    minidle = calctime * MINIDLEP / 100;
    waitt = (waitt < minidle) ? minidle : waitt;

    waitns(waitt);
    curtime(&markt);

    truewaitt = convtns(tdiff(markt, marktbeforeidle));
    if (waitdiff == 0)
	waitdiff = truewaitt - waitt;
    else
	waitdiff = (9 * waitdiff) / 10 + (truewaitt - waitt) / 10;

    totalt = calctime + truewaitt;
    // avoid positive feedback loop causing scheduled simulation time to
    // escalate on slower machines
    totalt = (totalt > MAXFT) ? MAXFT : totalt;

    return totalt;
}

// sleep for ns nanoseconds
void waitns(long ns)
{
    struct timespec t;

    if (ns <= 0)
	return;
    convttp(ns, &t);
    if (nanosleep(&t, NULL) != 0)
	printf("*** nanosleep error ***\n");
}

// convert nanoseconds into a struct used by nanosleep
void convttp(long ns, struct timespec *tp)
{
    tp->tv_sec  = ns / (long) 1e9;
    tp->tv_nsec = ns % (long) 1e9;
}

// convert timespec struct into nanoseconds. beware of overflow on 32 bit
// machines.
long convtns(struct timespec tp)
{
    return tp.tv_sec * 1e9 + tp.tv_nsec;
}

// get current monotinic time in nanoseconds
void curtime(struct timespec *tp)
{
    clock_gettime(CLOCK_MONOTONIC, tp);
}

// returns the delta between two times
struct timespec tdiff(struct timespec tp0, struct timespec tp1)
{
    struct timespec dtp;

    dtp.tv_sec = tp0.tv_sec - tp1.tv_sec;

    if (tp0.tv_nsec >= tp1.tv_nsec)
	dtp.tv_nsec = tp0.tv_nsec - tp1.tv_nsec;
    else {
	dtp.tv_sec--;
	dtp.tv_nsec = 1e9 + tp0.tv_nsec - tp1.tv_nsec;
    }

    return dtp;

}

