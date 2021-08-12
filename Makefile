all: ddmon.c ddchck.c
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -o ddchck ddchck.c -pthread

test : test2.c test3.c
	gcc -o test2 test2.c -pthread
	gcc -o test3 test3.c -pthread

clean:
	rm ddmon.so ddchck test2 test3
