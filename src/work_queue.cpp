#define BONSAI_WORK_QUEUE_INCLUDED (1)

link_internal work_queue_entry *
PopWorkQueueEntry(work_queue* Queue)
{
  TIMED_FUNCTION();

  work_queue_entry *Result = {};
  for (;;)
  {
    WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

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
      Result = (work_queue_entry*)(Queue->Entries + DequeueIndex);
      break;
    }
  }

  return Result;
}

link_internal void
DrainQueue(work_queue* Queue, thread_local_state* Thread, application_api *GameApi)
{
  TIMED_FUNCTION();

  for (;;)
  {
    WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

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
      volatile work_queue_entry* Entry = Queue->Entries + DequeueIndex;
      HandleJob(Entry, Thread, GameApi);
    }
  }
}

link_internal THREAD_MAIN_RETURN
DefaultWorkerThread(void *Input)
{
  thread_startup_params *ThreadParams = (thread_startup_params *)Input;
  WorkerThread_BeforeJobStart(ThreadParams);
  Assert(ThreadLocal_ThreadIndex > 0);

  /* Info("Starting Thread (%d)", ThreadLocal_ThreadIndex); */

  thread_local_state *Thread = GetThreadLocalState(ThreadLocal_ThreadIndex);
  Thread->Index = ThreadParams->ThreadIndex;


  if (ThreadParams->AppApi->WorkerInit) { ThreadParams->AppApi->WorkerInit(Global_ThreadStates, ThreadParams->ThreadIndex); }

  while (FutexNotSignaled(ThreadParams->WorkerThreadsExitFutex))
  {
#if 0
    // This is a pointer to a single semaphore for all queues, so only sleeping
    // on one is sufficient, and equal to sleeping on all, because they all
    // point to the same semaphore
    ThreadSleep( ThreadParams->HighPriority->GlobalQueueSemaphore );
#else
    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      /* TIMED_NAMED_BLOCK("CheckForWorkAndSleep"); */

      if (!QueueIsEmpty(ThreadParams->HighPriority)) break;

      if ( ! FutexIsSignaled(ThreadParams->HighPriorityModeFutex) &&
           ! QueueIsEmpty(ThreadParams->LowPriority) ) break;

      if ( FutexIsSignaled(ThreadParams->WorkerThreadsSuspendFutex) ) break;

      if ( FutexIsSignaled(ThreadParams->WorkerThreadsExitFutex) ) break;

      if (WorkerThread_BeforeSleep) WorkerThread_BeforeSleep();

      SleepMs(1);
    }
#endif

    WaitOnFutex(ThreadParams->WorkerThreadsSuspendFutex);

    // NOTE(Jesse): This is here to ensure the game lib (and, by extesion, the debug lib)
    // has ThreadLocal_ThreadIndex set.  This is super annoying and I want a better solution.
    WorkerThread_BeforeJobStart(ThreadParams);
    if (ThreadParams->AppApi->WorkerBeforeJob) { ThreadParams->AppApi->WorkerBeforeJob(Thread, ThreadParams); }

    AtomicIncrement(ThreadParams->HighPriorityWorkerCount);
    DrainQueue( ThreadParams->HighPriority, Thread, ThreadParams->AppApi );
    AtomicDecrement(ThreadParams->HighPriorityWorkerCount);

#if 1
    if ( ! FutexIsSignaled(ThreadParams->HighPriorityModeFutex) )
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

    work_queue* LowPriority = ThreadParams->LowPriority;
    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      if ( ! QueueIsEmpty(ThreadParams->HighPriority)) break;

      if ( FutexIsSignaled(ThreadParams->HighPriorityModeFutex) ) break;

      if ( FutexIsSignaled(ThreadParams->WorkerThreadsExitFutex) ) break;

      if ( FutexIsSignaled(ThreadParams->WorkerThreadsSuspendFutex) ) break;

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
        volatile work_queue_entry *Entry = LowPriority->Entries+DequeueIndex;
        HandleJob(Entry, Thread, ThreadParams->AppApi);
        Ensure( RewindArena(Thread->TempMemory) );
      }
    }
  }

  Info("Exiting Worker Thread (%d)", ThreadParams->ThreadIndex);
  WaitOnFutex(ThreadParams->WorkerThreadsExitFutex);

  return 0;
}

link_weak void
LaunchWorkerThreads(platform *Plat, application_api *AppApi, thread_main_callback_type_buffer *WorkerThreadCallbackProcs)
{
  s32 TotalThreadCount  = (s32)GetTotalThreadCount();

  // This loop is for worker threads; it's skipping thread index 0, the main thread
  for ( s32 ThreadIndex = 1; ThreadIndex < TotalThreadCount; ++ThreadIndex )
  {
    thread_startup_params *Params = &Plat->Threads[ThreadIndex];

    Params->ThreadIndex               = ThreadIndex;
    Params->AppApi                    = AppApi;
    Params->HighPriority              = &Plat->HighPriority;
    Params->LowPriority               = &Plat->LowPriority;
    Params->HighPriorityWorkerCount   = &Plat->HighPriorityWorkerCount;
    Params->HighPriorityModeFutex     = &Plat->HighPriorityModeFutex;
    Params->WorkerThreadsSuspendFutex = &Plat->WorkerThreadsSuspendFutex;
    Params->WorkerThreadsExitFutex    = &Plat->WorkerThreadsExitFutex;
    Params->EngineResources           = GetEngineResources();

    if (WorkerThreadCallbackProcs && umm(ThreadIndex-1) < WorkerThreadCallbackProcs->Count)
    {
      PlatformCreateThread( WorkerThreadCallbackProcs->Start[ThreadIndex-1], (void*)Params, ThreadIndex );
    }
    else
    {
      PlatformCreateThread( DefaultWorkerThread, (void*)Params, ThreadIndex );
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

  Queue->Entries = Allocate(work_queue_entry, Memory, WORK_QUEUE_SIZE);

  InitializeFutex(&Queue->EnqueueFutex);
}

link_weak void
PushWorkQueueEntry(work_queue *Queue, work_queue_entry *Entry)
{
  TIMED_FUNCTION();

  AcquireFutex(&Queue->EnqueueFutex);

  while (QueueIsFull(Queue))
  {
    platform *Plat = &GetEngineResources()->Stdlib.Plat;

    b32 HighPriorityMode = False;
    if (Plat->HighPriorityModeFutex.SignalValue != FUTEX_UNSIGNALLED_VALUE)
    {
      UnsignalFutex(&Plat->HighPriorityModeFutex);
      HighPriorityMode = True;
    }

    Perf("Queue full!");
    SleepMs(1);

    if (HighPriorityMode) { SignalFutex(&Plat->HighPriorityModeFutex); }
  }

  volatile work_queue_entry* Dest = Queue->Entries + Queue->EnqueueIndex;
  Clear(Dest);
  Assert(Dest->Type == type_work_queue_entry_noop);

  MemCopy((u8*)Entry, (u8*)Dest, sizeof(work_queue_entry));
  /* *Dest = *((volatile work_queue_entry*)Entry); */

  Assert(Dest->Type != type_work_queue_entry_noop);

  FullBarrier;

  u32 NewIndex = GetNextQueueIndex(Queue->EnqueueIndex);
  Assert(NewIndex != Queue->DequeueIndex);
  /* DebugLine("%S", CS(NewIndex)); */

  AtomicExchange(&Queue->EnqueueIndex, NewIndex);

  FullBarrier;

  ReleaseFutex(&Queue->EnqueueFutex);

  /* WakeThread( Queue->GlobalQueueSemaphore ); */
}
