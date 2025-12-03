// R Jesse Chaney
// rchaney@pdx.edu

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "cmd_parse.h"

#define PROMPT_LEN 5000
#define HIST 15

// I have this a global so that I don't have to pass it to every
// function where I might want to use it. Yes, I know global variables
// are frowned upon, but there are a couple useful uses for them.
// This is one.
unsigned short is_verbose = 0;
// Global variable to track the foreground child's PID
pid_t foreground_child_pid = 0;

char * hist[HIST] = {0};

void my_signal_handler(int signum) {
	//printf("\nReceived signal %d. Handling it...\n", signum);
	if(signum == SIGINT)
	{
		if (foreground_child_pid > 0) {
			kill(foreground_child_pid, SIGINT);
		}
	}

}

int process_user_input_simple(void)
{
	char str[MAX_STR_LEN] = {'\0'};
	char *ret_val = NULL;
	char *raw_cmd = NULL;
	cmd_list_t *cmd_list = NULL;
	int cmd_count = 0;
	char prompt[PROMPT_LEN] = {'\0'};

	char cwd[PATH_MAX] = {'\0'};
	char hostname[HOST_NAME_MAX] = {'\0'};
	char *username;
	//static char * hist[HIST] = {0};

	//getcwd(cwd, sizeof(cwd));
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		snprintf(cwd, sizeof(cwd), "[unknown_dir]");
	}

	username = getenv("LOGNAME");
	if (username == NULL) {
		username = "[unknown_user]";
	}

	//gethostname(hostname, sizeof(hostname));
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		snprintf(hostname, sizeof(hostname), "[unknown_system]");
	}

	for ( ; ; ) {

		//getcwd(cwd, sizeof(cwd));
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			snprintf(cwd, sizeof(cwd), "[unknown_dir]");
		}

		username = getenv("LOGNAME");
		if (username == NULL) {
			username = "[unknown_user]";
		}

		//gethostname(hostname, sizeof(hostname));
		if (gethostname(hostname, sizeof(hostname)) != 0) {
			snprintf(hostname, sizeof(hostname), "[unknown_system]");
		}
		/*sprintf(prompt, " %s <put directory here> \n%s<put @system name here> # "
			, PROMPT_STR
		//, current working directory
		, getenv("LOGNAME")
		//, fully qualified hostname
		);
		fputs(prompt, stdout);*/

		// test to see of stdout is a terminal device (a tty)
		if (isatty(STDOUT_FILENO)) {

			// Set up a cool user prompt.
			/*PSUsh current-working-directory
				user-name@system-name #*/
			sprintf(prompt, "PSUsh %s\n%s@%s # "
					, cwd
					, username
					, hostname
					);

			fputs(prompt, stdout);
		}



		memset(str, 0, MAX_STR_LEN);
		ret_val = fgets(str, MAX_STR_LEN, stdin);



		if (NULL == ret_val) {
			// end of input, a control-D was pressed.
			// Bust out of the input loop and go home.
			break;
		}

		// STOMP on the pesky trailing newline returned from fgets().
		if (str[strlen(str) - 1] == '\n') {
			// replace the newline with a NULL
			str[strlen(str) - 1] = '\0';
		}
		if (strlen(str) == 0) {
			// An empty command line.
			// Just jump back to the promt and fgets().
			// Don't start telling me I'm going to get cooties by
			// using continue.
			continue;
		}

		if (strcmp(str, BYE_CMD) == 0) {
			// Pickup your toys and go home. I just hope there are not
			// any memory leaks. ;-)
			break;
		}

		// I put the update of the history of command in here.


		if(!*hist)
		{
			memset(hist, 0, sizeof(hist));
		}
		free(hist[HIST - 1]);
		memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
		hist[0] = strdup(str);

		// Basic commands are pipe delimited.
		// This is really for Stage 2.
		raw_cmd = strtok(str, PIPE_DELIM);

		cmd_list = (cmd_list_t *) calloc(1, sizeof(cmd_list_t));

		// This block should probably be put into its own function.
		cmd_count = 0;
		while (raw_cmd != NULL ) {
			cmd_t *cmd = (cmd_t *) calloc(1, sizeof(cmd_t));

			cmd->raw_cmd = strdup(raw_cmd);
			cmd->list_location = cmd_count++;

			if (cmd_list->head == NULL) {
				// An empty list.
				cmd_list->tail = cmd_list->head = cmd;
			}
			else {
				// Make this the last in the list of cmds
				cmd_list->tail->next = cmd;
				cmd_list->tail = cmd;
			}
			cmd_list->count++;

			// Get the next raw command.
			raw_cmd = strtok(NULL, PIPE_DELIM);
		}
		// Now that I have a linked list of the pipe delimited commands,
		// go through each individual command.
		parse_commands(cmd_list);

		// This is a really good place to call a function to exec the
		// the commands just parsed from the user's command line.
		exec_commands(cmd_list);

		// We (that includes you) need to free up all the stuff we just
		// allocated from the heap. That linked list of linked lists looks
		// like it will be nasty to free up, but just follow the memory.
		free_list(cmd_list);
		cmd_list = NULL;
	}

	return(EXIT_SUCCESS);
}

	void 
