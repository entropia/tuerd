OBJS=curl.o\
	desfire.o\
	main.o\
	pcsc.o

%.o: %.c
	gcc -I../../libmf/include/ $< -c -o $@

all: $(OBJS)
	gcc -lcurl -o tuerd $(OBJS)
