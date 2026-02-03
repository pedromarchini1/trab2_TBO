CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = src/main.c src/btree.c src/btree_node.c src/queue.c
OBJ = $(SRC:.c=.o)

trab2: $(OBJ)
	$(CC) $(CFLAGS) -o trab2 $(OBJ)

clean:
	rm -f src/*.o trab2 btree.bin
