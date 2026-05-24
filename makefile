all: myshell mypipeline
myshell: myshell.o LineParser.o
	gcc -m32 -o myshell myshell.o LineParser.o
myshell.o: myshell.c
	gcc -m32 -Wall -g -c -o myshell.o myshell.c
LineParser.o: LineParser.c
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c
mypipeline: mypipeline.o
	gcc -m32  -Wall -g -o mypipeline mypipeline.o
mypipe.o: mypipeline.c
	gcc -m32 -Wall -g -c -o mypipeline.o mypipeline.c 

clean:
	rm -f myshell mypipe *.o