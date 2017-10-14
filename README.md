# OS_ShellCommand

**Overall Structure :**

* The 'main' calls one loop which continuosly reads lines as commands , pre-processes and tokenizes it, and then does the relevant job.
* This will go on and on until the status flag is 1, whic is the return value of each execution, except exit.
* When the exit is called, the return value is 0, which will  terminate the loop, and the exit shell program.
* Description of working of each function has been given in below table.
* Specific details of each of these as well as utility functions are also there in comments in the source code.

**NOTE :**
* Executables for running tests are already created by makefile.
* Go to the directory and run:  make
* The main shell program is main.c

**Command Descriptions :**

1. - Command : cd <directory_name> 
   - Usage Example : cd Desktop	
   - Description : Changes current directory if user has appropriate permissions.

******************************************************************************************

2. - Command : ls
   - Usage Example : ls			
   - Description : Lists information about files I the current directory.

*******************************************************************************************

3. - Command : rm <file_name>	
   - Usage Example : rm test.txt	
   - Description : Deletes indicated files. Supports options –r, -f, -v

*******************************************************************************************

4. - Command : history n
   - Usage Example : history 5
   - Description : Prints the most recent n commands issued by the numbers. If n is omitted, prints all commands issued by the user.

********************************************************************************************

5. - Command : issue n 
   - Usage Example : issue 4	 
   - Description : Issues the nth command in the history once again.

********************************************************************************************

6. - Command : <program_name>
   - Usage Example : ./test < input.txt > output.txt
   - Description : Creates a child process to run <program_name>. Supports the redirection operators > and < to redirect the input and ouput of the program to indicated files.

********************************************************************************************


7. - Command : rmexcept <list_of_files>
   - Usage Example : rmexcept test1.txt test2.txt
   - Description : Removes all files except those in <list_of_files> from the current directory. You can test this command in the dummy directory 'Dummy' given.

*********************************************************************************************

8. - Command : exectl <program_name> m
   - Usage Example : exectl ./test 5
   - Description : Creates a child process to execute program_name , but aborts the process if it does not complete its operation in m seconds.

**********************************************************************************************

9. - Command : exit
   - Usage Example : exit
   - Description : Exits the shell

**********************************************************************************************
