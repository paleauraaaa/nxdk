ifeq (CC,)
	CC=/usr/bin/cc
endif

all: vshc

vshc: main.c
	$(CC) -std=c99 main.c -o vshc

clean:
	rm -f main.o vshc
