NAME= spacedolphin

CC=gcc
CFLAGS=-Wall

INCL=-I$(IDIR) -I/usr/local/include/chipmunk
LIBS=-lm -lchipmunk -lSDL -lSDL_gfx
STATIC=

IDIR =.
ODIR=obj

_DEPS = spacedolphin.h
_OBJ = main.o shape.o draw.o move.o time.o

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all debug

all: $(ODIR) $(NAME) 

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -g -c $(CFLAGS) $(INCL) -o $@ $<

$(NAME): $(OBJ)
	gcc -g $(CFLAGS) $(LIBS) $(STATIC) $^ -o $@

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

