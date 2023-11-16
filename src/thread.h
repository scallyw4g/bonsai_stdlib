#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);

#define INVALID_THREAD_LOCAL_THREAD_INDEX (-1)
#define FUTEX_UNSIGNALLED_VALUE (u32_MAX) // The signal value is the thread holding the futex so we have to use the max

struct bonsai_futex
{
  volatile u32 SignalValue = FUTEX_UNSIGNALLED_VALUE;
  volatile u32 ThreadsWaiting;
  b32 Initialized;
};

#define ENSURE_FUTEX_INITIALIZED(F) if ((F)->Initialized == False) { InitializeFutex(F); }

link_internal void WaitOnFutex(bonsai_futex *Futex, b32 DoSleep = True);
link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void WaitForWorkerThreads(volatile u32 *);
link_internal void SignalAndWaitForWorkers(bonsai_futex *);



global_variable thread_local
s32 ThreadLocal_ThreadIndex = INVALID_THREAD_LOCAL_THREAD_INDEX;


link_internal void
InitializeFutex(bonsai_futex *Futex)
{
  Futex->SignalValue = FUTEX_UNSIGNALLED_VALUE;
  Futex->Initialized = True;
}


link_internal b32
UnsignalFutex(bonsai_futex *Futex, s32 SignallingThread)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  Assert(Futex->SignalValue == u32(SignallingThread));
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, FUTEX_UNSIGNALLED_VALUE, u32(SignallingThread));
  return Result;
}

link_internal b32
UnsignalFutex(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  Assert(Futex->SignalValue == (u32)ThreadLocal_ThreadIndex);
  b32 Result = UnsignalFutex(Futex, ThreadLocal_ThreadIndex);
  return Result;
}

link_internal b32
SignalFutex(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, (u32)ThreadLocal_ThreadIndex, FUTEX_UNSIGNALLED_VALUE);
  return Result;
}

link_internal b32
FutexNotSignaled(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  b32 Result = Futex->SignalValue == FUTEX_UNSIGNALLED_VALUE;
  return Result;
}

link_internal b32
FutexIsSignaled(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  b32 Result = Futex->SignalValue != FUTEX_UNSIGNALLED_VALUE;
  return Result;
}

link_internal void
AcquireFutex(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  while (SignalFutex(Futex) == False)
  {
    WaitOnFutex(Futex, False);
  }
  /* printf("(%d) acquired %p\n", ThreadLocal_ThreadIndex, Futex); */
}

link_internal void
ReleaseFutex(bonsai_futex *Futex)
{
  ENSURE_FUTEX_INITIALIZED(Futex);
  Ensure( UnsignalFutex(Futex) );
  /* printf("(%d) released %p\n", ThreadLocal_ThreadIndex, Futex); */
}








struct perlin_noise;
struct engine_resources;
struct memory_arena;

struct thread_local_state
{
  memory_arena *PermMemory;
  memory_arena *TempMemory;

  char *TempStdoutFormatStringBuffer;

  // TODO(Jesse): ?
  /* engine_resources *EngineResources; */
  perlin_noise *PerlinNoise;

  s32 Index;
  s32 Pad0[5];

  void *UserData;
};
CAssert(sizeof(thread_local_state) == CACHE_LINE_SIZE);

global_variable
thread_local_state *Global_ThreadStates;

link_internal void
SetThreadLocal_ThreadIndex(s32 Index)
{
  Assert(ThreadLocal_ThreadIndex == -1);
  ThreadLocal_ThreadIndex = Index;

  /* printf("Setting ThreadLocal_ThreadIndex (%d)\n", Index); */
}

link_internal thread_local_state *
GetThreadLocalState(s32 ThreadIndex)
{
  Assert(ThreadIndex >= 0);
  Assert(ThreadIndex < (s32)GetTotalThreadCount());

  Assert(Global_ThreadStates);

  return Global_ThreadStates + ThreadIndex;
}

link_internal memory_arena*
GetTranArena()
{
  thread_local_state *Thread = GetThreadLocalState(ThreadLocal_ThreadIndex);
  return Thread->TempMemory;
}
