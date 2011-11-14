OBJS=curl.o\
	desfire.o\
	main.o\
	pcsc.o

%.o: %.c
	gcc -I../../libmf/include/ -pthread -I/usr/include/PCSC -std=gnu99 $< -c -o $@

all: $(OBJS)
	gcc -lcurl -lpcsclite -L../../libmf/build/src/ -lmf -o tuerd $(OBJS)
