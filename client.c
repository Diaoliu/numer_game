#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* getopt */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "number_game.h"

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in dest;
	char *ip;
	int port = 8080;
	msg_t message;

	/* process arguments
	 * disable default error message from getopt */
	opterr = 0;
	int c;
	while ((c = getopt(argc, argv, "h:p:")) != -1)
	{
		switch(c)
		{
			case 'h':
				ip = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				if (port < 1024 || port > 49151)
					print_usage("Given port is invalid!", 1);
				break;
			case '?':
				print_usage("Invalid options!", 0);
				break;
		}
	}

	/* open socket for streaming */
	sockfd = open_inet_socket(&dest, ip, port);

	/* connect to server */
	if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
	{
		error_exit("Error on connect");
	}

	Recv(sockfd, &message, sizeof(msg_t), 0);
	if (message.type == INT)
	{
		printf("%s", "Numbers are: ");
		for (int i = 0; i < NUMBER_LEN; ++i)
		{
			printf("%d ", ((int *) message.data)[i]);
		}
		printf(", total is %d\n", (int) message.len);
	}

	for(EVER)
	{
		/* clean up bytes */
		bzero(&message, sizeof(msg_t));

		printf("Enter proposition in postfix notation or 'QUIT' to exit: ");
		if (fgets(message.data, BUF_SIZE, stdin) != NULL) {
			if (strcmp(message.data, "QUIT\n")) {
				/* remove \n from string */
				message.data[strcspn(message.data, "\n")] = 0;
				message.type = STR;
				message.len = strlen(message.data);		
			} else {
				message.type = QUIT;
				message.len = 0;
			}
			/* send message to server */
			Send(sockfd, &message, sizeof(msg_t), 0);

			if (message.type == QUIT)
				break;

			/* clean up bytes */
			bzero(&message, sizeof(msg_t));

			Recv(sockfd, &message, sizeof(msg_t), 0);

			if (message.type == STR)
				printf("%s\n", message.data);
			else if (message.type == ERROR)
				printf("%s\n", "Incorrect expression.");
		}
	}

	close(sockfd);
	return 0;
}