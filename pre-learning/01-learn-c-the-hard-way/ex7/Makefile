CC = gcc
TARGET = ex7

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET}

clean:
	rm -f ${TARGET}