simple_argv(int argc, char *argv[] )
{
	int opt;

	while ((opt = getopt(argc, argv, "hv")) != -1) {
		switch (opt) {
			case 'h':
				// help
				// Show something helpful
				fprintf(stdout, "You must be out of your Vulcan mind if you think\n"
						"I'm going to put helpful things in here.\n\n");
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				// verbose option to anything
				// I have this such that I can have -v on the command line multiple
				// time to increase the verbosity of the output.
				is_verbose++;
				if (is_verbose) {
					fprintf(stderr, "verbose: verbose option selected: %d\n"
							, is_verbose);
				}
				break;
			case '?':
				fprintf(stderr, "*** Unknown option used, ignoring. ***\n");
				break;
			default:
				fprintf(stderr, "*** Oops, something strange happened <%c> ... ignoring ...***\n", opt);
				break;
		}
	}
}

	void 
exec_commands( cmd_list_t *cmds ) 
{
	//char str1[MAX_STR_LEN] = {'\0'};
	char *target_dir = NULL;
	cmd_t *cmd = cmds->head;
	FILE *out_stream = stdout; // Default to stdout
	//FILE *in_stream = stdin; // Default to stdout
	/*static char * hist[HIST] = {0};

		if(!*hist)
		{
		memset(hist, 0, sizeof(hist));
		}*/

	if (1 == cmds->count) 
	{
		if (!cmd->cmd) 
		{
			// if it is an empty command, bail.
			return;
		}
		if (0 == strcmp(cmd->cmd, CD_CMD))
		{
			if (0 == cmd->param_count)
			{
				// Just a "cd" on the command line without a target directory
				// need to cd to the HOME directory.

				// Is there an environment variable, somewhere, that contains
				// the HOME directory that could be used as an argument to
				// the chdir() fucntion?

				target_dir = getenv("HOME");
				if (target_dir == NULL)
				{
					fprintf(stderr, "psush: cd: HOME environment variable not set.\n");
				}

				if(chdir(target_dir) != 0)
				{
					fprintf(stderr, "cd: chdir: failed invaid target directory/home directory\n");
				}
				/*{
					free(hist[HIST - 1]);
					memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
					hist[0] = strdup(cmd->raw_cmd);
					}*/
			}
			else 
			{
				// try and cd to the target directory. It would be good to check
				// for errors here.
				if(target_dir == NULL)
				{
					target_dir = cmd->param_list->param;
				}
				if(chdir(target_dir) != 0)
				{
					fprintf(stderr, "chdir() failed, invalid target directory\n");
				}
				/*{
					free(hist[HIST - 1]);
					memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
					hist[0] = strdup(cmd->raw_cmd);
				}*/
			}
		}
		else if (0 == strcmp(cmd->cmd, CWD_CMD)) 
		{
			char str[MAXPATHLEN];

			// Fetch the Current Working Wirectory (CWD).
			// aka - get country western dancing
			getcwd(str, MAXPATHLEN); 
			printf(" " CWD_CMD ": %s\n", str);
			/*{
				free(hist[HIST - 1]);
				memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
				hist[0] = strdup(cmd->raw_cmd);
			}*/
		}
		//echo
		else if (0 == strcmp(cmd->cmd, ECHO_CMD)) 
		{
		/*	if(cmd->output_dest != STDOUT_FILENO)
			{
				out_stream = cmd->output_dest;
			}*/
				fprintf(out_stream, "%s", cmd->raw_cmd);

			/*param_t *current_param = cmd->param_list;

			while (current_param != NULL) 
			{

				fprintf(out_stream, "%s", current_param->param);

				current_param = current_param->next;

				// 4. Add a space ONLY if there is a next parameter
				if (current_param != NULL) 
				{
					fprintf(out_stream, " ");
				}*/
					fprintf(out_stream, " ");
			//}
			/*{
				free(hist[HIST - 1]);
				memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
				hist[0] = strdup(cmd->raw_cmd);
			}*/
			fprintf(out_stream, "\n");
		}
		else if (0 == strcmp(cmd->cmd, HISTORY_CMD)) 
		{
			// display the history here
			for(int i = 0; i < HIST; ++i)
			{
				if(hist[i] != NULL)
				{
					fprintf(out_stream, "%s\n", hist[i]);
				}
			}
			/*{
				free(hist[HIST - 1]);
				memmove(&(hist[1]), &(hist[0]), (HIST - 1) * sizeof(char *));
				hist[0] = strdup(cmd->raw_cmd);
			}*/

		}
		else 
		{
			if(cmd == cmds->tail)
			{
				param_t * current = cmd->param_list;
				char *param_array[cmd->param_count + 2];
				int i = 1;

				if(!cmd)
				{
					return;
				}

				memset(param_array, '\0', (cmd->param_count + 2) * sizeof(char *));
				param_array[0] = cmd->cmd;

				while(current)
				{
					param_array[i] = current->param;
					++i;
					current = current -> next;
				}
				param_array[cmd->param_count + 1] = NULL;

				foreground_child_pid = fork();

				if(foreground_child_pid == 0)//child
				{
					//identify params vs next cmd
					//int status = 0;
					execvp(param_array[0], param_array);
					perror("exec failed");
					fprintf(stderr, "execvp failed\n");
					_exit(1);
				}
				else//parent process
				{
					int status = 0;
					wait(&status);
				}

				return;
			}

			// A single command to create and exec
			// If you really do things correctly, you don't need a special call
			// for a single command, as distinguished from multiple commands.
		}
	}
	else 
	{
		/*while(cmd != NULL)
		{
		// Other things???
		// More than one command on the command line. Who'da thunk it!
		// This is where I handle pipeline commands.
//# define REDIR_IN    "<"
//# define REDIR_OUT   ">" 
		//ls –l > file1.txt 
			//who –H > file2.txt  
			//wc < file1.txt 
			//wc < file1.txt > file2.txt 
	//FILE *out_stream = stdout; // Default to stdout
	//FILE *in_stream = stdin; // Default to stdout
		 

		if(strcmp(cmd->next->cmd, REDIR_IN) == 0)
		{
			char * in_file = cmd->input_file_name;
			//in_stream = cmd->next->next;
		}
		if(strcmp(cmd->next->cmd, REDIR_OUT) == 0)
		{
			char * out_file = cmd->output_file_name;
			//out_stream = cmd->next->next;
		}
		{
				param_t * current = cmd->param_list;
				char *param_array[cmd->param_count + 2];
				int i = 1;

				memset(param_array, '\0', (cmd->param_count + 2) * sizeof(char *));
				param_array[0] = cmd->cmd;

				while(current)
				{
					param_array[i] = current->param;
					++i;
					current = current -> next;
				}
				param_array[cmd->param_count + 1] = NULL;

				foreground_child_pid = fork();

				if(foreground_child_pid == 0)//child
				{
					int fd = 0;

					if(in_stream != stdin)
					{
						fd = open(in_stream, O_RDONLY);
						if(fd < 0)
						{
							perror(" open file input failed");
							_exit(1);
						}
						dup2(fd, STDIN_FILENO);
						close(fd);
					}

					if(out_stream != stdout)
					{
						fd = open(out_stream, O_WRONLY | O_CREAT | O_TRUNC, 0644);
						if(fd < 0)
						{
							perror("open file output failed");
							_exit(1);
						}
						dup2(fd, STDOUT_FILENO);
						close(fd);

					}

					execvp(param_array[0], param_array);
					perror("exec failed");
					fprintf(stderr, "execvp failed\n");
					_exit(1);
				}

		}

		cmd = cmd->next;*/
		
	}
				/*if(foreground_child_pid != 0)//parent process
				{
					int status = 0;
					wait(&status);
				}*/
}
/*

// A list of param_t elements.
typedef struct param_s {
    char *param;
    struct param_s *next;
} param_t;

// A linked list that has a linked list as a member.
typedef struct cmd_s {
    char    *raw_cmd;
    char    *cmd;
    param_t *param_list;
    char    *input_file_name;
    char    *output_file_name;
    struct cmd_s *next;
} cmd_t;

typedef struct cmd_list_s {
    cmd_t *head;
    cmd_t *tail;
    int count;
} cmd_list_t;
	 */
	void
