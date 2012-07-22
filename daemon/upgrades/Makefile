SOBJS=init_patchlevel.so\
      make_ndef.so

%.so: %.c
	gcc -Wall -g -I../../../libmf/include/ -I../ -pthread -I/usr/include/PCSC -std=gnu99 $< -shared -fPIC -o $@

all: $(SOBJS)

clean:
	rm -f *.so
