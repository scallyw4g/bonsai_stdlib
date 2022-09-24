#define DEBUG_THREAD_COUNT_BIAS (3)

link_internal u32
GetWorkerThreadCount()
{
  u32 LogicalCoreCount = PlatformGetLogicalCoreCount();
  u32 Bias = 1 + DEBUG_THREAD_COUNT_BIAS; // +1 because we already have a main thread

  if (Bias >= LogicalCoreCount)
  {
    Bias = LogicalCoreCount - 1;
  }

  u32 ThreadCount = LogicalCoreCount - Bias;
  return ThreadCount;
}

link_internal u32
GetTotalThreadCount()
{
  u32 Result = GetWorkerThreadCount() + 1;
  return Result;
}

inline void
WaitForWorkerThreads(volatile u32 *WorkerCount)
{
  TIMED_FUNCTION();
  while (*WorkerCount > 0);
}

link_internal void
SuspendWorkerThreads(bonsai_futex *Futex)
{
  SignalFutex(Futex);

  u32 WorkerThreadCount = GetWorkerThreadCount();
  while(Futex->ThreadsWaiting < WorkerThreadCount);
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
