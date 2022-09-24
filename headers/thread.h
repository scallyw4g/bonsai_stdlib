#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);

struct thread
{
  u32 ThreadIndex;
};

struct bonsai_futex
{
  volatile u32 SignalValue;
  volatile u32 ThreadsWaiting;
};

link_internal void
UnsignalFutex(bonsai_futex *Futex)
{
  AtomicExchange(&Futex->SignalValue, 0);
}

link_internal void
SignalFutex(bonsai_futex *Futex)
{
  AtomicExchange(&Futex->SignalValue, 1);
}

link_internal b32
FutexIsSignaled(bonsai_futex *Futex)
{
  b32 Result = Futex->SignalValue == 1;
  return Result;
}


link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void WaitForWorkerThreads(volatile u32 *);
link_internal void SignalAndWaitForWorkers(bonsai_futex *);

