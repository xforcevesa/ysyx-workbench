CC = gcc
TARGET = ex18

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET} Zed c

clean:
	rm -f ${TARGET}
