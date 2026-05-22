all: myshell mypipe
myshell: myshell.o LineParser.o
	gcc -m32 -o myshell myshell.o LineParser.o
myshell.o: myshell.c
	gcc -m32 -Wall -g -c -o myshell.o myshell.c
LineParser.o: LineParser.c
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c
mypipe: mypipe.o
	gcc -m32  -Wall -g -o mypipe mypipe.o
mypipe.o: mypipe.c
	gcc -m32 -Wall -g -c -o mypipe.o mypipe.c 

clean:
	rm -f myshell mypipe *.o