#ifndef IPC_UTILS_H
#define IPC_UTILS_H

void create_sem(int *);
void create_shm(int *, int);

void sem_lock(int);
void sem_unlock(int);

void rm_shm(int);

#endif