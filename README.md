SpaceDolphin
============

![sample screenshot](http://i.imgur.com/g0oow.png)


This is a simple little toy game released under
**[GPLv3+](http://www.gnu.org/licenses/quick-guide-gplv3.html)**. It uses the
[Chipmunk Physics](http://chipmunk-physics.net/) engine, which is under a
permissive license.


### Build Instructions:

**SDL**, **SDL\_gfx**, **curl**, and **cmake** need to be installed to build
and run this game. The following command will install these packages for you in
an apt-based packaging system, used by many distros of
[GNU/Linux](http://www.gnu.org/): (Pacakge names may be different in
newer/older systems.)

    sudo apt-get install build-essential curl cmake libsdl1.2debian libsdl1.2-dev libsdl-gfx1.2-4 libsdl-gfx1.2-dev

To build the game, run:

    make

It should automatically download and build the Chipmunk Physics library, so you
no longer have to go out of your way to install it manually. Of course, you'll
need an internet connection (or to copy the
"[ChipmunkLatest.tgz](http://chipmunk-physics.net/release/ChipmunkLatest.tgz)"
file into the "dl" subdirectory) to proceed.


### Game Instructions

To play the game, run this:

    ./spacedolphin

Control your flight with the arrow keys so that you can knock objects around
the screen. Go nuts with it.


