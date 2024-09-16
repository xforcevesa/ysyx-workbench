CC = gcc
TARGET = ex13

${TARGET}:
	$(CC) -o ${TARGET} ${TARGET}.c -g
	valgrind ./${TARGET} Zed

clean:
	rm -f ${TARGET}
