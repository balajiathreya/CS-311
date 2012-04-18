/*
 * CS:311 Operating systems - I HW1
 * Author: Balaji Athreya
 * A program written in C that copies file from one location to another.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

/*
 * The program takes 2 or at the most 3 inputs - 2 filenames and an
 * optional third parameter that determines the block size to
 * be used when copying. This param can take any integer
 * value between 0 and 13. The corresponding block size to be
 * used in derived by computing the power of 2. eg: if the
 * third parameter is 5, the block size is going to be 2^5 =
 * 32.
 */

int main(int argc, char *argv[]){
	int input_fd;				/* file descripter for the input file to be copied */
	int output_fd;				/* file descripter for the new file that will be created/overwritten */
	int open_flags;				/* flags to be passed to open the destination file */ 
	int overwrite_flags;		/* flags to be passed to open the destination file in overwrite mode */
	int power;					/* int to hold the block size param that is provied */ 				
	int overwrite_option;		/* holds the overwrite option user provides if the destination file already exists */
	int buf_size = 8192;		/* default size of the buffer used for reading/writing */
	mode_t file_perms;			/* file permissions used to read/write file */
	ssize_t num_read;			/* holds the number of bytes read/written */
	
	/*
	 * acceptable number of parameters are 1(only in case of
	 * help), 2(if you don't supply a block size) and 3(if you
	 * supply a blocksize). exit otherwise 
	 */
	if(argc < 3 || strcmp(argv[1], "--help") == 0){
		printf("Usage: %s old-file new-file [block-size <0-13>]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	else if (argc > 3){
		power = atoi(argv[3]);
	
		/*
		 * if the block size supplied is greater than 13 or
		 * negative set it to a max value fo 13
		 */
		if(power < 0 || power > 13){
			printf("Incorrect block size value. Automatically setting it 13\n");
			power = 13;
		}
		buf_size = 2<<power;
	}		
		
	char buf[buf_size];				/* buffer used to read/write */
	
	input_fd = open(argv[1], O_RDONLY);
	if (input_fd == -1){
		printf("error while opening file %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
    
	/* create if not exists. fail if already exist*/
	open_flags = O_CREAT | O_WRONLY | O_EXCL;					

	/* create one if not exists.truncate the file if it exists already */
	overwrite_flags = O_CREAT | O_WRONLY | O_TRUNC;

	/* set file permissions to read and write to user, group and any other user */
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	output_fd = open(argv[2], open_flags, file_perms);
	if(output_fd < 0){
		/*
		 * a file with the same name already exists in destination. 
		 * ask the user if he/she wants to proceed
		 * overwriting
		 */
		if (errno == EEXIST) {
			printf("Destination file already exists. Do you want to overwrite? [y/n]  ");
			overwrite_option = fgetc(stdin);
			if(overwrite_option == 'y' || overwrite_option == 'Y'){ 
				 output_fd = open(argv[2], overwrite_flags, file_perms);
			}
			else{
				printf("exiting without overwriting\n");
				exit(EXIT_SUCCESS);
			}
		}
	}
	
	/* read file until eof is reached */
	while ((num_read = read(input_fd, buf, buf_size)) > 0){
		/* write the bytes into the destination and check if all read bytes are written*/
		if (write(output_fd, buf, num_read) != num_read){
			printf("couldn't write whole buffer\n");
			exit(EXIT_FAILURE);
		}
	}

	if (num_read == -1){
		printf("couldn't read input file\n");
		exit(EXIT_FAILURE);
	}

	/* close files */	
	if (close(input_fd) == -1){
		printf("error while closing file %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}
	if (close(output_fd) == -1){
		printf("error while closing file %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}
	return 0;	
}

