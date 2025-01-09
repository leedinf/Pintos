#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
static void syscall_handler (struct intr_frame *);


void check_addr(void *addr) {
  if (!is_user_vaddr(addr) || !pagedir_get_page(thread_current()->pagedir, addr)) {
    sys_exit(-1);
  }
}
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  
  //2
  //intr_frame에 esp 멤버가 있고 이를 통해 스택에 접근이 가능함.
  //Arguments 있으면?? 그것들은 esp+4이렇게감.
  //!pagedir_get_page 해야하나? O
  check_addr(f->esp);
  
  int syscall_num = *(int*) f->esp; //*(int*) f->esp는 syscall_number임.
  
  // //3 syscall handler
  switch(syscall_num){
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      check_addr(f->esp + 4);
      sys_exit(*(int*)(f->esp+4));
      break;

    case SYS_EXEC:
      check_addr(f->esp + 4);
      //create child process
      f->eax = sys_exec(*(uint32_t *)(f->esp+4));
      break;
    case SYS_WAIT:
      check_addr(f->esp + 4);
      //wait child process until process finishes its work.
      f->eax = sys_wait(*(uint32_t *)(f->esp + 4));
      break;
    case SYS_READ:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      f->eax = sys_read(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8), *(uint32_t *)(f->esp + 12)); 
      break;
    case SYS_WRITE:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);

      f->eax = sys_write(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8), *(uint32_t *)(f->esp + 12));
      break;
    case SYS_FIBONACCI:
      check_addr(f->esp + 4);
      f->eax = sys_fibonacci(*(int*)(f->esp + 4));
      break;

    case SYS_MAX_OF_FOUR_INT:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      check_addr(f->esp + 16);
      f->eax = sys_max_of_four_int(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8), *(uint32_t *)(f->esp + 12), *(uint32_t *)(f->esp + 16));
      break;
  }
}

void sys_halt (void){
  shutdown_power_off();
}
void sys_exit (int status){
  
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  thread_exit();

}
pid_t sys_exec (const char *cmdline){
  check_addr(cmdline);
  return process_execute(cmdline);
}
int sys_wait (pid_t pid){
  return process_wait(pid);
}
int sys_read (int fd, void *buffer, unsigned size){
  //lock?

  if (!fd) {
    for (int i = 0; i < size; i++) {
      *((uint8_t *)buffer + i) = input_getc();
    }
    return size;
  }
  //lock?
  
  return -1;
}
int sys_write (int fd, void *buffer, unsigned size){
  //lock?
  check_addr(buffer);
  check_addr(buffer + size);
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  //lock?
  return -1; 
}
int sys_fibonacci(int n){
  int prev,now;
  prev=1;
  now=1;
  int tmp;
  if(n==0) return 0;
  n-=2;
  while(n--){
    tmp=now;
    now=prev+now;
    prev=tmp;
  }
  return now;
}

int sys_max_of_four_int(int a,int b,int c,int d){
  int m1,m2;
  m1 = a > b ? a : b;
  m2 = c > d ? c : d;
  return m1 > m2 ? m1 : m2;
}