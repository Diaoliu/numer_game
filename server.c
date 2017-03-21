/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* ntoa */
#include <arpa/inet.h>

#include "number_game.h"
#include "ipc_utils.h"

int main(int argc, char *argv[])
{
	game_t game;
	struct sockaddr_in addr;
	/* server run on default port */
	int port = 8080;
	/* listen to incoming connections */
	int sockfd;
	/* our shared memory identifier */
	int shmid;
	/* our semaphore identifier */
	int semid;

	/* create System V  shared memory */
	create_shm(&shmid, TABLE_SZ);
	/* create System V semaphores */
	create_sem(&semid);
	/* set up game board and running port */
	server_init(argc, argv, &game, &port);
	/* open the socket */
	sockfd = open_inet_socket(&addr, NULL, port);
	/* listen the requests */
	listen_socket(sockfd, &addr);

	printf("server run on port %d...\n", port);
	/* accept the connections and fork the process */
	for(EVER) 
	{
		struct sockaddr_in cli_addr;
		int newsockfd;
		int clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		/* create new process to handle the request */
		switch(fork())
		{
			case -1:
				error_exit("ERROR on fork");
			case 0:
				/* child process */
				printf("[pid %d] working on request from %s\n", 
						getpid(), inet_ntoa(cli_addr.sin_addr));
				do_request(newsockfd, game, semid, shmid);
				exit(EXIT_SUCCESS);
			default:
				/* father process */
				close(newsockfd);
				break;				
		}

	}
	/* close the socket */
	close(sockfd);
	/* de-allocate the shared memory segment. */
	rm_shm(shmid);
	return(EXIT_SUCCESS);
}
