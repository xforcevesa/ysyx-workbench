CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic
SRC = main.c
EXE = main

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:	
	rm -f $(EXE)

run: $(EXE)
	./$(EXE)

.PHONY: clean all run

