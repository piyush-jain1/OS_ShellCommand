#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#define MAX 1000000
#define call_RL_BUFSIZE 1024
#define call_TOK_BUFSIZE 64
#define call_TOK_DELIM " \t\r\n\a"

// Global Varibles
char* cmd[MAX];
int cmd_count = 0;
int CHILD_PID;

// Function Prototype Declarations
int call_cd(char **args);
int call_history(char **args);
int call_exit(char **args);
int call_rmexcept(char **args);
int call_issue(char **args);
int call_exectl(char **args);
int call_execute(char **args);
int call_launch_custom(char *line);
char **call_split_line(char *line);

// List of available commands
char *avail_command[] = {
  "cd",
  "ls",
  "history",
  "rm",
  "exit",
  "issue",
  "rmexcept",
  "exectl",
};


// List of builtin commands
char *builtin_str[] = {
  "cd",
  "history",
  "exit",
  "issue",
  "rmexcept",
  "exectl"
};

// list of functions for builtin commands
int (*builtin_func[]) (char **) = {
  &call_cd,
  &call_history,
  &call_exit,
  &call_issue,
  &call_rmexcept,
  &call_exectl
};

// size of builtins list
int size_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Pre-allocation of memory to save history of commands
void pre(){
  for(int i = 0;i < MAX; i++){
    cmd[i] = (char *)malloc(50*sizeof(char));
  }
}

// Utility function to save commands
void save_command(char *arg){
  char curr = arg[0];
  // printf("gine : %s\n", arg );
  int idx = 0;
  while(curr != '\0'){
    cmd[cmd_count][idx] = curr;
    curr = arg[idx+1];
    idx++;
  }
  cmd[cmd_count][idx] = '\0';
  cmd_count++;
}

// Utility function to copy strings (with spaces)
void str_copy(char *dest, char *src)
{ 
  int idx = 0;
  char curr = src[0];
  while(curr != '\0'){
    dest[idx] = curr;
    curr = src[idx+1];
    idx++;
  }
  dest[idx] = '\0';
  dest[idx+1] = '\n';
}

// Utility function to kill process on signal
void alarm_handler(int sig){
  signal(SIGALRM,SIG_IGN);
  kill(CHILD_PID,SIGKILL);
}

// Functions to check if the new command is in available list of commands
bool is_avail(char **args){
  bool avail = false;
  int sz = sizeof(avail_command)/sizeof(char *);
  for(int i = 0; i< sz; i++){
    if(strcmp(avail_command[i] , args[0]) == 0){
      avail = true;
    }
  }
  return avail;
}

// Builtin function implementations

/*
  Bultin command: cd - change directory.
  args : List of args.  args[0] is "cd".  args[1] is the directory.
  return : Always returns 1, to continue executing.
*/
int call_cd(char **args)
{ 
  if (args[1] == NULL) {
    fprintf(stderr, "shell_error: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shell_error");
    }
  }
  return 1;
}

/*
  Bultin command: issue - execute nth command from history.
  args : List of args.  args[0] is "issue".  args[1] is the 'n'.
  return : Always returns 1, to continue executing.
*/
int call_issue(char **args){
  char *line;
  char **new_args;
  int status;

  line = (char *)malloc(128*sizeof(char));

  if(args[1] == NULL ){
    fprintf(stderr, "shell_error: Invalid call to issue\n");
  }
  else{
    int arg_number = atoi(args[1]);

    if(arg_number > cmd_count || arg_number < 1){
      fprintf(stderr, "shell_error: command number out of bounds! \n");
    }
    else{

      str_copy(line,cmd[arg_number-1]);
      new_args = call_split_line(line);
      status = call_execute(new_args);

      free(line);
      free(new_args);
    }
  }

  return 1;
}

