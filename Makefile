all: mysiserver smallSet smallGet siserver
mysiserver: mysiserver.c siserver.c
	gcc -Wall mysiserver.c siserver.c -o smalld


smallGet: smallGet.c siserver.c
	gcc -Wall smallGet.c siserver.c -o smallGet

smallSet: smallSet.c siserver.c
	gcc -Wall smallSet.c siserver.c -o smallSet

clean: -rm -f *.o smallSet smallGet smalld