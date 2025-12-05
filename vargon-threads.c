//NIKKI RUDNICK
// nrudnick@pdx.edu

// example input
// biglouis:$argon2id$v=19$m=6128,t=2,p=1$UkJpTkZ4UHFBQlM$FJ0O/Wcu1F6J9Ux3HGv+GqlVJMxOYfBzJfIdz+PUv+5m
// fastfax:$argon2id$v=19$m=5515,t=2,p=1$UHF2TWhzZy9ZLi5rdHhr$T1fZNJtQXrOKyil19lelhVKkwTzBvh1WgWn/Qsob8l0

/*static __thread char buf[MAX_ERROR_LEN]; //local variable for a thread each thread gets a local copy*/
/*int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)); //create a specificed destrcutor for local thread data*/
/*pthread_once_t once_var = PTHREAD_ONCE_INIT; necesarry once initialization?*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <argon2.h>

#define BUF_SIZE 1000

int is_verbose;

void 
run_password_cracker(char *, char *);

void run_password_cracker(char *hash_file_name, char *dictionary_file_name)
{
	FILE *hash_file;
	FILE *dictionary_file;
	
	char hash_buf[BUF_SIZE];
	char dict_buf[BUF_SIZE];
	
	//char *username = NULL;
	char *hash_string = NULL;
	char *dict_word = NULL;
	int result = -1;

	const argon2_type argon2_algo = Argon2_id;

	//open files once!!

	//open the hash file
	if((hash_file = fopen(hash_file_name, "r")) == NULL) 
	{
		perror("error opening hash file");
		exit(EXIT_FAILURE);
	}

	//open dictionary file 
	if((dictionary_file = fopen(dictionary_file_name, "r")) == NULL) 
	{
		perror("error opening dictionary file");
		fclose(hash_file);
		exit(EXIT_FAILURE);
	}

	// outer loop: read one hash line at a time
	while(fgets(hash_buf, BUF_SIZE, hash_file) != NULL)
	{
		// clean newline
		int found = 0;

		hash_buf[strcspn(hash_buf, "\n")] = 0;
		hash_string = hash_buf;

		// reset dictionary file to the beginning
		rewind(dictionary_file);

		// inner loop: check every word in dictionary against this hash
		while(fgets(dict_buf, BUF_SIZE, dictionary_file) != NULL)
		{
			// clean newline from dictionary word
			dict_buf[strcspn(dict_buf, "\n")] = 0;
			dict_word = dict_buf;

			// verify
			result = argon2_verify(hash_string, dict_word, strlen(dict_word), argon2_algo);
			
			if(result == ARGON2_OK) 
			{
				// match found: print cracked and break loop
				printf("CRACKED: %s %s\n", dict_word, hash_string);
				found = 1;
				break; 
			}
		}

		if(found == 0)
		{
			printf("*** FAILED ***:  %s\n", hash_string);
		}
	}

	// close files once
	fclose(dictionary_file);
	fclose(hash_file);
}

int
main(int argc, char *argv[])
{
	/*char buf[BUF_SIZE] = {'\0'};
	char *hash = NULL;
	char *password = NULL;
	int result = 0;
	const argon2_type argon2_algo = Argon2_id;*/

	int num_threads = 1;
	char * input_file_name = NULL;
	char * dictionary_file_name = NULL;
	char * output_file_name = NULL;


	//-------------------------------
	//process getopt
	int opt;
	//int is_verbose = 0;
	int input_file_flag = 0;
	int dictionary_file_flag = 0;

	while((opt = getopt(argc, argv, "H:P:o:t:vhn")) != -1) 
	{
		switch(opt)
		{
			case 'H':
				/*Specify the name of the input file. This file contains the hashed
					password values that you will crack. This is a required
					command line option.*/
				input_file_flag++;

				input_file_name = optarg;
				break;
			case 'P':
				/*Specify the name of the dictionary file that contains plain-text words
					that will be used to crack the passwords. This is a required
					command line option.*/
				dictionary_file_flag++;

				dictionary_file_name = optarg;
				break;
			case 'o':
				/*Specify the name of the output file. If this option is not specified for
					input, stdout is written.*/
				output_file_name = optarg;
				break;
			case 't':
				/*Specify the number of threads to use. The default is to use a single
					thread. You’ll use up to 24 threads*/
				num_threads = atoi(optarg);
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
				/*Options: nH:P:o:t:vh
					-H file         Hashes file name (required)
					-P file         Passwords file name (required)
					-o file         Output file name (default stdout)
					-t #            number of threads to create (default 1)
					-n              renice to 10
					-v              enable verbose mode
					-h              helpful text
				 */
				fprintf(stderr,
						"./vargon-threads ...\n"
						"Options: nH:P:o:t:vh\n"
						"    -H file         Hashes file name (required)\n"
						"    -P file         Passwords file name (required)\n"
						"    -o file         Output file name (default stdout)\n"
						"    -t #            Number of threads to create (default 1)\n"
						"    -n              Renice to 10\n"
						"    -v              Enable verbose mode\n"
						"    -h              Helpful text\n"
						);
				exit(EXIT_SUCCESS);
				break;
			case 'n':
				/*Apply the nice() function to your running process. Pass the value
					10 to nice() to lower the probability your process will be
					scheduled. We want to be good citizens.*/
				nice(10);
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
	{//must give name for dictionary input file with -d filename

		fprintf(stderr, "no input file command line option found, exiting...\n");
		exit(EXIT_FAILURE);
	}
	if(dictionary_file_flag == 0)
	{
		fprintf(stderr, "no dictionary file command line option found, exiting...\n");
		exit(EXIT_FAILURE);
	}

	//redirect output if necesarry
	if (output_file_name != NULL)
	{
		if (freopen(output_file_name, "w", stdout) == NULL)
		{
			perror("Error opening output file");
			exit(EXIT_FAILURE);
		}
	}

	run_password_cracker(input_file_name, dictionary_file_name);
	//-------------------------------

	//DO VIDEO ASSIGNMENT

	//give each thread a password hash and have them run through dictrionary to check
	//print suc/fail info

/*
	while(fgets(buf, BUF_SIZE, stdin) != NULL)
	{
		password = strtok(buf, ":\n");
		if (password == NULL)
		{
			fprintf(stderr, "Baaaad input >>%s<<\n", buf);
			continue;
		}
		hash = strtok(NULL, ":\n");//strtok is not threat safe

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
	}*/

	return EXIT_SUCCESS;
}
