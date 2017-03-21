CFLAGS := -std=gnu99 # -Wall
LIB :=
INC := -I include

all: mkdir server client

mkdir:
	mkdir -p build out

server: build/server.o build/number_game.o build/stack.o build/ipc_utils.o
	@echo " Linking..."
	gcc $^ -o out/server $(LIB)

client: build/client.o build/number_game.o build/stack.o build/ipc_utils.o
	@echo " Linking..."
	gcc $^ -o out/client $(LIB)

build/server.o : server.c
	gcc $(CFLAGS) $(INC) -c -o $@ $<

build/client.o : client.c
	gcc $(CFLAGS) $(INC) -c -o $@ $<

build/ipc_utils.o : src/ipc_utils.c
	gcc $(CFLAGS) $(INC) -c -o $@ $<

build/number_game.o : src/number_game.c
	gcc $(CFLAGS) $(INC) -c -o $@ $<

build/stack.o : src/stack.c
	gcc $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " cleaning..."
	rm -r build client server