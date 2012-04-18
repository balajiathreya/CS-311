#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>


#define THREAD_LISTEN_PORT 8989	//port where the thread listens to KILL_SIGNAL itself
#define KILL_SIGNAL 0xBD

/* function that runs in a separate thread to listen for kill signal */
void *signal_listener(void *v);

int main(int argc, char **argv){
	
	pthread_t *listener_thread;             /* thread that listens for kill signal*/
	uintptr_t v = 0;                        /* To elimiate the pointer warning during pthread_create() */

	/*some variables for performance measurement*/
	uint32_t performance_a = 4294967295;
	uint32_t performance_d = 0;    
	int performance_b = 9;
	int performance_c = 0;
	int one = 1;
	clock_t start;     
	clock_t finish;	

	/* Socket Variables */
	struct addrinfo socket_address;
	struct addrinfo *result, *rp;
	int socket_fd;
	int s;
	int test_port = 0;
	unsigned char test[9];
	uint32_t res = 0;
	int red = 0;
	buf_size = 9;
	unsigned char buf[9] = {0};
	char hostname[255];
	char port[255];
	/* Range values */
	uint32_t min = 0;
	uint32_t max = 0;
	uint32_t sum = 0;

	/* User argument variables */
	char *endpt;
	char *str;

	if(argc != 3 || strcmp(argv[1], "--help") == 0){ /* Check to see if the user needs help  */
		printf("Usage: %s hostname port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	strcpy(hostname, argv[1]);
	errno = 0;
	str = argv[2];
	test_port = strtoumax(str, &endpt, 10);
	if ((errno == ERANGE && (test_port == INT_MAX || test_port == INT_MIN))
                   || (errno != 0 && test_port == 0) || endpt == str || test_port < 1024) {
		printf("Invalid Port.\nTerminating Client.\n");
		exit(EXIT_FAILURE);
	}
	/* fill the char array with the port input */
	strcpy(port, argv[2]);

    
	listener_thread = (pthread_t *) malloc (sizeof(pthread_t));
	if(listener_thread == NULL){
		printf("failed to allocate threads!\n");
		exit(EXIT_FAILURE);
	}
    
	if(pthread_create(listener_thread, NULL, signal_listener, (void *)v)){
		printf("failed to create listener thread!\n");
		exit(EXIT_FAILURE);
	}
    
	/* measure performance*/
	start = clock();
	while ((finish = clock())<(1*CLOCKS_PER_SEC)){
		performance_c=(performance_a%performance_b) >> one;
		++performance_d;
	}
    
	/* Obtain address(es) matching host/port */
	memset(&socket_address, 0, sizeof(struct addrinfo));
	socket_address.ai_family = AF_INET;
	socket_address.ai_socktype = SOCK_STREAM;
	socket_address.ai_flags = 0;
	socket_address.ai_protocol = 0;

	printf("Manage host and port values %s:%s\n",hostname, port);

	for (;;){
		s = getaddrinfo(hostname, port, &socket_address, &result);
		if (s != 0) {
			printf("Invaid hostname or port.\n");
			exit(EXIT_FAILURE);
		}
		
		/* getaddrinfo() returns a list of address structures.
		   Try each address until we successfully connect(2).
		   If socket(2) (or connect(2)) fails, we (close the socket
		   and) try the next address. */
		
		for (rp = result; rp != NULL; rp = rp->ai_next) {
			socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (socket_fd == -1)
				continue;
			if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
				break;                  /* Success */
			close(socket_fd);
		}
		
		if (rp == NULL) {               /* No address succeeded */
			printf("Could not connect!\n");
			exit(EXIT_FAILURE);
		}
		
		freeaddrinfo(result);           /* No longer needed */
		
		/* formulate the request msg */
		test[0] = 274;                  /* Message type */
		test[1] = performance_d >> 24;  /* perf characteristics */
		test[2] = performance_d >> 16;  /* " " */
		test[3] = performance_d >> 8;   /* " " */
		test[4] = performance_d;        /* " " */
		test[5] = 0;                    /* No results */
		test[6] = 0;                    /* " " */
		test[7] = 0;                    /* " " */
		test[8] = 0;                    /* " " */
		
		if (write(socket_fd, test,9) != 9) {
			printf("partial/failed write!\n");
			exit(EXIT_FAILURE);
		}
		shutdown(socket_fd, 1);
		
		
		memset(buf,0,buf_size); /* reset buffer to 0 */
		red = read(socket_fd, buf, buf_size);
		if (red == -1) {
			printf("read failed.\n");
			exit(EXIT_FAILURE);
		}
		shutdown(socket_fd, 0);
		close(socket_fd);

		min = 0; /* reset min & max */
		max = 0;
		if (buf[0] == 277){

			/* parse the range data */
			min = buf[1];
			min = (min << 8) | buf[2];
			min = (min << 8) | buf[3];
			min = (min << 8) | buf[4];
			max = buf[5];
			max = (max << 8) | buf[6];
			max = (max << 8) | buf[7];
			max = (max << 8) | buf[8];
			
			if(min == 0 && max == 0){
				printf("All done.\nTerminating Client.\n");
				KILL_SIGNALl(0,SIGINT);
				exit(EXIT_SUCCESS);
			}


			
			if(min<6){
				min = 6;
			}else if(min%2 != 0){
				++min;
			}
			if(max%2 != 0){
				--max;
			}
			for(uint32_t l_min = min; l_min < max; l_min += 2){
				for(uint32_t i = 1; i < l_min; ++i){
					if(l_min%i==0){
						sum += i;
					}
				}
				if (sum == l_min){
					res = l_min;
					s = getaddrinfo(hostname, port, &socket_address, &result);
					if (s != 0) {
						printf("Failed to resolve hostname.\n");
						exit(EXIT_FAILURE);
					}
					for (rp = result; rp != NULL; rp = rp->ai_next) {
						socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
						if (socket_fd == -1)
							continue;
						if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
							break;                  /* Success */
						close(socket_fd);
					}
					if (rp == NULL) {               /* No address succeeded */
						printf("Connection Failed.\n");
						exit(EXIT_FAILURE);
					}					
					freeaddrinfo(result);           /* No longer needed */

					/* formulate the result message */					
					test[0] = 274;
					test[1] = 0;
					test[2] = 0;
					test[3] = 0;
					test[4] = 0;
					test[5] = res >> 24;
					test[6] = res >> 16;
					test[7] = res >> 8;
					test[8] = res;
					
					/*printf("Sending perfect number: %u\n",res);*/
					
					if (write(socket_fd, test, 9) != 9) {
						printf("partial/failed write!\n");
						exit(EXIT_FAILURE);
					}
					close(socket_fd);
				}
				sum = 0;
			}
			close(socket_fd);
		}
	}
	if(pthread_join(*listener_thread, NULL)){
		printf("Failed to join listener thread.\n");
		exit(EXIT_FAILURE);
	}
	printf("Terminiating Client.\n");
	pthread_exit(NULL);
}

void *signal_listener (void *v){

	int listener_fd;	//fd for listening socket
	int incoming_fd;	//fd for new connection
	int buf_size = 1;
	int num_read = 0;
	int yes = 1;
	char buf[buf_size];
	socklen_t addrlen;
	struct sockaddr_storage claddr;
	struct sockaddr_in serveraddr;


	if((listener_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Socket initialization failed.\n");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)	{
		perror("Socket config failed.");
		exit(EXIT_FAILURE);
	}    

	// setting up
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(THREAD_LISTEN_PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);

	/* bind the socket fd with this sockaddr_in struct */
	if(bind(listener_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1){
		printf("Socket bind failed.\n");
		exit(EXIT_FAILURE);
	}
     
	/* listen-set the socket/fd to be passive */
	if(listen(listener_fd, 10) == -1){
		printf("Listen failed.\n");
		exit(EXIT_FAILURE);
	}

	
	for(;;){
		addrlen = sizeof(struct sockaddr_storage);
		incoming_fd = accept(listener_fd, (struct sockaddr *) &claddr, &addrlen);
		if (incoming_fd == -1) {
			printf("Incoming connection failed.\n");
			continue;
		}	
		num_read = read(incoming_fd,buf,buf_size);
		if(num_read != -1){
			if((buf[0] & KILL_SIGNAL) == KILL_SIGNAL){
				printf("Terminiating Client.\n");
				KILL_SIGNALl(0,SIGINT);
			}
		}
		close(incoming_fd);
	}
	pthread_exit(NULL);
}
