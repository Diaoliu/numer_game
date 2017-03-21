#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/shm.h> 
#include <time.h>
/* ntoa */
#include <arpa/inet.h>

#include "number_game.h"
#include "ipc_utils.h"
#include "stack.h"

void error_exit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void print_usage(const char *err_msg, int flag)
{
	fprintf(stderr, "ERROR: %s\n", err_msg);
	if (flag == SERVER) {
		fprintf(stderr, "usage: server [-e numbers] [-n total] [-p port]\n"
		"   numbers: 7 numbers from 1 to 100\n"
		"   total: a number between 1 and 1000\n"
		"   port: 1024 to 49151 without superuser privileges\n");
	} else {
		fprintf(stderr, "usage: client [-h host] [-p port]\n"
		"   host: IPv4 address\n"
		"   port: 1024 to 49151 without superuser privileges\n");
	}
	
	exit(EXIT_FAILURE);
}

void server_init(int argc, char **argv, game_t *game, int *port)
{
	srand(time(NULL));
	game->total = (rand() % 1000) + 1;
	/* set random challenging numbers */
	for (int i = 0; i < NUMBER_LEN; ++i) {
		game->numbers[i] = (rand() % 100) + 1;
	}
	/* process arguments
	 * disable default error message from getopt */
	opterr = 0;
	int c;
	while ((c = getopt(argc, argv, "e:n:p:")) != -1)
	{
		switch(c)
		{
			case 'e':
				for (int i = 0; i < NUMBER_LEN; ++i) {
					/* optind is next argv pointer after optarg */
					int index = optind - 1 + i;
					if (index < argc) {
						/* convert string to integer */
						game->numbers[i] = atoi(argv[index]);
						if (game->numbers[i] < 1 || game->numbers[i] > 100)
							print_usage("Given numbers are invalid or not enough!", SERVER);
					} else {
						print_usage("Given numbers are invalid or not enough!", SERVER);
					}       
				}
				break;
			case 'p':
				*port = atoi(optarg);
				if (*port < 1024 || *port > 49151)
					print_usage("Given port is invalid!", SERVER);
				break;
			case 'n':
				game->total = atoi(optarg);
				if (game->total < 1 || game->total > 1000)
					print_usage("Given total is invalid!", SERVER);
				break;
			case '?':
				print_usage("Invalid options!", SERVER);
				break;
		}
	}
}

int open_inet_socket(struct sockaddr_in *addr, char *ip, const int port)
{
	/* get socket file descriptor */
	int socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (socketfd == -1)
		error_exit("ERROR on opening socket");
	/* clean up bytes */
	bzero(addr, sizeof(*addr));
	/* prepare address */
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (ip == NULL) {
		addr->sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		if(inet_pton(AF_INET, ip, &(addr->sin_addr)) <= 0)
			print_usage("Invalid IPv4 address!", CLIENT);
	}
	return socketfd;
}

void listen_socket(int socketfd, struct sockaddr_in *addr)
{
	/* bind fd and port */
	if (bind(socketfd, (struct sockaddr *)addr, sizeof(*addr)) < 0)
		error_exit("ERROR on binding");
	/* maximum 5 connections */
	if (listen(socketfd, 5) < 0)
		error_exit("ERROR on listening");
}

