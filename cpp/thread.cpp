#define DEBUG_THREAD_COUNT_BIAS (0)

link_internal u32
GetWorkerThreadCount()
{
  // TODO(Jesse, blog, win32): Write code to get the number of actual physical cores
  u32 CoreCount = PlatformGetLogicalCoreCount() / 2;
  u32 Bias = 1 + DEBUG_THREAD_COUNT_BIAS; // +1 because we already have a main thread

  if (Bias >= CoreCount)
  {
    Bias = CoreCount - 1;
  }

  u32 ThreadCount = CoreCount - Bias;
  return ThreadCount;
}

link_internal u32
GetTotalThreadCount()
{
  TIMED_FUNCTION();
  u32 Result = GetWorkerThreadCount() + 1;
  return Result;
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
WaitOnFutex(bonsai_futex *Futex)
{
  TIMED_FUNCTION();

  if (Futex->SignalValue)
  {
    AtomicIncrement(&Futex->ThreadsWaiting);
    while (Futex->SignalValue)
    {
      SleepMs(3);
    }
    Assert(Futex->ThreadsWaiting > 0);
    AtomicDecrement(&Futex->ThreadsWaiting);
  }
}
