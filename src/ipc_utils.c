#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <limits.h>

#include "ipc_utils.h"

void create_sem(int *semid)
{
	/* semaphore value, for semctl(). */
	union semun { 
		int val;
		struct semid_ds *buf;
		ushort * array;
	} sem_val;
	/* create a semaphore set */
	*semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
	if (*semid == -1) {
		perror("Error on semget");
		exit(1);
	}

	/* intialize the first (and single) semaphore in our set to '1'. */
	sem_val.val = 1;
	int rc = semctl(*semid, 0, SETVAL, sem_val);
	if (rc == -1) {
		perror("Error on semctl");
		exit(1);
	}
}

void create_shm(int *shmid, int len)
{
	int *block;
	/* create the shm block */
	if ((*shmid = shmget(IPC_PRIVATE, sizeof(int) * len, IPC_CREAT | 0660)) < 0)
	{
		perror("Error on shmget");
		exit(1);
	}
	/* attach the segment to the shm block */
	block = (int *) shmat(*shmid, NULL, 0);
	/* set every element as biggest number */
	for (int i = 0; i < len; ++i)
	{
		block[i] = INT_MAX;
	}
	shmdt(block);
}

void sem_lock(int semid)
{
	/* structure for semaphore operations.   */
	struct sembuf sem_op;

	/* wait on the semaphore, unless it's value is non-negative. */
	sem_op.sem_num = 0;
	sem_op.sem_op = -1;
	sem_op.sem_flg = 0;
	semop(semid, &sem_op, 1);
}

void sem_unlock(int semid)
{
	/* structure for semaphore operations.   */
	struct sembuf sem_op;

	/* signal the semaphore - increase its value by one. */
	sem_op.sem_num = 0;
	sem_op.sem_op = 1;
	sem_op.sem_flg = 0;
	semop(semid, &sem_op, 1);
}

void rm_shm(int shmid)
{
	if (shmctl(shmid, IPC_RMID, NULL) < 0) {
		perror("Error on shmctl");
		exit(1);
	}
}
