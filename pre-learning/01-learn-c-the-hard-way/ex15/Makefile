CC = gcc
TARGET = ex15

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET} Zd

clean:
	rm -f ${TARGET}
