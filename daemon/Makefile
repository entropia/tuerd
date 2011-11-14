OBJS=curl.o\
	desfire.o\
	main.o\
	pcsc.o

%.o: %.c
	gcc -g -I../../libmf/include/ -pthread -I/usr/include/PCSC -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -g -lcurl -lpcsclite -L../../libmf/build/src/ -lmf -o tuerd $(OBJS)

clean:
	rm *.o tuerd
