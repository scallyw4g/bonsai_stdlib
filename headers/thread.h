#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);

struct bonsai_futex
{
  volatile u32 SignalValue;
  volatile u32 ThreadsWaiting;
};

link_internal void WaitOnFutex(bonsai_futex *Futex, b32 DoSleep = True);
link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void WaitForWorkerThreads(volatile u32 *);
link_internal void SignalAndWaitForWorkers(bonsai_futex *);


link_internal b32
UnsignalFutex(bonsai_futex *Futex)
{
  Assert(Futex->SignalValue == 1);
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, 0, 1);
  return Result;
}

link_internal b32
SignalFutex(bonsai_futex *Futex)
{
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, 1, 0);
  return Result;
}

link_internal b32
FutexNotSignaled(bonsai_futex *Futex)
{
  b32 Result = Futex->SignalValue == 0;
  return Result;
}

link_internal b32
FutexIsSignaled(bonsai_futex *Futex)
{
  b32 Result = Futex->SignalValue == 1;
  return Result;
}

link_internal void
AcquireFutex(bonsai_futex *Futex)
{
  while (SignalFutex(Futex) == False)
  {
    WaitOnFutex(Futex, False);
  }
}

link_internal void
ReleaseFutex(bonsai_futex *Futex)
{
  UnsignalFutex(Futex);
}








struct perlin_noise;
struct engine_resources;
struct memory_arena;

// TODO(Jesse): Make these cache-line sized
struct thread_local_state
{
  memory_arena *PermMemory;
  memory_arena *TempMemory;

  // TODO(Jesse): ?
  engine_resources *EngineResources;
  perlin_noise *PerlinNoise;
  void* DebugStuff;

  s32 Index;

  /* mesh_freelist *MeshFreelist; */
  /* perlin_noise   Noise; */
};

global_variable thread_local
s32 ThreadLocal_ThreadIndex = -1;

global_variable
thread_local_state *Global_ThreadStates;

link_internal void
SetThreadLocal_ThreadIndex(s32 Index)
{
  Assert(ThreadLocal_ThreadIndex == -1);
  ThreadLocal_ThreadIndex = Index;
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
