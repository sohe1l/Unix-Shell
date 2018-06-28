# csc415-p3 - Shell program

## Build Instructions:
  1. Navigate to the project folder (csc415-shell-program-sohe1l)
  2. In the terminal run "make" to compile the code
  3. Alternatively run "gcc -I -Wall myshell.c -o myshell"

## Run Instructions:
  1. Navigate to the project folder (csc415-shell-program-sohe1l)
  2. In the terminal run "./myshell"

## What The Code Does:
Simple shell program parsing input and run commands using execvp which supports:	
	- Parameters passing
	- Running commands in the background using&
	- Redirects; reading input from file (<) and writing or appending to files (> and >>)
	- Running multiple commands using pipes
	- Supports cd and pwd commands.

## Extra credit:
The following extra credit items are implemented:	
	1 Nested pipe commands
	2 Supports ~ in the prompt text and the 'cd' command.
