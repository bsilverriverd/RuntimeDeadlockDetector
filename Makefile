
all: ddmon.c ddchck.c
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -o ddchck ddchck.c -pthread

debug: ddmon.c ddchck.c
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread -DDEBUG
	gcc -o ddchck ddchck.c -pthread -DDEBUG

clean:
	rm ddmon.so ddchck
