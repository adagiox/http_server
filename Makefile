build:
	@gcc -I./includes/ src/server.c src/files.c src/util.c

run: build
	@./a.out