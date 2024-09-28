CC = gcc
TARGET = ex12

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET}

clean:
	rm -f ${TARGET}
