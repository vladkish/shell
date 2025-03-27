all:
	make build
	make run

build:
	gcc -Wall -Wextra -g src/main.c src/parser.c src/cmd-executor.c -o bin/main
run:
	./bin/main
