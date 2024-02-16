all: shell

shell: shell.c List.o Job.o
	gcc -o shell shell.c List.o Job.o -lreadline -lhistory

List: List.c List.h
	gcc -c List.c

Job: Job.c Job.h
	gcc -c Job.c

clean:
	rm -f shell *.o