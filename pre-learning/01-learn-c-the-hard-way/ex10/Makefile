CC = gcc
TARGET = ex10

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET}

clean:
	rm -f ${TARGET}
