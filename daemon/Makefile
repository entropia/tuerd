OBJS=door.o\
	desfire.o\
	main.o\
	git.o

GIT_REV=$(shell git rev-parse HEAD)

%.o: %.c
	gcc -Wall -DGIT_REV=\"$(GIT_REV)\" -g `pkg-config --cflags libgit2 jansson` -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -g -lnfc `pkg-config --libs libgit2 jansson` -o tuerd $(OBJS)

clean:
	rm -f *.o tuerd
