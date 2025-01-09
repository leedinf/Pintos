#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"

static void syscall_handler (struct intr_frame *);
bool sys_create(const char *file, unsigned size);
bool sys_remove(const char *file);

struct lock f_lock;//file 접근 lock 
extern struct lock frame_lock;
void check_addr(void *addr) {
  if (!is_user_vaddr(addr) || addr ==NULL) {
    sys_exit(-1);
  }
}
void
syscall_init (void) 
{
  lock_init(&f_lock);
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
  
  switch(syscall_num){
    
    case SYS_HALT:
      sys_halt();
      break;

    case SYS_EXIT:
      check_addr(f->esp + 4);
      sys_exit(*(int*)(f->esp+4));
      break;

    case SYS_EXEC:
      check_addr(f->esp + 4);
      f->eax = sys_exec(*(uint32_t *)(f->esp+4));
      break;
    case SYS_WAIT:
      check_addr(f->esp + 4);
      f->eax = sys_wait(*(uint32_t *)(f->esp + 4));
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
    
    //filesys
    //1
    case SYS_READ:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      f->eax = sys_read(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8), *(uint32_t *)(f->esp + 12)); 
      break;

    case SYS_WRITE:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);

      f->eax = sys_write(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8), *(uint32_t *)(f->esp + 12));
      break;
    //2 
    //OPEN, CLOSE, CREATE, REMOVE, FILESIZE, SEEK, TELL
    case SYS_OPEN:
      check_addr(f->esp + 4);
      f->eax = sys_open(*(uint32_t*)(f->esp+4));
      
      break;
     
    case SYS_CLOSE:
      check_addr(f->esp + 4);
      sys_close(*(uint32_t *)(f->esp + 4)); 
      break;

    case SYS_CREATE:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      f->eax = sys_create(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8)); 
      break;

    case SYS_REMOVE:
      check_addr(f->esp + 4);
      f->eax = sys_remove(*(uint32_t *)(f->esp + 4)); 
      break;

    case SYS_FILESIZE:
      check_addr(f->esp + 4);
      f->eax = sys_filesize(*(uint32_t *)(f->esp + 4)); 
      break;

    case SYS_SEEK:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      sys_seek(*(uint32_t *)(f->esp + 4), *(uint32_t *)(f->esp + 8)); 
      break;

    case SYS_TELL:
      check_addr(f->esp + 4);
      f->eax = sys_tell(*(uint32_t *)(f->esp + 4)); 
      break;

  }
}
// PROJECT 1

void sys_halt (void){
  shutdown_power_off();
}
void sys_exit (int status){
  
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;

  for(int i=2;i<128;i++){
    if(thread_current()->thread_fd[i] != NULL) sys_close(i);//close(fd);
  }
  // struct list* c_list = &(thread_current ()->child_list);
  // for (struct list_elem *iter = list_begin (&(thread_current ()->child_list));
  //     iter != list_end (&(thread_current ()->child_list));
  //     iter = list_next (iter))
  //   sys_wait(list_entry(iter, struct thread, child_elem)->tid);
    
  
  thread_exit();
}
pid_t sys_exec (const char *cmdline){
  check_addr(cmdline);
  return process_execute(cmdline);
}
int sys_wait (pid_t pid){
  return process_wait(pid);
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

// PROJECT 2
// READ-, WRITE- << SEMA
// OPEN, CLOSE, CREATE+, REMOVE+, FILESIZE, SEEK, TELL << ADD
int sys_read (int fd, void *buffer, unsigned size){

  int ret = -1; // default
  if (fd == 1 || fd < 0) sys_exit(-1);
  if(thread_current()->thread_fd[fd] == NULL) sys_exit(-1);
  if(buffer == NULL) sys_exit(-1);
  check_addr(buffer);
  
  if (fd == 0) {
    lock_acquire(&f_lock);
    for (int i = 0; i < size; i++) {
      *((uint8_t *)buffer + i) = input_getc();
    }
    lock_release(&f_lock);
    ret = size;
  }
  else{
    struct file* fp = thread_current()->thread_fd[fd];
    if(fp == NULL) sys_exit(-1);

    lock_acquire(&f_lock);
    ret = file_read(fp, buffer, size);
    lock_release(&f_lock);
  }
  
  return ret;
}
int sys_write (int fd, void *buffer, unsigned size){
  int ret = -1; // default
  check_addr(buffer);
  check_addr(buffer + size - 1);
  if(buffer == NULL) sys_exit(-1);
  if (fd < 1) return ret;
  if (fd == 1) {
    lock_acquire(&f_lock);
    putbuf(buffer, size);
    lock_release(&f_lock);
    ret = size;
  }
  else{//fd>=2 -- stdin out 아닐 때
    // struct file* fp = thread_current()->thread_fd[fd];

    if(thread_current()->thread_fd[fd] == NULL) sys_exit(-1);
    
    lock_acquire(&f_lock);
    ret = file_write(thread_current()->thread_fd[fd], buffer, size);
    lock_release(&f_lock);
  }
  return ret; 
}
// OPEN, CLOSE+, CREATE+, REMOVE+, FILESIZE+, SEEK+, TELL+ << ADD

bool sys_remove (const char *file) {
  if(file == NULL) sys_exit(-1);
  bool success =false;

  lock_acquire(&f_lock);
  success = filesys_remove(file); 
  lock_release(&f_lock);

  return success; 
}

int sys_open(const char *file){//fd 리턴
  //예외필요 open empty, missing --> (f->eax) 반환 안해줘서 그랬음.
  if(file == NULL) sys_exit(-1);
  for(int i=0;i<20000000;i++){
    i= i;
  }

  check_addr(file);

  lock_acquire(&f_lock);

  struct file *open_f = filesys_open(file);

  if(open_f == NULL){
    lock_release(&f_lock);
    return -1;
  }

  struct thread* cur = thread_current();
  for(int i=2;i<128;i++){//빈 fd 찾기
    if(cur -> thread_fd[i] == NULL){ // 빈 fd 찾음
      if(strcmp(cur->name, file) == 0) file_deny_write(open_f);//열린 파일에 쓰기 방지
      thread_current()->thread_fd[i] = open_f;
      lock_release(&f_lock);
      return i;
    }
  }
  //fd못찾을 때, default
  file_close(file);

  lock_release(&f_lock);
  return -1;
}

void sys_close(int fd){

  struct thread* cur = thread_current();
  if(fd==NULL || cur->thread_fd[fd] == NULL) sys_exit(-1);

  file_close(cur->thread_fd[fd]);
  cur->thread_fd[fd] = NULL;
}

void sys_seek (int fd, unsigned pos) {
  if(thread_current()->thread_fd[fd] == NULL) sys_exit(-1);
  file_seek(thread_current()->thread_fd[fd], pos);
}

unsigned sys_tell (int fd) {
  if(thread_current()->thread_fd[fd] == NULL) sys_exit(-1);
  return file_tell(thread_current()->thread_fd[fd]);
}
  
int sys_filesize (int fd) {
  if(thread_current()->thread_fd[fd] == NULL ) sys_exit(-1);

  if (fd == 0) return -1;
  return file_length(thread_current()->thread_fd[fd]);
}

bool sys_create (const char *file, unsigned size) {
  if(file == NULL) sys_exit(-1);
  bool success=false;
  lock_acquire(&f_lock);
  success = filesys_create(file,size); 
  lock_release(&f_lock);
  return success;
}