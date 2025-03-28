all:
	make build
	make run

build:
	gcc -Wall -Wextra -g src/main.c src/parser.c src/commands.c src/features/bg_jobs.c src/features/history.c src/utils.c -o bin/main
run:
	./bin/main
