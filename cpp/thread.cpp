#define DEBUG_THREAD_COUNT_BIAS (0)

link_internal u32
GetTotalThreadCount()
{
  /* TIMED_FUNCTION(); */
  /* u32 Result = PlatformGetLogicalCoreCount() ; */
  /* u32 Result = 16; */
  u32 Result = 2;
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
  while(Futex->ThreadsWaiting < WorkerThreadCount)
  {
    SleepMs(1);
  }
}

link_internal void
WaitOnFutex(bonsai_futex *Futex, b32 DoSleep)
{
  TIMED_FUNCTION();

  AtomicIncrement(&Futex->ThreadsWaiting);
  while (Futex->SignalValue) { if (DoSleep) { SleepMs(1); } }
  Assert(Futex->ThreadsWaiting > 0);
  AtomicDecrement(&Futex->ThreadsWaiting);
}

