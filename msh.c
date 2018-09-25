#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MSH_RL_BUFSIZE 1024
#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELIM " \t\r\n\a"

void msh_loop();
char* msh_read_line();
char** msh_split_line(char* line);
int msh_launch(char **args);
int msh_execute(char **args);
int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);
int msh_num_builtins();

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &msh_cd,
  &msh_help,
  &msh_exit
};


int main(int argc , char **argv)
{
	msh_loop();
	return EXIT_SUCCESS;
}

void msh_loop(void)
{
  char* line;
  char** args;
  int status;

  do {
    printf("> ");
    line = msh_read_line();
    args = msh_split_line(line);
    status = msh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char* msh_read_line()
{
	int bufsize = MSH_RL_BUFSIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer)
	{
		fprintf(stderr , "msh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		//Read a character
		c = getchar();

		//if we hit EOF, replace it with a null character and return
		if (c == EOF || c == '\n')
		{
			buffer[position] = 0;
			return buffer;
		}
		else
		{
			buffer[position] = c;
		}

		position++;

		//if we had succeded the buffer, rellocate
		if(position >= bufsize)
		{
			bufsize += MSH_RL_BUFSIZE;
			buffer = realloc(buffer , bufsize);
			if(!buffer)
			{
				fprintf(stderr, "msh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char** msh_split_line(char* line)
{
	int bufsize = MSH_TOK_BUFSIZE , position = 0;
	char** tokens = malloc(bufsize * sizeof(char*));
	char* token;

	if(!tokens)
	{
		fprintf(stderr, "msh: allocation error\n");
    	exit(EXIT_FAILURE);
	}

	token = strtok(line, MSH_TOK_DELIM);
	while(token != NULL)
	{
		tokens[position] = token;
		position++;

		if(position >= bufsize)
		{
			bufsize = MSH_TOK_BUFSIZE;
			tokens = realloc(tokens , bufsize * sizeof(char*));
			if(!tokens)
			{
				fprintf(stderr, "msh: allocation error\n");
		    	exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, MSH_TOK_DELIM);
	}

	tokens[position] = NULL;
  	return tokens;
}

int msh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("msh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("msh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int msh_num_builtins() 
{
  return sizeof(builtin_str) / sizeof(char *);
}

int msh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "msh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("msh");
    }
  }
  return 1;
}

int msh_help(char **args)
{
  int i;
  printf("Manuel's shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < msh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int msh_exit(char **args)
{
  return 0;
}

int msh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < msh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return msh_launch(args);
}
