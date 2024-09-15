CC = gcc
TARGET = ex33

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c list.c list_algo.c -g
	valgrind ./${TARGET} Zed c

clean:
	rm -f ${TARGET}
