
struct work_queue_task;
struct work_queue_job;

poof(hashtable_impl(work_queue_job_stats))
#include <generated/hashtable_impl$work_queue_job_stats$5cwEEtEf.h>
poof(hashtable_get_by_key(work_queue_job_stats))
#include <generated/hashtable_get_by_key$work_queue_job_stats$H3A23qAm.h>
link_internal global_job_index
GetGlobalJobIndex(work_queue *Queue, queue_job_index QueueIndex)
{
  global_job_index Result = Queue->JobIndices[QueueIndex.Index];
  return Result;
}

link_internal work_queue_job *
GetJobFromQueue(platform *Plat, work_queue *Queue, queue_job_index QueueJobIndex)
{
  work_queue_job *Result = GetJobFromGlobal(Plat, GetGlobalJobIndex(Queue, QueueJobIndex));
  return Result;
}

link_internal work_queue_task *
PopNextTaskForNextQueuedJob(platform *Plat, work_queue *Queue, queue_job_index QueueIndex)
{
  global_job_index GlobalJobIndex = GetGlobalJobIndex(Queue, QueueIndex);
  work_queue_job *Job = GetJobFromGlobal(Plat, GlobalJobIndex);

  work_queue_task *Result = PopNextTask(Job);
  return Result;
}

link_internal work_queue_job *
PopNextJob(platform *Plat, work_queue* Queue)
{
  TIMED_FUNCTION();

  work_queue_job *Result = {};
  for (;;)
  {
    /* WORKER_THREAD_ADVANCE_DEBUG_SYSTEM(); */

    // NOTE(Jesse): Must read and comared DequeueIndex instead of calling QueueIsEmpty
    u32 DequeueIndex = Queue->DequeueIndex;
    if (DequeueIndex == Queue->EnqueueIndex)
    {
      break;
    }

    b32 Exchanged = AtomicCompareExchange( &Queue->DequeueIndex,
                                           GetNextQueueIndex(DequeueIndex),
                                           DequeueIndex );
    if ( Exchanged )
    {
      Result = GetJobFromQueue(Plat, Queue, {DequeueIndex});
      break;
    }
  }

  return Result;
}








link_internal void
DrainQueue(platform *Plat, work_queue* Queue, thread_local_state* Thread, application_api *GameApi)
{
  TIMED_FUNCTION();

  for (;;)
  {
    /* WORKER_THREAD_ADVANCE_DEBUG_SYSTEM(); */

    // NOTE(Jesse): Must read and comared DequeueIndex instead of calling QueueIsEmpty
    u32 DequeueIndex = Queue->DequeueIndex;
    if (DequeueIndex == Queue->EnqueueIndex)
    {
      break;
    }

    b32 Exchanged = AtomicCompareExchange( &Queue->DequeueIndex,
                                           GetNextQueueIndex(DequeueIndex),
                                           DequeueIndex );
    if ( Exchanged )
    {
      work_queue_job *Job = GetJobFromQueue(Plat, Queue, {DequeueIndex});
      HandleJob(Job, Thread, GameApi);
    }
  }
}

