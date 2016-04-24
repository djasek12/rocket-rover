all: 	main

main:
	gcc -c gfx5.c
	gcc -o final final.c gfx5.o -lX11 -lm

clean:
	rm -f final *.o
