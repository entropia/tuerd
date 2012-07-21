SOBJS=init_patchlevel.so

%.so: %.c
	gcc -Wall -g -I../../../libmf/include/ -I../ -pthread -I/usr/include/PCSC -std=gnu99 $< -c -o $@

all: $(SOBJS)

clean:
	rm *.so
