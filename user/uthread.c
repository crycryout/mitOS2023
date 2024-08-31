#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/**
 * I've been debugging it for so long, every time I call uthread in qemu, there will only be
 * C thread that print messages, I use gdb to debug, and I find every time when the cpu start execute in the
 * create b thread, thread a's state will be set FREE, and when in C create , b's state will be
 * set FREE, I see the assembler code and find that , It's an store sp instruction did that,
 * so , it must be someting wrong to the stack, I think the stack may be small, so the code overlay,I tried to give a 
 * bigger stack, it failed still, I move the field of stack, context , and state, in some situration , even c won't print,and in
 * another situation, the program crashed.
 * and I also tried to alloc a stack on the heap...., and when do that, the program just crashed.
 * I know the problem is on the stack, but I just don't know how to deal with it.
 * one morning has passed and I'm too hungry, and I think it's time to just give up and seach the internet for 
 * answer, It turns out that I just forget the stack is goes downside and should initialize the sp with the higher address
 * no wonder why the program will crash when I change the order of stack and state, context. because an stack overflow happens and
 * it just write my context and state fields with garbage!!!
 * */

/*
 * another part I need to note is , when I add t->state = RUNNABLE , the program can't print, at first I don't know why.
 * and after I look through the code, it turns out not only yiled will call schedule, but main and other stuff will call, 
 * and only the yield will call it after set the state to RUNNABLE, others might set the state as FREE, and If I set the
 * state into RUNNABLE, of coure the program will fail. I think one possible thing is: the main will just exit. and the code won't work any more.
 * */
/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4

struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  char       stack[STACK_SIZE]; /* the thread's stack */
  int        state;             /* FREE, RUNNING, RUNNABLE */
  struct context *context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(uint64, uint64);
              
void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule(). It needs a stack so that the first thread_switch() can
  // save thread 0's state.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
  current_thread->context = malloc(sizeof(struct context));
}

void 
thread_schedule(void)
{
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread;
    if(t->state == RUNNABLE) {
      next_thread = t;
      break;
    }
    t = t + 1;
  }

  if (next_thread == 0) {
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    /* YOUR CODE HERE
     * Invoke thread_switch to switch from t to next_thread:
     * thread_switch(??, ??);
     */
    //t->state = RUNNABLE; 
    thread_switch((uint64)t->context, (uint64)current_thread->context);
  } else
    next_thread = 0;
}

void 
thread_create(void (*func)())
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;
  // YOUR CODE HERE
  t->context = malloc(sizeof(struct context));
  t->context->ra = (uint64)func;
  t->context->sp = (uint64)((char*)t->stack+STACK_SIZE);
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void 
thread_a(void)
{
  int i;
  printf("thread_a started\n");
  a_started = 1;
  while(b_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_a %d\n", i);
    a_n += 1;
    thread_yield();
  }
  printf("thread_a: exit after %d\n", a_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_b(void)
{
  int i;
  printf("thread_b started\n");
  b_started = 1;
  while(a_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_b %d\n", i);
    b_n += 1;
    thread_yield();
  }
  printf("thread_b: exit after %d\n", b_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_c(void)
{
  int i;
  printf("thread_c started\n");
  c_started = 1;
  while(a_started == 0 || b_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_c %d\n", i);
    c_n += 1;
    thread_yield();
  }
  printf("thread_c: exit after %d\n", c_n);

  current_thread->state = FREE;
  thread_schedule();
}

int 
main(int argc, char *argv[]) 
{
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();
  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);
  current_thread->state = FREE;
  thread_schedule();
  exit(0);
}