/*
  Bultin command: rmexcept - delete all FILES except list of files given as argument.
  args : List of args.  args[0] is "rmexcept".  Rest are list of files NOT to be deleted.
  return : Always returns 1, to continue executing.
*/
int call_rmexcept(char **args){

  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    struct dirent **namelist;
    int n;
    
    n=scandir(".",&namelist,NULL,alphasort);
    if(n < 0)
    {
      perror("scandir");
      exit(EXIT_FAILURE);
    }
    else
    {
      while (n--)
      {
        if(namelist[n]->d_type == DT_REG){
          char *new_name;
          new_name = (char *)malloc(128*sizeof(char));
          str_copy(new_name,namelist[n]->d_name);
          
          int idx = 1;
          bool delete = true;
          while(args[idx] != NULL){
            if(strcmp(args[idx],new_name) == 0){
              delete = false;
            }
            idx++;
          }

          if(delete) unlink(new_name);

        }
        free(namelist[n]);
      }
      free(namelist);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell_error");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

/*
  Bultin command: exectl - creates a child process and executes program (argument) for a maximum of m (argument) seconds.
  args : List of args.  args[0] is "exectl".  Last argument is 'm'.
  return : Always returns 1, to continue executing.
*/
int call_exectl(char **args){

  pid_t pid, wpid;
  int status,st;

  char *new_line;
  char **new_args;
  char *copy_line;
  new_line = (char *)malloc(128*sizeof(char));
  copy_line = (char *)malloc(128*sizeof(char));

  int idx = 1,pos = 0;
  int cnt = 0;
  while(args[idx] != NULL && args[idx+1] != NULL){
    cnt++;
    int tpos = 0;
    char curr = args[idx][tpos];
    while(curr != '\0'){
      new_line[pos++] = curr;
      tpos++;
      curr = args[idx][tpos];
    }
    new_line[pos++] = ' ';
    idx++;
  }
  new_line[pos-1] = '\0';
  new_line[pos] = '\n';

  str_copy(copy_line,new_line);
  if(cnt == 0){
    fprintf(stderr, "shell_error: Not enough arguments \n");
  }
  else{

    int tm = atoi(args[idx]);

    if(tm<1){
      fprintf(stderr, "shell_error: invalid time argument \n");
      return 1;
    }
    
    pid = fork();
    if (pid == 0) {
      // Child process
      new_args = call_split_line(new_line);
      bool ok = is_avail(new_args);

      if(ok) st = call_execute(new_args);
      else st = call_launch_custom(copy_line);

      exit(EXIT_FAILURE);
    } else if (pid < 0) {
      // Error forking
      perror("shell_error");
    } else {
      // Parent process
      CHILD_PID = pid;
      signal(SIGALRM,alarm_handler);
      alarm(tm);

      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
  return 1;

}

/*
  Bultin command: history - Prints the most recent n commands issued by the numbers. If n is omitted, prints all commands issued by the user.
  args : List of args.  args[0] is "history".  args[1] is 'n' (optional).
  return : Always returns 1, to continue executing.
*/
int call_history(char **args)
{ 

  int upr = cmd_count;
  if (args[1] != NULL) {
    upr = atoi(args[1]);
  } 
  for(int i = 0; i < upr; i++){
    printf("%d.  %s", i+1, cmd[i]);
  }
  return 1;
}


/*
  Bultin command: exit - Exits the shell.
  return : Always returns 0, to terminate.
*/
int call_exit(char **args)
{
  return 0;
}

/*
  Function to Launch a program and wait for it to terminate.
  args: Null terminated list of arguments (including program).
  return Always returns 1, to continue execution.
*/
int call_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell_error");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell_error");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/*
  Function to Launch a custom program and wait for it to terminate.
  args: Null terminated list of arguments (including program).
  return Always returns 1, to continue execution.
*/
int call_launch_custom(char *line)
{
  pid_t pid, wpid;
  int status;


  pid = fork();

  if (pid == 0) {
    // Child process
    system(line);    
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell_error");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

/*
  Function to Execute shell built-in or launch program.
  args: Null terminated list of arguments.
  return 1 if the shell should continue running, 0 if it should terminate
*/
int call_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < size_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return call_launch(args);
}

/*
  Function to Read a line of input from stdin.
  return The line from stdin.
*/
char *call_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}

/*
  Function to Split a line into tokens (very naively).
  Parameter: line - The line.
  return Null-terminated array of tokens.
*/
char **call_split_line(char *line)
{
  int bufsize = call_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "shell_error: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, call_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += call_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "shell_error: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, call_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

//Loop getting input and executing it.
void call_loop(void)
{
  char *line;
  char **args;
  int status;
  char *line_arg;

  do {

    line_arg = (char *)malloc(128*sizeof(char));
    printf("> ");
    line = call_read_line();
    if(line[0] != '\n'){
      save_command(line);
      str_copy(line_arg, line);
    }
    args = call_split_line(line);
        
    if(args[0] != NULL){
      if(is_avail(args))   status = call_execute(args);
      else status = call_launch_custom(line_arg);
    }
    else{
      status = 1;
    }
    
    free(line);
    free(args);

  } while (status);
}



/*
  Main Program.
  return: status code
*/
int main()
{
  // Pre-allocation of memory
  pre();

  // Run command loop
  call_loop();

  // Perform any shutdown/cleanup
  return EXIT_SUCCESS;
}
