OBJS=door.o\
	desfire.o\
	main.o\
	pcsc.o\
	git.o\
	powercycle.o

GIT_REV=$(shell git rev-parse HEAD)

%.o: %.c
	gcc -Wall -DGIT_REV=\"$(GIT_REV)\" -g -I../../libmf/include/ -pthread -I/usr/include/PCSC `pkg-config --cflags libgit2 jansson` -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -g -lpcsclite -L../../libmf/build/src/ -lmf `pkg-config --libs libgit2 jansson` -o tuerd $(OBJS)

clean:
	rm -f *.o tuerd
