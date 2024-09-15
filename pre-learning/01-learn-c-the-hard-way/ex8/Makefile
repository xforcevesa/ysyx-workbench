CC = gcc
TARGET = ex8

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET}

clean:
	rm -f ${TARGET}
