# SPU Falsh, version 0.0.1-alpha

Usage: `falsh [-h]`

`-h`: Show the usage information and exit

These shell commands are defined internally. Type `help` to see this list.

help - display the "help" dialog
exit - exit `falsh` shell
pwd - print the current working directory
cd [dir] - change directory to the given directory, or `$HOME` if not specified
setpath <dir> [dir ...] - set `$PATH` equal to the directory or directories given

Redirection: Standard and error output can be redirected by adding a `>` and a filename after your command. Standard output will be directed to `filename.out` and errors to `filename.err`.

Example: `pwd > working_directory`