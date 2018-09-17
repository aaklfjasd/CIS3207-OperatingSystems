# Lab 2

myshell manual
<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
Description:
myshell is a Unix shell program that serves as a command line interpreter that will execute commands inputted from either a user or read from a file. The functions are listed below. All input must be terminated via 'Enter' which is effectively the '\n' newline character.
These functions are performed by making system calls to the Linux Kernel.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Execution:
User Input:
	./myshell
Batchfile:
	./myshell batchfile
Note: Batchfile MUST be spelled 'batchfile', must contain a list of commands to be executed, and will quit the shell upon execution of all commands in the file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Built-Ins:
Users can enter commands from either /bin or a command from the list below:
	1.) cd <name of directory>
	-> Changes the directory at the time of execution to the indicated directory.
	-> If no arguments are provided the path of current directory will be printed.
	-> Non-existent directories will yield proper error statements and will clearly not change the directory
	2.) clr
	-> Removes all content from the screen effectively clearing it, with the prompt reappearing at the top of the screen.
	3.) dir <name of directory>
	-> Lists the content of the indicated directory if it exists
	-> If no arguments are provided the contents of the current directory are printed.
	-> A correct path name is needed to execute and if not an error message will indicate that the indicated pathname is incorrect
	4.) environ
	-> Lists environment and prints them to stdout
	5.) echo <argument>
	-> Prints the <argument> contents back to stdout and continues to print until the newline '\n' character is detected.
	6.) help
	-> Opens up a help manual describing how to run the shell, descriptions of the internal commands of the shell and how to use them, and descriptions on 	how to use I/O redirections, pipes, and running in the background.
	7.) pause
	-> Pauses shell until 'Enter' key which is the '\n' newline character is hit.
	8.) quit
	-> Quits shell
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I/O Redirection
1.) Input redirections
	-> The file following the '<' symbol will be the file which will serve as the argument when called to the function that is to be executed.
	Example: {command} {arg} < {file}
2.) Output redirections
	-> The file following the '>' symbol will serve as the file that the output of the command executed before the symbol will write to. This is done by first clearing said file and then writing into that file (as such effectively writing over the file). If the file does not exist, the file is created.
	Example: {command} {arg} > {file}
	-> The file following the '>>' symbol will serve as the file that the output of the command executed before the symbol will write to. This is done by writing to the end of file after the contents pre-existing in said file (as such effectively appending the file). If the file does not exist, the file is created.
	Example: {command} {arg} >> {file}
3.) Input and Output redirections
	-> The file following the '<' symbol will serve as the input argument for the command to be executed and the file following the '>' symbol will be the output file that will written into by writing over that file if already existing or creating that file if non-existent.
	Example: {command} {arg} < {file1} > {file2}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Piping
	-> The command before the '|' symbol will be the command that will write to the pipe. The command after the '|' symbol will read the contents of the pipe, use it as an input, and execute the command accordingly. This allows the two commands to communicate to each other with a one directional flow.
	Example: {command1} {arg1}| {command2} {arg2}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Running Background
	-> Using the '&' symbol after a command will allow a command to execute in the background and as such allows other commands to run concurrently as the indicated one does. Myshell, by default, waits for a command to finish executing before prompting the user for another command.
	Example: {command} {arg} &

<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
