// R Jesse Chaney
// rchaney@pdx.edu

// example input
// biglouis:$argon2id$v=19$m=6128,t=2,p=1$UkJpTkZ4UHFBQlM$FJ0O/Wcu1F6J9Ux3HGv+GqlVJMxOYfBzJfIdz+PUv+5m
// fastfax:$argon2id$v=19$m=5515,t=2,p=1$UHF2TWhzZy9ZLi5rdHhr$T1fZNJtQXrOKyil19lelhVKkwTzBvh1WgWn/Qsob8l0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    while(fgets(buf, BUF_SIZE, stdin) != NULL) {
		password = strtok(buf, ":\n");
		if (password == NULL) {
			fprintf(stderr, "Baaaad input >>%s<<\n", buf);
			continue;
		}
		hash = strtok(NULL, ":\n");
		
		// In case you want to see the strings before being hashed
		//printf("TESTING: >>%s<< >>%s<<\n", password, hash);

		result = argon2_verify(hash, password, strlen(password), argon2_algo);
		if (result == ARGON2_OK) {		
			printf("CRACKED: %s %s\n", password, hash);
		}
		else {
			printf("*** FAILED ***:  %s\n", hash);
		}
    }
        
    return EXIT_SUCCESS;
}
