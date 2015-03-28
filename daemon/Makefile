OBJS=door.o\
	main.o\
	git.o\
	rfid.o

GIT_REV=$(shell git rev-parse HEAD)

%.o: %.c
	gcc -Wall -Wextra -Werror -DGIT_REV=\"$(GIT_REV)\" -g `pkg-config --cflags libgit2 jansson` -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -g -lnfc -lfreefare `pkg-config --libs libgit2 jansson` -o tuerd $(OBJS)

clean:
	rm -f *.o tuerd
