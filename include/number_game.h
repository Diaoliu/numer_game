#ifndef NUMBER_GAME_H
#define NUMBER_GAME_H
/* FILE */
#include <stdio.h>
/* sockaddr_in */
#include <netinet/in.h>

#define NUMBER_LEN 7
#define TABLE_SZ 10
#define BUF_SIZE 1024
#define EVER ;;
/* socket closed in 60 seconds */
#define INTERVAL 60000

enum { SERVER, CLIENT } MODE;

enum { INT, STR, QUIT, ERROR } MSG_TYPE;

typedef struct {
	/* The challenge is made of 7 numbers from 1 to 100
	 * chosen randomly by the game master
	 * possibly with repeated numbers */
	int numbers[NUMBER_LEN];
	/* set random number for total */
	int total;
} game_t;

typedef struct {
	int type;
	size_t len;
	char data[BUF_SIZE];
} msg_t;

void error_exit(const char *);
void print_usage(const char *, int);

void create_shared_memory(int *, int *);
void server_init(int, char **, game_t *, int *);
int  open_inet_socket(struct sockaddr_in *, char *, const int);
void listen_socket(int, struct sockaddr_in *);
void do_request(int, game_t, int, int);
int parse_expr(FILE *, game_t, int *);
int update_table(int, int, int);

void Send(int, void *, size_t, int);
void Recv(int, void *, size_t, int);

#endif