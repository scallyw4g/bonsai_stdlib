poof(buffer(thread_main_callback_type))
#include <generated/buffer_thread_main_callback_type.h>

#define DEBUG_THREAD_COUNT_BIAS (0)

link_internal u32
GetTotalThreadCount()
{
  /* TIMED_FUNCTION(); */
  /* u32 Result = PlatformGetLogicalCoreCount() / 2 + (PlatformGetLogicalCoreCount()/4); */
  /* u32 Result = 4; */
  /* u32 Result = PlatformGetLogicalCoreCount(); */
  u32 Result = Max(4u, PlatformGetLogicalCoreCount()/2);
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
DefaultThreadLocalState(s32 ThreadIndex, platform *Plat, void *UserData)
{
  Assert(Global_Stdlib);

  thread_local_state Thread = {};

  Thread.Stdlib = Global_Stdlib;
  Thread.ThreadIndex = ThreadIndex;
  Thread.TempMemory = AllocateArena();
  Thread.PermMemory = AllocateArena(Megabytes(8));
  Thread.TempStdoutFormatStringBuffer = Allocate(char, Thread.PermMemory, TempStdoutFormatStringBufferSize);
  Thread.UserData = UserData;

  // TODO(Jesse)(safety): Given the below, how exactly is it safe to register
  // the PermMemory?  Seems to me like that's still just as liable to cause bad
  // behavior, but less likely.
  //
  // NOTE(Jesse): As it stands the debug system doesn't do any locking when
  // constructing the debug arena stats, so we can't ever free memory allocated
  // on debug registered arenas on threads outside the main one.
  //
  DEBUG_REGISTER_NAMED_ARENA(Thread.TempMemory, ThreadIndex, FormatCountedString(Thread.PermMemory, CSz("Thread (%d) Temp Memory"), ThreadIndex).Start);
  DEBUG_REGISTER_NAMED_ARENA(Thread.PermMemory, ThreadIndex, FormatCountedString(Thread.PermMemory, CSz("Thread (%d) Perm Memory"), ThreadIndex).Start);

  return Thread;
}

link_internal thread_local_state *
Initialize_ThreadLocal_ThreadStates(platform *Plat, s32 TotalThreadCount, void *ThreadState_UserData, memory_arena* Memory)
{
  thread_local_state *Result = AllocateAligned(thread_local_state, Memory, TotalThreadCount, CACHE_LINE_SIZE);

  for ( s32 ThreadIndex = 0; ThreadIndex < TotalThreadCount; ++ThreadIndex )
  {
    Result[ThreadIndex] = DefaultThreadLocalState(ThreadIndex, Plat, ThreadState_UserData);
  }

  return Result;
}

link_internal void
WorkerThread_BeforeJobStart(thread_local_state *Thread)
{
  Global_Stdlib = Thread->Stdlib;
  Assert(Global_Stdlib);

  if (ThreadLocal_ThreadIndex == INVALID_THREAD_LOCAL_THREAD_INDEX) { SetThreadLocal_ThreadIndex(Thread->ThreadIndex); }

#if BONSAI_DEBUG_SYSTEM_API
  Assert(GetDebugState());
  DEBUG_REGISTER_THREAD(Thread);
#endif
}


link_internal thread_local_state *
GetThreadLocalState(s32 ThreadIndex)
{
  Assert(ThreadIndex >= 0);
  Assert(ThreadIndex < (s32)GetTotalThreadCount());

  auto Stdlib = GetStdlib();
  return Stdlib->ThreadStates + ThreadIndex;
}

link_internal memory_arena*
GetTranArena()
{
  thread_local_state *Thread = GetThreadLocalState(ThreadLocal_ThreadIndex);
  return Thread->TempMemory;
}
