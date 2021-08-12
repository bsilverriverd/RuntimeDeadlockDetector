all: ddmon.c ddchck.c
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -o ddchck ddchck.c -pthread

test : test2.c test3.c test4.c test5.c test6.c
	gcc -g -o test2 test2.c -pthread
	gcc -g -o test3 test3.c -pthread
	gcc -g -o test4 test4.c -pthread
	gcc -g -o test5 test5.c -pthread
	gcc -g -o test6 test6.c -pthread

clean:
	rm ddmon.so ddchck test2 test3 test4 test5 test6