free_list(cmd_list_t *cmd_list)
{
	cmd_t *current_cmd = cmd_list->head;
	cmd_t *next_cmd = NULL;

	if(cmd_list == NULL)
	{
		return;
	}

	while (current_cmd != NULL) 
	{
		next_cmd = current_cmd->next;
		free_cmd(current_cmd);
		current_cmd = next_cmd;
	}

	cmd_list->head = NULL;
  cmd_list->tail = NULL;
  cmd_list->count = 0;

}

	void
print_list(cmd_list_t *cmd_list)
{
	cmd_t *cmd = cmd_list->head;

	while (NULL != cmd) {
		print_cmd(cmd);
		cmd = cmd->next;
	}
}

	void
free_cmd (cmd_t *cmd)
{

	param_t *current_param = cmd->param_list;
	param_t *next_param = NULL;

	if(cmd == NULL)
	{
		return;
	}

	while(current_param != NULL)
	{
		next_param = current_param->next;
		if(current_param->param != NULL) 
		{
			free(current_param->param);
		}

		free(current_param);
		current_param = next_param;
	}

	if(cmd->raw_cmd != NULL)
	{
		free(cmd->raw_cmd);
	}
	if(cmd->cmd != NULL)
	{
		free(cmd->cmd);
	}

	if(cmd->input_file_name != NULL)
	{
		free(cmd->input_file_name);
	}

	if(cmd->output_file_name != NULL)
	{
		free(cmd->output_file_name);
	}

	free(cmd);
}

