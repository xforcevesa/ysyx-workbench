CC = gcc
TARGET = ex32

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c list.c -g
	valgrind ./${TARGET} Zed c

clean:
	rm -f ${TARGET}