link_internal THREAD_MAIN_RETURN
DefaultWorkerThread(void *Input)
{
  thread_local_state *Thread = (thread_local_state *)Input;
  Assert(Thread->ThreadIndex > 0);

  WorkerThread_BeforeJobStart(Thread);
  Assert(ThreadLocal_ThreadIndex > 0);

        auto Stdlib        =  GetStdlib();
        auto Plat          = &Stdlib->Plat;
  work_queue *LowPriority  = &Plat->LowPriority;
  work_queue *HighPriority = &Plat->HighPriority;

  auto WorkerThreadsExitFutex    = &Plat->WorkerThreadsExitFutex;
  auto WorkerThreadsSuspendFutex = &Plat->WorkerThreadsSuspendFutex;
  auto HighPriorityModeFutex     = &Plat->HighPriorityModeFutex;
  auto HighPriorityWorkerCount   = &Plat->HighPriorityWorkerCount;

  if (Stdlib->AppApi.WorkerInit) { Stdlib->AppApi.WorkerInit(GetThreadLocalState(ThreadLocal_ThreadIndex)); }

  WaitOnFutex(&Plat->ReadyToStartMainLoop, True);

  while (FutexNotSignaled(WorkerThreadsExitFutex))
  {
#if 0
    // This is a pointer to a single semaphore for all queues, so only sleeping
    // on one is sufficient, and equal to sleeping on all, because they all
    // point to the same semaphore
    ThreadSleep( Thread->HighPriority->GlobalQueueSemaphore );
#else
    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      /* TIMED_NAMED_BLOCK("CheckForWorkAndSleep"); */

      if (!QueueIsEmpty(HighPriority)) break;

      if ( ! FutexIsSignaled(HighPriorityModeFutex) &&
           ! QueueIsEmpty(LowPriority) ) break;

      if ( FutexIsSignaled(WorkerThreadsSuspendFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsExitFutex) ) break;

      if (WorkerThread_BeforeSleep) WorkerThread_BeforeSleep();

      SleepMs(1);
    }
#endif

    WaitOnFutex(WorkerThreadsSuspendFutex);

    // NOTE(Jesse): This is here to ensure the game lib (and, by extesion, the debug lib)
    // has ThreadLocal_ThreadIndex set.  This is super annoying and I want a better solution.
    WorkerThread_BeforeJobStart(Thread);
    if (Stdlib->AppApi.WorkerBeforeJob) { Stdlib->AppApi.WorkerBeforeJob(Thread); }

    AtomicIncrement(HighPriorityWorkerCount);
    DrainQueue(Plat, HighPriority, Thread, &GetStdlib()->AppApi );
    AtomicDecrement(HighPriorityWorkerCount);

#if 1
    if ( ! FutexIsSignaled(HighPriorityModeFutex) )
    {
      Ensure( RewindArena(Thread->TempMemory) );
    }
#else
    // Can't do this because the debug system needs a static handle to the base
    // address of the arena, which VaporizeArena unmaps
    //
    Ensure( VaporizeArena(Thread.TempMemory) );
    Ensure( Thread.TempMemory = AllocateArena() );
#endif

    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      if ( ! QueueIsEmpty(HighPriority)) break;

      if ( FutexIsSignaled(HighPriorityModeFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsExitFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsSuspendFutex) ) break;

      // NOTE(Jesse): Must read and comared DequeueIndex instead of calling QueueIsEmpty
      u32 DequeueIndex = LowPriority->DequeueIndex;
      if (DequeueIndex == LowPriority->EnqueueIndex)
      {
        break;
      }

      b32 Exchanged = AtomicCompareExchange( &LowPriority->DequeueIndex,
                                              GetNextQueueIndex(DequeueIndex),
                                              DequeueIndex );
      if ( Exchanged )
      {
        work_queue_job *Job = GetJobFromQueue(Plat, LowPriority, {DequeueIndex});
        HandleJob(Job, Thread, &GetStdlib()->AppApi);
        Ensure( RewindArena(Thread->TempMemory) );
      }
    }
  }

  Info("Exiting Worker Thread (%d)", Thread->ThreadIndex);
  WaitOnFutex(WorkerThreadsExitFutex);

  return 0;
}

link_weak void
LaunchWorkerThreads(platform *Plat, application_api *AppApi, thread_main_callback_type_buffer *WorkerThreadCallbackProcs)
{
  TIMED_FUNCTION();

  s32 TotalThreadCount  = (s32)GetTotalThreadCount();

  // This loop is for worker threads; it's skipping thread index 0, the main thread
  for ( s32 ThreadIndex = 1; ThreadIndex < TotalThreadCount; ++ThreadIndex )
  {
    thread_local_state *Thread =  GetThreadLocalState(ThreadIndex);

    umm CallbackProcIndex = umm(ThreadIndex-1);
    if (WorkerThreadCallbackProcs &&  CallbackProcIndex < WorkerThreadCallbackProcs->Count)
    {
      PlatformCreateThread( WorkerThreadCallbackProcs->Start[CallbackProcIndex], Cast(void*, Thread), ThreadIndex );
    }
    else
    {
      PlatformCreateThread( DefaultWorkerThread, Cast(void*, Thread), ThreadIndex );
    }
  }

  return;
}

link_internal void
ShutdownWorkerThreads(platform *Plat)
{
  if (ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX)
  {
    SignalAndWaitForWorkers(&Plat->WorkerThreadsExitFutex);
    Ensure( UnsignalFutex(&Plat->WorkerThreadsExitFutex) );
    while (Plat->WorkerThreadsExitFutex.ThreadsWaiting > 0) { SleepMs(1); };
  }
}

link_internal void
InitQueue(work_queue* Queue, memory_arena* Memory)
{
  Queue->EnqueueIndex = 0;
  Queue->DequeueIndex = 0;

  Queue->JobIndices = Allocate(global_job_index, Memory, WORK_QUEUE_SIZE);
}


