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
WaitForWorkerThreads(volatile u32 *WorkerThreadsWaiting)
{
  TIMED_FUNCTION();
  u32 WorkerThreadCount = GetWorkerThreadCount();
  while (*WorkerThreadsWaiting < WorkerThreadCount);
}
