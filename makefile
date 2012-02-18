NAME= spacedolphin

CC=gcc
CFLAGS=-O3 -Wall

INCL=-I$(IDIR) -I/usr/local/include/chipmunk
LIBS=-lm -lchipmunk -lSDL -lSDL_gfx
STATIC=

IDIR =.
ODIR=obj

_DEPS = spacedolphin.h
_OBJ = main.o shape.o draw.o move.o time.o

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all

all: $(NAME)

$(NAME): $(ODIR) $(OBJ)
	gcc -g $(CFLAGS) $(LIBS) $(STATIC) $(OBJ) -o $@

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -g -c $(CFLAGS) $(INCL) -o $@ $<

.PHONY: strip clean unstrip

strip: all
	strip $(NAME)

unstrip: all
	rm $(NAME)
	make all

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
	rmdir -p $(ODIR)
	rm -f $(NAME)

