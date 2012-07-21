OBJS=curl.o\
	desfire.o\
	main.o\
	pcsc.o

GIT_REV=$(shell git rev-parse HEAD)

%.o: %.c
	gcc -Wall -DGIT_REV=\"$(GIT_REV)\" -g -I../../libmf/include/ -pthread -I/usr/include/PCSC -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -g -lcurl -lpcsclite -L../../libmf/build/src/ -lmf -o tuerd $(OBJS)

clean:
	rm -f *.o tuerd
