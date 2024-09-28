CC = gcc
TARGET = ex14

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET} Zd

clean:
	rm -f ${TARGET}
