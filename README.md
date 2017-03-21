# Inter-process-communication

The goal of this assignment is to get used to the IPC mechanisms available on UNIX systems in order to develop a client/server application.

You will create the classic "number game" where multiple participants have to solve a challenge proposed by the game master. The challenge is made of a list of numbers and a total number. The goal is to combine the numbers in the list using only the four basic arithmetic operations (+,-,*,/) to reach the given total, or the number closest to the total if there is no exact solution.

You will program a client program for the participants (each player runs one instance of the client program). You will also program a server program for the game master. There is only one instance of the server running during a game.

## Game rules

The challenge is made of 7 numbers from 1 to 100, chosen randomly by the game master, possibly with repeated numbers. The total is a number between 1 and 1000. The only allowed operations are +,-,* and /.

Each number from the initial list may be used only once or left unused.

Here is an example:

> Say the list is 50, 100, 3, 2, 5, 1, 5 and the total is 723. One possibility is (100+3+1)*(5+2)-5 which gives the exact result 723.

## Client/server organization

The server is started first. The parameters are given on the command line:

`./server -e 50 100 3 2 5 1 5 -n 723  -p <TCP port>`

When a client connects to the server, a process handling that specific connection is started on the server side (hint: use 'fork()'). This process sends the parameters of the game to the client.

The client is started like so:

`./client -h <host_name> -p <TCP port>`

The player can then enter the following:

- a proposition in postfix notation, such as: 100 3 + 1 + 5 2 + * 5 -
- TOP to get the 10 best results so far,
- QUIT to end.

When a client sends a proposition, the server verifies first if the expression is correct. Based on the result, the server adapts the table of best results (if needed). The table has 10 entries that must be kept sorted. If the client stays inactive for a specified time interval (a define in the server code is fine, default 10 minutes), the connection is closed by the server. The server answers the client by giving its position in the table (or "Outside best score table."). If the expression is incorrect, the server answers "Incorrect expression.".

> Hint: since multiple processes could access the score table, use a semaphore (get it using 'semget' then use the related functions to handle it) to guarantee that only one process accesses the table at a time. The table has to be stored in shared memory (use 'shmget' and family).

Use sockets for the communication between the clients and the server (use the 'socket', 'bind', 'accept', 'send' and 'recv' functions and related functions).

> Hint: use ipcs to watch shared memory and ipcsrm to remove allocated resources left over after a program crash.