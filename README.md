Features that work as intended:
standard (non-backgrounded) commands
Ctrl+c, ctrl+z, etc. do not crash the main shell
Commands are added to the background list
jobs prints the active jobs
kill -9 terminates the given process

Partially working:
kill

Not implemented:
bg: I'm not sure how this is different from fg, I haven't done anything with resuming processes

Bonus features:
history: lists the jobs in history
cd: changes the working directory

Known issues:
execute_command sometimes exits with two processes running after backgrounding, which creates many issues down the line
the first command entered will not always work (seg fault).
valgrind reports issues with uninitialized memory (this does not crash the shell)
kill %n without the -9 flag causes a seg fault
random .nsf files are created: find the processes with lsof | grep .nfs (tab complete the file name)