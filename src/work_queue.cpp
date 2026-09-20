link_internal global_job_index
GetGlobalJobIndex(work_queue *Queue, queue_job_index QueueIndex)
{
  global_job_index Result = Queue->JobIndices[QueueIndex.Index];
  return Result;
}

link_internal work_queue_job *
GetJobFromGlobal(platform *Plat, global_job_index GlobalJobIndex)
{
  work_queue_job *Result = StripVolatile(work_queue_job*, Plat->Jobs+GlobalJobIndex.Index);
  return Result;
}

link_internal work_queue_job *
GetJobFromQueue(platform *Plat, work_queue *Queue, queue_job_index QueueJobIndex)
{
  work_queue_job *Result = GetJobFromGlobal(Plat, GetGlobalJobIndex(Queue, QueueJobIndex));
  return Result;
}

link_internal work_queue_entry *
PeekNextTask(work_queue_job *Job)
{
  work_queue_entry* Result = TryGetPtr(&Job->Tasks, Job->NextTaskIndex);
  return Result;
}

link_internal work_queue_entry *
PopNextTask(work_queue_job *Job)
{
  work_queue_entry* Result = GetPtr(&Job->Tasks, Job->NextTaskIndex++);
  return Result;
}

link_internal work_queue_entry *
PopNextTaskForNextQueuedJob(platform *Plat, work_queue *Queue, queue_job_index QueueIndex)
{
  global_job_index GlobalJobIndex = GetGlobalJobIndex(Queue, QueueIndex);
  work_queue_job *Job = GetJobFromGlobal(Plat, GlobalJobIndex);

  work_queue_entry *Result = PopNextTask(Job);
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

  auto Engine =  GetEngineResources();
  auto Stdlib =  GetStdlib();
  auto Plat   = &Stdlib->Plat;
  work_queue* LowPriority = &Plat->LowPriority;
  work_queue* HighPriority = &Plat->HighPriority;

  auto WorkerThreadsExitFutex    = &Plat->WorkerThreadsExitFutex;
  auto WorkerThreadsSuspendFutex = &Plat->WorkerThreadsSuspendFutex;
  auto HighPriorityModeFutex     = &Plat->HighPriorityModeFutex;
  auto HighPriorityWorkerCount   = &Plat->HighPriorityWorkerCount;

  if (Stdlib->AppApi.WorkerInit) { Stdlib->AppApi.WorkerInit(GetThreadLocalState(ThreadLocal_ThreadIndex)); }

  WaitOnFutex(&Engine->ReadyToStartMainLoop, True);

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
    GetStdlib()->AppApi.WorkerBeforeJob(Thread);

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

link_internal void
ReleaseWorkQueueJob(platform *Plat, work_queue_job *Job)
{
  Plat->FreeJobs = Plat->FreeJobs +1;

  // TODO(Jesse): This is fucking gnarly .. we should poof a freelist type ..?
  Link_TS(
    Cast(volatile freelist_entry **, &Plat->JobsFreelist),
    Cast(freelist_entry *, Job)
  );
}

link_internal work_queue_job *
ReserveWorkQueueJob(platform *Plat)
{
  Plat->FreeJobs = Plat->FreeJobs -1;

  // TODO(Jesse): This is fucking gnarly .. we should poof a freelist type ..?
  work_queue_job *Result = Cast(work_queue_job*,
                             Unlink_TS(
                               Cast(volatile freelist_entry **, &Plat->JobsFreelist)
                             )
                           );
  Assert(Result);
  return Result;
}

link_internal void
PushTask(work_queue_job *Job, work_queue_task *Task)
{
  Push(&Job->Tasks, Task);
}

// TODO(Jesse): We should actually just check which queue the current task
// wants to be submitted to, but this is compatible with the current API, so
// we'll explicitly take a queue and assert when we pop the job instead.
//
// @assert_job_queue
//
link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job )
{
  Assert(Queue);

  TIMED_FUNCTION();

  platform *Plat = GetPlatform();

  AcquireFutex(&Queue->EnqueueFutex);

  while (QueueIsFull(Queue))
  {
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

  FullBarrier;


  Queue->JobIndices[Queue->EnqueueIndex] = Job->Index;

  u32 NewIndex = GetNextQueueIndex(Queue->EnqueueIndex);
  Assert(NewIndex != Queue->DequeueIndex); // QueueIsFull check
  AtomicExchange(&Queue->EnqueueIndex, NewIndex);

  FullBarrier;

  ReleaseFutex(&Queue->EnqueueFutex);
}

link_internal void
SubmitSingleTask( work_queue *Queue, work_queue_entry *Entry )
{
  TIMED_FUNCTION();

  Assert(Queue);
  // @assert_job_queue
  Assert(Entry->Queue == Queue);

  // TODO(Jesse): Pass in Platform
  work_queue_job *Job = ReserveWorkQueueJob(GetPlatform());
  PushTask(Job, Entry);
  SubmitJob(Queue, Job);
}
