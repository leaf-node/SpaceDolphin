SpaceDolphin
============

![sample screenshot](http://i.imgur.com/g0oow.png)


This is a simple little toy game released under
**[GPLv3+](http://www.gnu.org/licenses/quick-guide-gplv3.html)**. It uses the
[Chipmunk Physics](http://chipmunk-physics.net/) engine, which is under a
permissive license.


### Build Instructions:

**SDL**, **SDL\_gfx**, and **CMake** need to be installed to build and run this
game. The following command will install these packages for you in an apt-based
packaging system, used by many distros of [GNU/Linux](http://www.gnu.org/):
(Pacakge names may be different in newer or older systems.)

    sudo apt-get install build-essential cmake libsdl1.2-dev libsdl-gfx1.2-dev

To build the game in the SpaceDolphin directory, run:

    mkdir build ; cd build
    cmake ..
    make

CMake will automatically build and statically link the included Chipmunk
Physics library, without needing to install it.


### Game Instructions

To play the game, run this:

    ./spacedolphin

Control your flight with the arrow keys so that you can knock objects around
the screen. Go nuts with it! :)


