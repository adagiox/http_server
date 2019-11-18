build:
	@gcc -I./includes/ src/server.c

run: build
	@./a.out