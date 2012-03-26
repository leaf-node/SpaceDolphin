NAME= spacedolphin

CC=gcc
CFLAGS=-O3 -Wall

INCL=-I$(IDIR) -Idl/chipmunk/include/chipmunk
LIBS=-lm -lSDL -lSDL_gfx
STATIC= dl/chipmunk/src/libchipmunk.a

IDIR =.
ODIR=obj

_DEPS = spacedolphin.h
_OBJ = main.o shape.o draw.o move.o time.o

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



.PHONY: all

all: $(NAME)

$(NAME): $(ODIR) $(OBJ)
	gcc -g $(CFLAGS) $(LIBS) $(OBJ) $(STATIC) -o $@

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS) $(STATIC)
	$(CC) -g -c $(CFLAGS) $(INCL) -o $@ $<



dl/ChipmunkLatest.tgz:
	@echo "\n\n Downloading Chipmunk Physics... \n\n"
	mkdir -p dl
	curl -# http://chipmunk-physics.net/release/ChipmunkLatest.tgz > dl/ChipmunkLatest.tgz

dl/chipmunk: dl/ChipmunkLatest.tgz
	tar xzf dl/ChipmunkLatest.tgz
	mv `tar tzf dl/ChipmunkLatest.tgz | head -1` dl/chipmunk

$(STATIC): dl/chipmunk
	cd dl/chipmunk && cmake .
	make -C dl/chipmunk chipmunk_static



.PHONY: strip clean unstrip

strip: all
	strip $(NAME)

unstrip: all
	rm $(NAME)
	make all

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
	rmdir -p $(ODIR)
	rm -rf dl/chipmunk
	rm -f $(NAME)

