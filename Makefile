all: ddmon.c ddchck.c cigarette_smokers.c
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -o ddchck ddchck.c -pthread
	gcc -o test cigarette_smokers.c -pthread
clean:
	rm *.so ddchck test
