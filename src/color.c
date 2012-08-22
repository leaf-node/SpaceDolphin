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

#include <spacedolphin.h>


struct entrylist {
    ENTRY *entry;
    struct entrylist *next;
    struct entrylist *prev;
};


struct colorset *makecolorset(double c1r, double c1g, double c1b,
			      double c1a, double c2r, double c2g,
			      double c2b, double c2a);
void savecs(struct colorset *colors, int colortype, int ownedby);
char *makekey(int x, int y);


// create the hashtable that holds color lookup info.
// also set the colors for various objects used in the game.
void initcolors(void)
{
    if (hcreate(HSIZE) == 0) {
	fprintf(stderr, "*** error: failed to create hash table with "
		"max elems: %d\n", HSIZE);
	exit(6);
    }
    // values:          r,g,b,a,            r,g,b,a
    // the first set of colors is for the fill, the second for the border.
    savecs(makecolorset(0,0,0,0,	    1,1,1,1), COLOR_SHIP, P_NONE);
    savecs(makecolorset(0.5,0,1,1,	    1,0,0,1), COLOR_SHIP, P_ONE);
    savecs(makecolorset(1,0.5,0,1,	    1,1,1,1), COLOR_SHIP, P_TWO);

    savecs(makecolorset(0.5,0.5,0.5,1,	    1,1,1,1), COLOR_LARGE, P_NONE);
    savecs(makecolorset(0.5,0,1,1,	    1,1,1,1), COLOR_LARGE, P_ONE);
    savecs(makecolorset(1,0.5,0,1,	    1,1,1,1), COLOR_LARGE, P_TWO);

    savecs(makecolorset(0,0,0,1,	    1,0,0,1), COLOR_SMALL, P_NONE);
    savecs(makecolorset(0,0,1,1,	    1,1,1,1), COLOR_SMALL, P_ONE);
    savecs(makecolorset(1,0,0,1,	    1,1,0,1), COLOR_SMALL, P_TWO);

    savecs(makecolorset(0.10,0.10,0.10,1,
			0.75,0.75,0.75,1), COLOR_BHOLE, P_NONE);
    savecs(makecolorset(1,0,0,1,	    1,1,1,1), COLOR_LINE, P_NONE);


}

// returns pointer to struct that stores color info (fill and border)
struct colorset *makecolorset(double c1r, double c1g, double c1b,
			      double c1a, double c2r, double c2g,
			      double c2b, double c2a)
{
    struct colorset *colors;

    colors = malloc(sizeof(struct colorset));

    colors->c1.r = c1r;
    colors->c1.g = c1g;
    colors->c1.b = c1b;
    colors->c1.a = c1a;

    colors->c2.r = c2r;
    colors->c2.g = c2g;
    colors->c2.b = c2b;
    colors->c2.a = c2a;

    return colors;
}


struct entrylist entryroot = { NULL, NULL, NULL };

struct entrylist *entrylast = &entryroot;

// enters color struct into hashtable, with a key based on the functional type,
// and the owner. (the owner changes when a player touches an object, etc.)
void savecs(struct colorset *colors, int colortype, int ownedby)
{
    ENTRY entry, *res;
    char *keystr;
    struct entrylist *currentry;

    keystr = makekey(colortype, ownedby);

    entry.key = keystr;
    entry.data = (void *) colors;

    if ((res = hsearch(entry, ENTER)) == NULL) {
	fprintf(stderr,
		"*** error: failed to insert hash table element.\n");
	exit(6);
    }

    // keep a list of entries to free later on.
    currentry = malloc(sizeof(struct entrylist));

    entrylast->next = currentry;
    currentry->prev = entrylast;
    currentry->next = NULL;

    currentry->entry = res;

    entrylast = currentry;

}

// free the keys/values associated with entries in the hash table
void freecolorentries(void)
{
    struct entrylist *entryx, *entrynext;

    for (entryx = entryroot.next; entryx != NULL; entryx = entrynext) {
	free(entryx->entry->key);
	free(entryx->entry->data);
	entrynext = entryx->next;
	free(entryx);
    }

    entrylast = &entryroot;
}

// create a hash key to be used with color data entry and lookup.
char *makekey(int x, int y)
{
    char *buff;
    int len;
    int err;

    // if the max values need to be increased, then the len and format
    // strings need to be increased as well.
    if (x >= 16 * 16 || y >= 16 * 16) {
	fprintf(stderr, "*** error: ints in key must be less than %d\n",
		16 * 16);
	exit(6);
    }

    len = strlen("color:00:00");
    buff = malloc(len + 1);

    err = snprintf(buff, len + 1, "color:%2x:%2x", x, y);

    if (err < 0 || err >= len + 1) {
	fprintf(stderr, "*** error: snprintf gives this error code: %d\n",
		err);
	exit(6);
    }

    return buff;
}

// finds the appropriate color pair based on the type of the object and the
// "owner".
struct colorset *findcolors(int colortype, int ownedby)
{
    ENTRY entry, *res;
    char *keystr;

    keystr = makekey(colortype, ownedby);

    entry.key = keystr;

    if ((res = hsearch(entry, FIND)) == NULL) {
	fprintf(stderr, "*** error: failed to find element: %s.\n",
		keystr);
	exit(6);
    }

    free(keystr);

    return (struct colorset *) res->data;
}
