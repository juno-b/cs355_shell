DO NOT run this on goldengate. There are unresolved bugs which frequently crash the terminal. See known issues for more.

Features that work as intended:
standard (non-backgrounded) commands
Ctrl+c, ctrl+d, ctrl+z, etc. do not crash the main shell
Commands are added to the background list with appropriate fields
jobs prints the active jobs
kill -9 terminates the given process

Partially working:
fg: waiting for PID to finish infinitely, if you enter exit it says it gives control back but the shell dies with infinite mysh: printed
kill (memory leaks)
Tokenizer handles ; but no testing on it, I don't know what it's used for

Not implemented:
termios: attempted but probably wrong
bg: I'm not sure how this is different from fg, I haven't done anything with resuming processes
Extra credit (pipes, redirections) tokenizer should handle |, <, > but haven't implemented anything with it

Bonus features:
history: lists the jobs in history
cd: changes the working directory

Known issues:
execute_command sometimes exits with two processes running after backgrounding, which creates many issues down the line
This includes times where it just prints mysh: infinitely
kill has memory leaks, fixing above may resolve?
On exit, "mysh: " is printed for each existing job. after that it looks like it goes back to normal but if you try to type it starts printing mysh: infinitely until it crashes
the first command entered will not always work (seg fault).
valgrind reports issues with uninitialized memory (this does not crash the shell)
random .nsf files are created: find the processes with lsof | grep .nfs (tab complete the file name)