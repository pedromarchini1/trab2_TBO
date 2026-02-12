CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC_DIR = src
OBJ_DIR = src

OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/btree.o $(OBJ_DIR)/btree_node.o $(OBJ_DIR)/queue.o

EXEC = trab2

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC) btree.bin