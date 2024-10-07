poof(buffer(thread_main_callback_type))
#include <generated/buffer_thread_main_callback_type.h>

#define DEBUG_THREAD_COUNT_BIAS (0)

link_internal u32
GetTotalThreadCount()
{
  /* TIMED_FUNCTION(); */
  /* u32 Result = PlatformGetLogicalCoreCount() / 2 + (PlatformGetLogicalCoreCount()/4); */
  u32 Result = 4;
  /* u32 Result = PlatformGetLogicalCoreCount(); */
  /* u32 Result = PlatformGetLogicalCoreCount()/2; */
  Assert(Result >= 4);
  return Result;
}

link_internal u32
GetWorkerThreadCount()
{
  // TODO(Jesse, blog, win32): Write code to get the number of actual physical cores
  u32 CoreCount = GetTotalThreadCount() -1;
  return CoreCount;
}

inline void
WaitForWorkerThreads(volatile u32 *WorkerCount)
{
  TIMED_FUNCTION();
  while (*WorkerCount > 0) { SleepMs(1); }
}

link_internal void
SignalAndWaitForWorkers(bonsai_futex *Futex)
{
  TIMED_FUNCTION();

  SignalFutex(Futex);

  u32 WorkerThreadCount = GetWorkerThreadCount();
  while(Futex->ThreadsWaiting < WorkerThreadCount) { SleepMs(1); }
}

link_internal void
WaitOnFutex(bonsai_futex *Futex, b32 DoSleep)
{
  /* TIMED_FUNCTION(); */

  AtomicIncrement(&Futex->ThreadsWaiting);
  while (Futex->SignalValue != FUTEX_UNSIGNALLED_VALUE) { if (DoSleep) { SleepMs(1); } }
  /* Assert(Futex->ThreadsWaiting > 0); */
  AtomicDecrement(&Futex->ThreadsWaiting);
}

link_internal thread_local_state
DefaultThreadLocalState(s32 ThreadId)
{
  thread_local_state Thread = {};

  /* Thread.EngineResources = EngineResources; */

  Thread.TempMemory = AllocateArena();
  Thread.PermMemory = AllocateArena(Megabytes(8));
  Thread.TempStdoutFormatStringBuffer = Allocate(char, Thread.PermMemory, TempStdoutFormatStringBufferSize);

  // TODO(Jesse)(safety): Given the below, how exactly is it safe to register
  // the PermMemory?  Seems to me like that's still just as liable to cause bad
  // behavior, but less likely.
  //
  // NOTE(Jesse): As it stands the debug system doesn't do any locking when
  // constructing the debug arena stats, so we can't ever free memory allocated
  // on debug registered arenas on threads outside the main one.
  //
  DEBUG_REGISTER_NAMED_ARENA(Thread.TempMemory, ThreadId, FormatCountedString(Thread.PermMemory, CSz("Thread (%d) Temp Memory"), ThreadId).Start);
  DEBUG_REGISTER_NAMED_ARENA(Thread.PermMemory, ThreadId, FormatCountedString(Thread.PermMemory, CSz("Thread (%d) Perm Memory"), ThreadId).Start);

  return Thread;
}

link_internal thread_local_state*
Initialize_ThreadLocal_ThreadStates(s32 TotalThreadCount, memory_arena* Memory)
{
  thread_local_state *Result = AllocateAligned(thread_local_state, Memory, TotalThreadCount, CACHE_LINE_SIZE);

  for ( s32 ThreadIndex = 0; ThreadIndex < TotalThreadCount; ++ThreadIndex )
  {
    Result[ThreadIndex] = DefaultThreadLocalState(ThreadIndex);
  }

  return Result;
}

link_internal void
WorkerThread_BeforeJobStart(thread_startup_params *StartupParams)
{
  if (ThreadLocal_ThreadIndex == INVALID_THREAD_LOCAL_THREAD_INDEX) { SetThreadLocal_ThreadIndex(StartupParams->ThreadIndex); }

#if BONSAI_DEBUG_SYSTEM_API
  Assert(GetDebugState());
  DEBUG_REGISTER_THREAD(StartupParams);
#endif
}


