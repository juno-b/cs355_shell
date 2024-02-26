all: shell shells

shells: shell_semicolon.c List.o Job.o Tokenizer.o
	gcc -g -o shells shell_semicolon.c List.o Job.o Tokenizer.o -lreadline -lhistory

shell: shell.c List.o Job.o Tokenizer.o
	gcc -g -o shell shell.c List.o Job.o Tokenizer.o -lreadline -lhistory

List: List.c List.h
	gcc -c List.c

Job: Job.c Job.h
	gcc -c Job.c

Tokenizer: Tokenizer.c Tokenizer.h
	gcc -c Tokenizer.c

clean:
	rm -f shell *.o
