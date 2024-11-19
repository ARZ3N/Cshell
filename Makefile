
CC=gcc

seashell: main.c
	$(CC) main.c -o seashell

clean:
	rm seashell
