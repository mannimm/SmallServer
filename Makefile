all: mysiserver smallSet smallGet
mysiserver_server: mysiserver.c siserver.c
	gcc -Wall mysiserver.c siserver.c -o mysiserver


smallGet: smallGet.c siserver.c
	gcc -Wall smallGet.c siserver.c -o smallGet

smallSet: smallSet.c siserver.c
	gcc -Wall smallSet.c siserver.c -o smallSet

clean: -rm -f *.o