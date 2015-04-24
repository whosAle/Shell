all:	shell

shell: shell.c shell.h
	gccx -o shell shell.c
clean:
	rm -f shell *.o
