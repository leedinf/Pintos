#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
typedef int pid_t;

void syscall_init (void);

void sys_halt (void);
void sys_exit (int);
pid_t sys_exec (const char *cmdline);
int sys_wait (pid_t pid);
int sys_read (int fd, void *buffer, unsigned size);
int sys_write (int fd, void *buffer, unsigned size);
int sys_fibonacci(int n);
int sys_max_of_four_int(int a,int b,int c,int d);

struct lock filesys_lock;
#endif /* userprog/syscall.h */
