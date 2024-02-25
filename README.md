DO NOT run this on goldengate. There are unresolved bugs which occasionally crash the terminal. See known issues for more.

Features that work as intended:
standard (non-backgrounded) commands
Ctrl+c, ctrl+z, etc. do not crash the main shell
Commands are added to the background list with appropriate fields
jobs prints the active jobs
kill -9 terminates the given process

Partially working:
kill (memory leaks)

Not implemented:
termios: attempted but probably wrong
bg: I'm not sure how this is different from fg, I haven't done anything with resuming processes

Bonus features:
history: lists the jobs in history
cd: changes the working directory

Known issues:
execute_command sometimes exits with two processes running after backgrounding, which creates many issues down the line
This includes times where it just prints mysh: infinitely
kill has memory leaks, fixing above may resolve?
the first command entered will not always work (seg fault).
valgrind reports issues with uninitialized memory (this does not crash the shell)
random .nsf files are created: find the processes with lsof | grep .nfs (tab complete the file name)