// Oooooo, this is nice. Show the fully parsed command line in a nice
// easy to read and digest format.
	void
print_cmd(cmd_t *cmd)
{
	param_t *param = NULL;
	int pcount = 1;

	fprintf(stderr,"raw text: +%s+\n", cmd->raw_cmd);
	fprintf(stderr,"\tbase command: +%s+\n", cmd->cmd);
	fprintf(stderr,"\tparam count: %d\n", cmd->param_count);
	param = cmd->param_list;

	while (NULL != param) {
		fprintf(stderr,"\t\tparam %d: %s\n", pcount, param->param);
		param = param->next;
		pcount++;
	}

	fprintf(stderr,"\tinput source: %s\n"
			, (cmd->input_src == REDIRECT_FILE ? "redirect file" :
				(cmd->input_src == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
	fprintf(stderr,"\toutput dest:  %s\n"
			, (cmd->output_dest == REDIRECT_FILE ? "redirect file" :
				(cmd->output_dest == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
	fprintf(stderr,"\tinput file name:  %s\n"
			, (NULL == cmd->input_file_name ? "<na>" : cmd->input_file_name));
	fprintf(stderr,"\toutput file name: %s\n"
			, (NULL == cmd->output_file_name ? "<na>" : cmd->output_file_name));
	fprintf(stderr,"\tlocation in list of commands: %d\n", cmd->list_location);
	fprintf(stderr,"\n");
}

// Remember how I told you that use of alloca() is
// dangerous? You can trust me. I'm a professional.
// And, if you mention this in class, I'll deny it
// ever happened. What happens in stralloca stays in
// stralloca.
#define stralloca(_R,_S) { (_R) = alloca(strlen(_S) + 1); strcpy(_R,_S); }

	void
parse_commands(cmd_list_t *cmd_list)
{
	cmd_t *cmd = cmd_list->head;
	char *arg;
	char *raw;

	while (cmd) {
		// Because I'm going to be calling strtok() on the string, which does
		// alter the string, I want to make a copy of it. That's why I strdup()
		// it.
		// Given that command lines should not be tooooo long, this might
		// be a reasonable place to try out alloca(), to replace the strdup()
		// used below. It would reduce heap fragmentation.
		//raw = strdup(cmd->raw_cmd);

		// Following my comments and trying out alloca() in here. I feel the rush
		// of excitement from the pending doom of alloca(), from a macro even.
		// It's like double exciting.
		stralloca(raw, cmd->raw_cmd);

		arg = strtok(raw, SPACE_DELIM);
		if (NULL == arg) {
			// The way I've done this is like ya'know way UGLY.
			// Please, look away.
			// If the first command from the command line is empty,
			// ignore it and move to the next command.
			// No need free with alloca memory.
			//free(raw);
			cmd = cmd->next;
			// I guess I could put everything below in an else block.
			continue;
		}
		// I put something in here to strip out the single quotes if
		// they are the first/last characters in arg.
		if (arg[0] == '\'') {
			arg++;
		}
		if (arg[strlen(arg) - 1] == '\'') {
			arg[strlen(arg) - 1] = '\0';
		}
		cmd->cmd = strdup(arg);
		// Initialize these to the default values.
		cmd->input_src = REDIRECT_NONE;
		cmd->output_dest = REDIRECT_NONE;

		while ((arg = strtok(NULL, SPACE_DELIM)) != NULL) {
			if (strcmp(arg, REDIR_IN) == 0) {
				// redirect stdin

				//
				// If the input_src is something other than REDIRECT_NONE, then
				// this is an improper command.
				//

				// If this is anything other than the FIRST cmd in the list,
				// then this is an error.

				cmd->input_file_name = strdup(strtok(NULL, SPACE_DELIM));
				cmd->input_src = REDIRECT_FILE;
			}
			else if (strcmp(arg, REDIR_OUT) == 0) {
				// redirect stdout

				//
				// If the output_dest is something other than REDIRECT_NONE, then
				// this is an improper command.
				//

				// If this is anything other than the LAST cmd in the list,
				// then this is an error.

				cmd->output_file_name = strdup(strtok(NULL, SPACE_DELIM));
				cmd->output_dest = REDIRECT_FILE;
			}
			else {
				// add next param
				param_t *param = (param_t *) calloc(1, sizeof(param_t));
				param_t *cparam = cmd->param_list;

				cmd->param_count++;
				// Put something in here to strip out the single quotes if
				// they are the first/last characters in arg.
				if (arg[0] == '\'') {
					arg++;
				}
				if (arg[strlen(arg) - 1] == '\'') {
					arg[strlen(arg) - 1] = '\0';
				}
				param->param = strdup(arg);
				if (NULL == cparam) {
					cmd->param_list = param;
				}
				else {
					// I should put a tail pointer on this.
					while (cparam->next != NULL) {
						cparam = cparam->next;
					}
					cparam->next = param;
				}
			}
		}
		// This could overwite some bogus file redirection.
		if (cmd->list_location > 0) {
			cmd->input_src = REDIRECT_PIPE;
		}
		if (cmd->list_location < (cmd_list->count - 1)) {
			cmd->output_dest = REDIRECT_PIPE;
		}

		// No need free with alloca memory.
		//free(raw);
		cmd = cmd->next;
	}

	if (is_verbose > 0) {
		print_list(cmd_list);
	}
}
