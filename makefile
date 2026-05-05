all: myshell mypipe
myshell: myshell.c LineParser.c
	cc -m32 -o myshell myshell.c LineParser.c
mypipe: mypipe.c
	cc -m32 -o mypipe mypipe.c
clean:
	rm -f myshell mypipe