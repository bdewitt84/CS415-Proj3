/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/

	char *str, *token, *saveptr1, *saveptr2, *saveptr3;
	int count = 0;

	//check for null string
	if (buf == NULL) {
		return 0;
	}

	// iterate through string counting tokens
	for (str = buf; ;str = NULL) {
		token = strtok_r(str, "\n", &saveptr1);
		token = strtok_r(str, "\r", &saveptr2);
		token = strtok_r(str, delim, &saveptr3);
		if (token == NULL) {
			break;
		}
		if (strlen(token) > 0) {
			count++;
		}
		// count++;
	}

	// return the number of token
	return count;
}

command_line str_filler (char* buf, const char* delim) {	
	//TODO：
	/*
	*	#1. create command_line variable to be filled and returned
	*	#2. count the number of tokens with count_token function, set num_token. 
        *           one can use strtok_r to remove the \n at the end of the line.
	*	#3. malloc memory for token array inside command_line variable
	*	    based on the number of tokens.
	*	#4. use function strtok_r to find out the tokens 
        *       #5. malloc each index of the array with the length of tokens,
	*	    fill command_list array with tokens, and fill last spot with NULL.
	*	#6. return the variable.
	*/
    command_line result;

    // make copy because the buffer will be consumed by the function
    char* buf_cpy = strdup(buf);
    int num_token = count_token(buf_cpy, delim);
    free(buf_cpy);

    result.num_token = num_token;
    result.command_list = malloc((num_token + 1) * sizeof(char*));

    char *str, *token, *saveptr1, *saveptr2, *saveptr3;
    int i = 0;
    
    for (str = buf; ; str = NULL) {
	token = strtok_r(str, "\n", &saveptr1);
	token = strtok_r(str, "\r", &saveptr2);
        token = strtok_r(str, delim, &saveptr3);
        if (token == NULL) {
            break;
        }
	if (strlen(token) > 0 ) {
	        result.command_list[i] = malloc((strlen(token) + 1) * sizeof(char));
	        strcpy(result.command_list[i], token);
	        i++;
	}
    }

    result.command_list[num_token] = NULL;

    return result;
}

void free_command_line(command_line* command)
{
	for (int i = 0; i < command->num_token; i++) {
		free(command->command_list[i]);
	}
	free(command->command_list);
}

