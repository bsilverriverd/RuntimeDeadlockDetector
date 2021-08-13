all: ddmon.c ddchck.c ddmon_chck.c ddgraph.c
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -o ddchck ddchck.c ddgraph.c -pthread
	gcc -g -shared -fPIC -o ddmon_chck.so ddmon_chck.c ddgraph.c -ldl -pthread
	gcc -o ddpred ddpred.c ddgraph.c -pthread

debug: ddmon.c ddchck.c
	gcc -g -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread -DDEBUG
	gcc -o ddchck ddchck.c ddgraph.c -pthread -DDEBUG

clean:
	rm ddmon.so ddmon_chck.so ddchck ddpred
