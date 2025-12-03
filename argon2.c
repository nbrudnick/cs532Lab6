//NIKKI RUDNICK
// nrudnick@pdx.edu

// example input
// biglouis:$argon2id$v=19$m=6128,t=2,p=1$UkJpTkZ4UHFBQlM$FJ0O/Wcu1F6J9Ux3HGv+GqlVJMxOYfBzJfIdz+PUv+5m
// fastfax:$argon2id$v=19$m=5515,t=2,p=1$UHF2TWhzZy9ZLi5rdHhr$T1fZNJtQXrOKyil19lelhVKkwTzBvh1WgWn/Qsob8l0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <argon2.h>

#define BUF_SIZE 1000

int
main(void)
{
	char buf[BUF_SIZE] = {'\0'};
	char *hash = NULL;
	char *password = NULL;
	int result = 0;
	const argon2_type argon2_algo = Argon2_id;

//-------------------------------
//process getopt
	int opt;
	int is_verbose = 0;
	int input_file_flag = 0;
	int dictionary_file_flag = 0;

	while((opt = getopt(argc, argv, "H:P:o:t:vhn")) != -1) 
	{
		switch(opt)
		{
			case 'H':
				input_file_flag++;
				/*Specify the name of the input file. This file contains the hashed
password values that you will crack. This is a required
command line option.*/
				break;
			case 'P':
				dictionary_file_flag++;
				/*Specify the name of the dictionary file that contains plain-text words
that will be used to crack the passwords. This is a required
command line option.*/
				//
				break;
			case 'o':
				/*Specify the name of the output file. If this option is not specified for
input, stdout is written.*/
					break;
			case 't':
					/*Specify the number of threads to use. The default is to use a single
thread. You’ll use up to 24 threads*/
					break;
			case 'v':
				/*Enable some verbose processing.
• This is really to help you follow what your code is doing.
• You need to accept this switch, have your code emit some
diagnostics with this set. If your code simply prints a message
that verbose is enabled, it will meet this objective.
• All the messages from the verbose output must be sent to
stderr, NOT stdout. Do not comingle output and
diagnostics. Comingle means Comangle.*/
				is_verbose++;
				if (is_verbose) {
					fprintf(stderr, "verbose: verbose option selected: %d\n"
							, is_verbose);
				}
				break;
			case 'h':
				/* Output some helpful text about command line options. Make your
output for this option look like mine. This output must go to
stderr.*/
				fprintf(stderr, "You must be out of your Vulcan mind if you think\n"
						"I'm going to put helpful things in here.\n\n");
				exit(EXIT_SUCCESS);
				break;
			case 'n':
				/*Apply the nice() function to your running process. Pass the value
10 to nice() to lower the probability your process will be
scheduled. We want to be good citizens.*/
				break;
			case '?':
				fprintf(stderr, "*** Unknown option used, ignoring. ***\n");
				break;
			default:
				fprintf(stderr, "*** Oops, something strange happened <%c> ... ignoring ...***\n", opt);
				break;
		}
	}
	//check for required options
	if(input_file_flag == 0)
	{
				fprintf(stderr, "no input file command line option found, exiting...\n");
				exit(EXIT_FAILURE);
	}
	if(dictionary_file_flag == 0)
	{
				fprintf(stderr, "no dictionary file command line option found, exiting...\n");
				exit(EXIT_FAILURE);
	}
//-------------------------------
	while(fgets(buf, BUF_SIZE, stdin) != NULL)
	{
		password = strtok(buf, ":\n");
		if (password == NULL)
		{
			fprintf(stderr, "Baaaad input >>%s<<\n", buf);
			continue;
		}
		hash = strtok(NULL, ":\n");

		// In case you want to see the strings before being hashed
		//printf("TESTING: >>%s<< >>%s<<\n", password, hash);

		result = argon2_verify(hash, password, strlen(password), argon2_algo);
		if (result == ARGON2_OK)
		{		
			printf("CRACKED: %s %s\n", password, hash);
		}
		else 
		{
			printf("*** FAILED ***:  %s\n", hash);
		}
	}

	return EXIT_SUCCESS;
}
