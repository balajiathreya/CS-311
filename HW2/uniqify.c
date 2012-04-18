/*
 * CS:311 Operating systems - I HW2
 * Author: Balaji Athreya
 * A program written in C that reads a text file and outputs the unique words in the file, sorted in alphabetic order.
 */
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#define NO_OF_SORTERS	5

/*
 * this function reads from standard in and writes to a pipes
 * which is read by the suppressor
 */
void read_and_parse(int no_of_sorters, int **sorter_in, int cur_stdin);


/* 
 * this function suppresses duplicates and writes to 
 * standard output
 */
void suppressor(int no_of_sorters,int **sorter_out);

/*
 * converts given string to lowercase
 */
static char *to_lower (char *str);

int main(int argc, char *argv[]){
	int no_of_sorters;		// stores no of sorters given from input. By default, set to 5.
	int **sorter_in;		// fd array btw parser's output and sorter's input
	int **sorter_out;		// fd array btw sorter's output and suppressor's input
	int i;					// loop variable
	int sorter_in_status;	// variable to hold the status when pipes are created
	int sorter_out_status;	// variable to hold the status when pipes are created

	int cur_stdin = dup(STDIN_FILENO);
	int cur_stdout = dup(STDOUT_FILENO);
	
	if(argc > 1 &&  strcmp(argv[1], "--help") == 0){
		printf("Usage: %s [number of sorters <0-13>] < <fileName>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	else if(argc >= 2){
		no_of_sorters = atoi(argv[1]);
	}
	else{
		no_of_sorters = NO_OF_SORTERS;
	}
	/* 
	 * dynamically allocate array of file descriptor arrays sorter_in is for the parent to write parser output
	 * to and child sorter to read from sorter_out is for the child sorter to write to and
	 * suppressor to read from
	 */
	sorter_in = (int**)malloc((no_of_sorters)*sizeof(int*));
	sorter_out = (int**)malloc((no_of_sorters)*sizeof(int*));
	
	/*
	 * if the system is not able to allocate the
	 * specified memory, it will return NULL 
	 */
	if(sorter_in == NULL){
		fputs("Not able to allocate memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	if(sorter_out == NULL){
		fputs("Not able to allocate memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	//dynamically create the fd arrays for pipes.
	for(i = 0; i < no_of_sorters ; i++){
		sorter_in[i] = malloc(2 * sizeof(int));
		sorter_out[i] = malloc(2 * sizeof(int)); 
		if(sorter_in[i] == NULL){
			fputs("Not able to allocate memory\n", stderr);
			exit(EXIT_FAILURE);
		}
		if(sorter_out[i] == NULL){
			fputs("Not able to allocate memory\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	//create sorter children
	for (i = 0; i < no_of_sorters; i++){
		/*
		 * create pipes to enable communication between
		 * parent(parser) and the children(sorter)
		 */
		sorter_in_status = pipe(sorter_in[i]);
		sorter_out_status = pipe(sorter_out[i]);

		if(sorter_in_status == -1){
			fputs("Error occured while creating pipes\n",stderr);
			exit(EXIT_FAILURE);
		}
		if(sorter_out_status == -1){
			fputs("Error occured while creating pipes\n",stderr);
			exit(EXIT_FAILURE);
		}

		switch(fork()){
			case -1:
				fputs("Error while trying create child sort processes\n",stderr);
				exit(EXIT_FAILURE);
			case 0:
				/*
				 * child sorter processes are just going to read
				 * from the pipe. so close the write fd;
				 */ 
				if(close(sorter_in[i][1]) == -1){
					fputs("Error while closing sorter_in's write end in child\n",stderr);
					printf("errno: %d",errno);
					exit(EXIT_FAILURE);
				}
				/* 
				 * child sorters are not going to read from
				 * sorter_out's in. so close it
				 */
				if(close(sorter_out[i][0]) == -1){
					fputs("Error while closing sorter_out's read end in child\n",stderr);
					exit(EXIT_FAILURE);
				}
				/*
				 * child processes are going to	read from the parent. so set pipe' out as sort
				 * program's input. after duping, close the original fd
				 */
				if(sorter_in[i][0] != STDIN_FILENO){
					if(dup2(sorter_in[i][0],STDIN_FILENO) == -1){
						fputs("Error while trying to set sort's stdin to the pipe\n",stderr);
						exit(EXIT_FAILURE);
					}
					if(close(sorter_in[i][0]) == -1){
						fputs("Error while trying to close sort's in\n",stderr);
						exit(EXIT_FAILURE);
					}
				}
				/*
				 * child process will be writing to sorter_out in, so set sorter_out fd as sort program's
				 *  stdout. after duping, close the original fd
				 */
				if(sorter_out[i][1] != STDOUT_FILENO){
					if(dup2(sorter_out[i][1],STDOUT_FILENO) == -1){
						fputs("Error while trying to set sort's stdout to the pipe\n",stderr);
						exit(EXIT_FAILURE); 
					}
					if(close(sorter_out[i][1]) == -1){
						fputs("Error while trying to close sort's out\n",stderr);
						exit(EXIT_FAILURE);
					}
				}
			execlp("sort","sorting begins", (char *)NULL);
		default:
			break;
		}
		/*
		* parent process is going to write into the pipe.
		* so close the read fd
		*/
		if(close(sorter_in[i][0]) == -1){
			fputs("Error while closing sorter_in's read end in parent\n",stderr);
			exit(EXIT_FAILURE);
		}
		if(close(sorter_out[i][1]) == -1){
			fputs("Error while closing sorter_out's write end in parent\n",stderr);
			exit(EXIT_FAILURE);
		}
	}

	//fork to create the suppressor
	switch(fork()){
	case -1:
		exit(EXIT_FAILURE);
	case 0:
		for(i = 0; i < no_of_sorters; i++){
			if(close(sorter_in[i][1]) == -1){
				fputs("Error while closing file descriptor in parent\n",stderr);
				exit(EXIT_FAILURE);
			}
		}
		suppressor(no_of_sorters,sorter_out);
		//fputs("suprressor is exiting now\n",stdout);
		exit(EXIT_SUCCESS);
	default:
		break;
	}

	for(i = 0; i < no_of_sorters; i++){
		if(close(sorter_out[i][0]) == -1){
			fputs("Error while closing file descriptor in parent\n",stderr);
			exit(EXIT_FAILURE);
		}
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	dup2(cur_stdin,STDIN_FILENO);
	 dup2(cur_stdout,STDOUT_FILENO);

	read_and_parse(no_of_sorters, sorter_in, cur_stdin);

	 for (i = 0; i < no_of_sorters; i++){
		wait(0);
	 }

	 free(sorter_in);
	 free(sorter_out);
	 exit(EXIT_SUCCESS);
}

void read_and_parse(int no_of_sorters, int **sorter_in, int cur_stdin){
	char buffer[200];		// buffer to store each word
	int i = 0;				//loop variable
	
	FILE *stdin_stream;		//stream for reading standard input
	
	stdin_stream = fdopen(cur_stdin, "r");
	if(stdin_stream == NULL){
		fputs("Error while opeing standard input to read \n",stderr);
		exit(EXIT_FAILURE);
	}

	FILE **suppressor_in_streams;		//stream array to read the output of the sorters
	
	suppressor_in_streams = (FILE**)malloc((no_of_sorters)*sizeof(FILE*));
	if( suppressor_in_streams == NULL){
		fputs("Not able to allocate memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	//create streams to read from sorter's output
	for (i = 0; i < no_of_sorters; i++){
		suppressor_in_streams[i] = fdopen(sorter_in[i][1],"w");
		if(suppressor_in_streams[i] == NULL){
			fputs("Error while opening sorter_in write end\n",stderr);
			exit(EXIT_FAILURE);
		}
	}
	/*
	 * read one word at a time. words are delimited by any non-alphabetic character.
	 * and feed it to the suppressor in a round robin fashion
	 */
	while(fscanf(stdin_stream, "%*[^A-Za-z]"), fscanf(stdin_stream, "%198[a-zA-Z]", buffer) > 0) {
		//reset if counter has reached the last of the round robin distribution
		if(i > no_of_sorters-1){
			i = 0;
		}
		//add new line character to each word, so that the output is like a list of words
		strcat(buffer,"\n");
		fputs(to_lower(buffer), suppressor_in_streams[i]);
		i++;
	}
	//we have read all data from standard in. close it now.
	fclose(stdin_stream);
	
	for(i = 0; i < no_of_sorters; i++){
		if(fclose(suppressor_in_streams[i]) < 0){
			fputs("Error while closing suppressor input streams\n",stderr);
		}
	}
	  
	free(suppressor_in_streams);
	return;
}


void suppressor(int no_of_sorters, int **sorter_out){
	int numbers_string = 4096;
	int index = 0;
	int i = 0;						// loop variable
	int j = 0;						// loop variable
	int k = 0;						// loop variable
	int updateIndex = 0;
	int exists = 0;					// flag to set when match is present
	char temp[200];
	char next_word[200];			// holds the current word read
	char **words;					// final array that holds all the sorted and unique words
	
	words = malloc(numbers_string*sizeof(char*));
	if (words == NULL){
		fputs("Not able to allocate memory\n", stderr);
		exit(EXIT_FAILURE);
	}

	FILE **suppressor_in_streams = (FILE**)malloc((no_of_sorters)*sizeof(FILE*));
	if(suppressor_in_streams == NULL){
		fputs("Not able to allocate memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	// open the stream for sorter output in read mode
	for(i = 0; i < no_of_sorters; i++){
		suppressor_in_streams[i] = fdopen(sorter_out[i][0],"r");
	}

	for (i = 0; i < no_of_sorters; i++){
		while(fgets(next_word,199,suppressor_in_streams[i]) != 0){
			updateIndex = 0;
			exists = 0;
			/* 
			 * words array is full. reallocate and expand its
			 * capacity
			 */
			if(index >= numbers_string){
				numbers_string = numbers_string + 4096;
				words = realloc(words, numbers_string);
			}
			// allocate memory for a new word
			words[index] = (char*)malloc(199*sizeof(char));
			if (words[index] == NULL){
				fputs("Not able to allocate memory\n", stderr);
				exit(EXIT_FAILURE);
			}
			//copy the first word from the first stream to the array.
			if(index == 0){
				strcpy(words[index++], next_word);
			}
			else{
				/* 
				 * loop through the words array. if the word already exists
				 * set exists to 1.
				 */
				for( j = 0; j < index; j++){
					if(strcmp(next_word, words[j]) == 0){
						exists = 1;
					}
				}
				// only if exists is equal to zero, proceed further
				if(exists == 0 && strcmp(next_word,words[index-1]) != 0){
					strcpy(words[index],next_word);
					if(strcmp(words[index], words[index-1]) < 0){
						for(k = index; k > 0; k--){
							if(strcmp(words[k], words[k-1]) < 0){
								strcpy(temp,words[k-1]);
								strcpy(words[k-1],words[k]);
								strcpy(words[k],temp);
								updateIndex = 1;
							}else if(strcmp(words[k],words[k-1]) == 0){
								k=0;
								updateIndex = 0;
							}
						}
					}else if(exists == 0 && strcmp(next_word,words[index-1]) > 0){
						strcpy(words[index++],next_word);
					}
					if(updateIndex == 1){
						index++;
						updateIndex = 0;
					}
				}
			}
		}
		
	}
	//all data from all sorters are read. close the streams
	for(i = 0; i < no_of_sorters; i++){
		fclose(suppressor_in_streams[i]);
	}
	//print words array which is the output to standard out
	for(i = 0; i < index; i++){
		printf("%s",words[i]);
	}
	free(words);
	return;
}

//from http://stackoverflow.com/questions/6857445/lowercase-urls-in-varnish-inline-c
static char *to_lower (char *str){
	char *s = str;
	while (*s){
		if (isupper (*s))
		*s = tolower (*s);
		s++;
	}
	return str;
}
