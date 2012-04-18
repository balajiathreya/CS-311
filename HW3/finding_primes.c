/*
 * CS:311 Operating systems - I HW3
 * Author: Balaji Athreya
 * A program written in C that uses bitmap and threads to find prime number within the range of a 32 bit unsigned integer.
 * Sieve of Eratosthenes is used with the algorithm following
 * the algorithm mentioned in wikipedia. the for loop starts at prime^2 and
 * increments at 2*prime. all even numbers are ignored. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#define total_threads 50
#define zero_mask 0x00				// mask for setting a bit to zero in the bitmap
#define one_mask 0x01				// mask for setting a bit to one in the bitmap

uint16_t *prime_numbers;			// an array to hold the prime_numbers found	
char *bitmap;

/*
 * array to hold the partitions of the range of input numbers.
 * This array actually stores the only the maximum value in the range and 
 * not the range itself. We have as many partitions as the number of threads
 */	
uint32_t *partitions;

/*
 *  function that crosses off all non-prime number positions
 *  in the bitmap
 */

void *cross_off_non_primes(void *id);

int main(int argc, char *argv[]){
	pthread_t *threads = NULL;		// array of threads that will set non-primes to one
	int no_of_threads = 50;			// no of threads. initially set to 50.		
	int is_prime = 1;				// set it to prime, if a prime is found1

	/* 
	 * variable to hold the prime number count. initially set it 10 because
	 * we are using the first 11 prime numbers by default
	 */
	int prime_numbers_counter = 10;		
	char bit;						// variable to point to individual bits in the bitmap
	uint32_t row;					// variable to point to each row in bitmap
	uint32_t partition_size = 0;	// variable to hold the partition size

	uint64_t i = 0;					// loop counter
	int j = 0;						// loop counter
	intptr_t v;						// variable to pass to the pthread function
	
	if(argc > 1 &&  strcmp(argv[1], "--help") == 0){
		printf("Usage: %s [number of threads ] \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	else if(argc >= 2){
		no_of_threads = atoi(argv[1]);
	}

	// allocate the threads array
	threads = (pthread_t *) malloc (no_of_threads*sizeof(pthread_t));
	if(threads == NULL){
		fprintf(stderr,"error while trying to allocate threads\n");
		exit(EXIT_FAILURE);
	}
	

	// allocate the prime numbers array
	prime_numbers = (uint16_t *) malloc (6542*USHRT_MAX);
	if(prime_numbers == NULL){
		fprintf(stderr,"error while trying to allocate te prime numbers array\n");
		exit(EXIT_FAILURE);
	}
	/* 
	 * set all prime numbers in prime number array to zero because
	 * we use this to check if a new prime number has been found
	 * we know that there are only 6542 primes under square
	 * root of UINT_MAX
	 */
	memset(prime_numbers,0,6542*USHRT_MAX);

	// allocate the bitmap
	bitmap = (char *) malloc(UINT_MAX/8);
	if(bitmap  == NULL){
		fprintf(stderr,"error while trying to allocate the bitmap\n");
		exit(EXIT_FAILURE);
	}
	/* 
	 * intialize the bitmap to 0 because we are going to set
	 * all non-primes to one
	 */
	memset(bitmap,0,UINT_MAX/8);

	/*
	 * start with some known prime numbers. These are seed values
	 * to our threads
	 */
	prime_numbers[0] = 2;
	prime_numbers[1] = 3;
	prime_numbers[2] = 5;
	prime_numbers[3] = 7;
	prime_numbers[4] = 11;
	prime_numbers[5] = 13;
	prime_numbers[6] = 17;
	prime_numbers[7] = 19;
	prime_numbers[8] = 23;
	prime_numbers[9] = 29;
	prime_numbers[10] = 31;
	
	// allocate the partitions array.
	partitions = (uint32_t *) malloc(9*UINT_MAX);
	if(bitmap  == NULL){
		fprintf(stderr,"error while trying to allocate the partitions array\n");
		exit(EXIT_FAILURE);
	}

	partition_size = UINT_MAX/no_of_threads;

	/*
	 * add values to the partition array. As said before, the array holds only
	 * the maximum value in the partition and not the whole range
	 */
	partitions[0] = 2; 
	for (i = 1; i < no_of_threads; ++i){
		partitions[i] = partition_size * i;
	}
	/* 
	 * the last partition will have some value remaining as it will not be 
	 * exactly divisible by no_of_threads.
	 */
	partitions[i] = (partition_size * i) + UINT_MAX % no_of_threads;

	for(i = 0; i < no_of_threads; ++i){
		v = i;
		if(pthread_create(&threads[i], NULL, cross_off_non_primes, (void *)v)){
			fprintf(stderr,"error while trying to create thread number %lu\n",i);
			exit(EXIT_FAILURE);
		}
	}
	/*
	 *  this loop adds all prime numbers to the prime_number
	 *  array
	 */
	for(i = 33; i*i < UINT_MAX; i=i+2){
		row = i >> 3;
		bit = i & 7;
		if(!(bitmap[row] & (zero_mask << bit))){ 
			for(j = 1; j < prime_numbers_counter; ++j){
				// remainder is zero. so it is not a prime - so break
				if(i % prime_numbers[j] == 0){
					is_prime = 0;
					break;
				}
			}
			if(is_prime){
				prime_numbers[++prime_numbers_counter] = i;
			}
			else{
				is_prime = 1;
			}
		}
	}
	// join all pthreads
	for(i =  0; i < no_of_threads; ++i){
		if(pthread_join(threads[i], NULL)){
			fprintf(stdout,"error while trying to join thread %lu\n",i);
			exit(EXIT_FAILURE);
		}
	}

	free(bitmap);
	free(threads);
	free(prime_numbers);
	free(partitions);
	pthread_exit(NULL);
}

void *cross_off_non_primes(void *id){
	intptr_t p_id = (intptr_t)id;				// argument to get partition id
	int partition_id =  p_id;
	/*
	 * two variables to hold the minimum and 
	 * maximum values in a partition. Minimum value of a
	 * partition is the maximum value of previous partition +
	 * 1 and maximum value of a partition is the minimum
	 * value of next partition
	 */
	uint32_t partition_min = partitions[partition_id] + 1;
	uint32_t partition_max = partitions[partition_id + 1];
	uint32_t prime_number;						// variable to hold current prime number
	uint64_t square_of_prime;					// variable to hold the square of current prime number
	uint32_t row = 0;
	int column = 0;
	int i = 1;									// loop variable
	uint64_t j = 1;								// loop variable

	/* 
	 * we always want the min value to be an odd number
	 * so that we can skip even numbers
	 */
	if(partition_min % 2 == 0){
		++partition_min;
	}
	// get current prime number and its square
	prime_number = prime_numbers[i];
	square_of_prime = prime_number*prime_number;

	while(square_of_prime < partition_max){		
		if(square_of_prime 	< partition_min){
			j = partition_min;
			// time to cross off all non-primes
			while(j < partition_max){
				if( j >= square_of_prime){
					row = j >> 3;
					column = j & 7;
					bitmap[row] = bitmap[row] | (one_mask << column);
					// optimization from wikipedia - increment by 2*prime number
					j = j + 2 * prime_number;
				}
				else{
					j = j + 2;
				}		
			}
		}
		else{
			j = square_of_prime;
			// time to cross off all non-primes
			while(j < partition_max){
				row = j >> 3;
				column = j & 7;
				bitmap[row] = bitmap[row] | (one_mask << column);
				j = j + 2 * prime_number;
			}
		}
		// get the next prime number
		prime_number = prime_numbers[++i];
		square_of_prime = prime_number * prime_number;
		/* 
		 * when i becomes 6541, we have found all primes below
		 * sqrt of UINT_MAX
		 */
		if(i == 6541)
			break;
	}
//	fprintf(stdout,"ending thread: %d\n",partition_id);
	pthread_exit(NULL);
}