void do_request(int fd, game_t game, int semid, int shmid)
{
	FILE *in;
	msg_t message;
	clock_t start = clock(), diff;
	int parse_error = 0;
	int result;

	/* set fd as non-blocking mode */
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	/* send game numbers first */
	message.type = INT;
	message.len = game.total;
	memcpy(message.data, game.numbers, sizeof(int) * NUMBER_LEN);
	Send(fd, &message, sizeof(msg_t), 0);
	/* busy wait for incoming message */
	for(EVER)
	{
		bzero(&message, sizeof(msg_t));

		if (recv(fd, &message, sizeof(msg_t), 0) > 0)
		{
			start = clock();
			if (message.type == STR)
			{
				in = fmemopen(message.data, message.len, "r");
				result = parse_expr(in, game, &parse_error);

				bzero(&message, sizeof(msg_t));

				if (parse_error)
				{
					message.type = ERROR;
				}
				else
				{
					printf("get a proposition: %d.\n", result);

					message.type = STR;
					message.len = BUF_SIZE;
					int diff = ((game.total - result) > 0)? game.total - result : result - game.total;
					int posi = update_table(semid, shmid, diff);
					if (posi >= 0)
						sprintf(message.data, "Your solution is best %d.", posi + 1);
					else
						strcpy(message.data, "Outside best score table.");
				}

				Send(fd, &message, sizeof(msg_t), 0);
			}
			else if (message.type == QUIT)
			{
				printf("%s\n", "client quit ... connection closed.");
				break;
			}		
		}
		else
		{
			/* no dates are in kernel buffer */
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			if ((msec / INTERVAL) > 0) {
				printf("%s\n", "time out ... connection closed.");
				break;
			}
		}
	}
	
	close(fd);
}

int parse_expr(FILE *in, game_t game, int *parse_error)
{
	char token[BUF_SIZE];
	stack_t stack = stack_init(NUMBER_LEN);
	/* bug fixed : reset result from last time */
	*parse_error = 0;
	int valid = 0;
	while(!feof(in))
	{
		int operator;
		fscanf(in, "%s", token);
		operator = atoi(token);
		
		if (operator) {
			for (int i = 0; i < NUMBER_LEN; ++i) {
				if(operator == game.numbers[i]) {
					/* input out of range */
					valid = 1;
					break;
				}
			}

			if (valid) {
				if(stack_push(&stack, operator) < 0) {
					*parse_error = 1;
					return 0;
				}			
			} else {
				*parse_error = 1;
				return 0;
			}
			
		} else {
			if (strlen(token) == 1) {
				switch(token[0]) {
					case '+':
						if(stack_add(&stack) < 0) {
							*parse_error = 1;
							return 0;
						}
						break;
					case '-':
						if(stack_sub(&stack) < 0) {
							*parse_error = 1;
							return 0;
						}
						break;
					case '*':
						if(stack_multi(&stack) < 0) {
							*parse_error = 1;
							return 0;
						}
						break;
					case '/':
						if(stack_divide(&stack) < 0) {
							*parse_error = 1;
							return 0;
						}
						break;
					default:
						/* invalid operands */
						*parse_error = 1;
						return 0;
				}
			} else {
				/* invalid chars */
				*parse_error = 1;
				return 0;
			}
		}
	}
	/* stack may return -1, if two or more numbers on the stack*/
	int rt = stack_get(&stack);
	free_stack(&stack);
	if (rt < 0)
	{
		*parse_error = 1;
		return 0;
	}
	return rt;
}

int update_table(int semid, int shmid, int score)
{
	int index = TABLE_SZ;
	int *table = shmat(shmid, NULL, 0);
	/* enter critical area */
	sem_lock(semid);
	
	int shift = score;
	int i;
	/* find the postion */
	for (i = 0; i < TABLE_SZ; ++i)
	{
		if (shift < table[i])
		{
			int tmp = table[i];
			table[i] = score;
			shift = tmp;
			index = i;
			break;
		}
	}
	/* shift elements in the table */
	for (i = index + 1; i < TABLE_SZ; ++i)
	{
		if (shift < table[i])
		{
			int tmp = table[i];
			table[i] = shift;
			shift = tmp;
		}
	}

	/* leave critical area */
	sem_unlock(semid);
	shmdt(table);
	return (index < TABLE_SZ)? index : -1;
}

void Send(int sockfd, void *buf, size_t len, int flags)
{
	if (send(sockfd, buf, len, flags) < 0)
	{
		error_exit("Erron on send message.");
	}
}
void Recv(int sockfd, void *buf, size_t len, int flags)
{
	if (recv(sockfd, buf, len, flags) < 0)
	{
		error_exit("Erron on send message.");
	}
}