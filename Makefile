
CC=gcc

shell: main.c
	$(CC) main.c -o shell

clean:
	rm shell
