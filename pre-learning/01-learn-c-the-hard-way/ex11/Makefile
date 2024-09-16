CC = gcc
TARGET = ex11

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET}

clean:
	rm -f ${TARGET}
