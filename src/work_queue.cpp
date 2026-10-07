
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

  // Signal to main thread we're ready to start
  WaitOnFutex(&Plat->WorkerThreadsReady, True);

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
  Assert(Queue->JobIndices == 0);

  Queue->EnqueueIndex = 0;
  Queue->DequeueIndex = 0;

  Queue->JobIndices = Allocate(global_job_index, Memory, WORK_QUEUE_SIZE);
}


link_internal void
AssertWorkerThreadsSuspended(platform *Plat)
{
  Assert(Plat->WorkerThreadsSuspendFutex.SignalValue != FUTEX_UNSIGNALLED_VALUE);
  Assert(Plat->WorkerThreadsSuspendFutex.ThreadsWaiting == GetWorkerThreadCount());
}




poof(
  for_datatypes(func)
  func (func_t)
  {
    func_t.has_tag(async)?
    {
      asyncify_function_closure_params(func_t)
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$LAtQuQ4R.h>


// Generate tagged_union for async functions
//
enum async_function_call_type
{
  poof(
    for_datatypes(struct) @code_fragment
    func (struct_t)
    {
      struct_t.has_tag(async_function_params)?
      {
        type_(struct_t.name),
      }
    }
  )
#include <generated/poof_builtin.for_datatypes$$hj2VWJGQ.h>
};

#if 1
struct work_queue_task_await_continuation
{
};

struct work_queue_task_async_function_call
{
  async_function_call_type Type;
  union
  {
    poof(
      for_datatypes(struct) @code_fragment
      func (struct_t)
      {
        struct_t.has_tag(async_function_params)?
        {
          struct_t.name struct_t.name;
        }
      }
    )
#include <generated/poof_builtin.for_datatypes$$NiTeiJJT.h>  
  };
};
#else
poof(
  func gen_work_queue_task_async_function_call()
  {
    struct work_queue_task_async_function_call
    {
      async_function_call_type Type;
      union
      {
        for_datatypes(struct) @code_fragment
        func (struct_t)
        {
          struct_t.has_tag(async_function_params)?
          {
            struct_t.name struct_t.name;
          }
        }
      };
    };
  }
)

poof(gen_work_queue_task_async_function_call())
#include <generated/gen_work_queue_task_async_function_call$$rJefEXhD.h>

poof(string_and_value_tables(async_function_call_type))
#include <generated/string_and_value_tables$async_function_call_type$hJOrda0k.h>
#endif



link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job );

link_internal void
DispatchAsyncFunctionCall(work_queue_task_async_function_call *WrappedTask);

enum work_queue_task_type
{
  type_work_queue_task_async_function_call,
  type_work_queue_task_await_continuation,
};

struct work_queue_task
{
  // NOTE(Jesse): This is the queue the job needs to be submitted to to
  // complete this task.
  work_queue_ptr Queue;
  work_queue_task_type Type;

  union
  {
    work_queue_task_async_function_call work_queue_task_async_function_call;
    work_queue_task_await_continuation work_queue_task_await_continuation;
  };
};

poof(block_array_h(work_queue_task, {8}, {}))
#include <generated/block_array_h$work_queue_task.688856411.0$Dps6gqjO.h>
poof(block_array_c(work_queue_task, {8}))
#include <generated/block_array_c$work_queue_task.688856411$8iAEZ8gE.h>

struct work_queue_job_stats;

enum work_queue_job_state
{
  WorkQueueJobState_Undefined,      // Initial, cleared state.  Should never be hit except during init

  WorkQueueJobState_Free,           // Is on the freelist
  WorkQueueJobState_Reserved,       // Has been reserved by someone intending to submit it
  WorkQueueJobState_Submitted,      // Has been submitted
  /* WorkQueueJobState_Active,      // Has been popped by a worker thread and has a task in-flight */

                                    // TODO(Jesse): Should we actually have this?  I think it might
                                    // be more necessary once we do continuations on awaits, but right
                                    // now it's mostly superfluous.
                                    //
  WorkQueueJobState_Complete,       // All tasks complete.  This is here mainly for safety..
                                    // there's an assert in StateTransition that there are no remaining tasks
                                    // and, if we're going to await, we want to know that we completed
                                    //
  WorkQueueJobState_Await,          // Other threads are waiting for the job, do not retire yet
  WorkQueueJobState_AwaitComplete,  // Await thread signalled we can retire
  /* WorkQueueJobState_Retired,        // Retired by the runtime, to eventually be re-reserved */
};

#define WORK_QUEUE_JOB_MAGIC_NUMBER (0x1337)
struct work_queue_job
{
  work_queue_job *Next;   // TODO(Jesse): Pretty sure we don't actually need this..
  work_queue_job_stats *Stats;
  work_queue_task_block_array Tasks;

  u16 NextTaskIndex; // Index into Tasks for the next task to Pop
  u16 Pad;

  volatile u32 AwaitCount;
  global_job_index AwaitContinuationJobId; // If we have awaiters, this job fires when they all hit 0

  global_job_index Index;         // global index for this job; indexes into platform::Jobs
  work_queue_job_state State;
};
CAssert(sizeof(work_queue_job) == CACHE_LINE_SIZE);

link_internal b32
StateTransition(work_queue_job *Job, work_queue_job_state NextState)
{
  // TODO(Jesse): This isn't stricly necessary because if it's not valid, none
  // of the compares will hit,  but it would be nice to have
  /* Assert(IsValid(Job->State)); */

  b32 Result = False;
  switch (NextState)
  {
    InvalidCase(WorkQueueJobState_Undefined);

    case WorkQueueJobState_Free:
    {
      Result = (Job->State == WorkQueueJobState_Undefined    ||
                Job->State == WorkQueueJobState_Complete     ||
                Job->State == WorkQueueJobState_AwaitComplete );
    } break;

    case WorkQueueJobState_Reserved:
    {
      Result = (Job->State == WorkQueueJobState_Free);
      Job->Index.Generation += 1;
    } break;

    case WorkQueueJobState_Submitted:
    {
      Result = (Job->State == WorkQueueJobState_Reserved);
    } break;

    case WorkQueueJobState_Complete:
    {
      Assert(PeekNextTask(Job) == 0);
      Result = (Job->State == WorkQueueJobState_Submitted);
    } break;

    case WorkQueueJobState_Await:
    {
      Result = (Job->State == WorkQueueJobState_Complete);
    } break;

    case WorkQueueJobState_AwaitComplete:
    {
      Result = (Job->State == WorkQueueJobState_Await);
    } break;

    /* case WorkQueueJobState_Retired: */
    /* { */
    /*   Result = (Job->State == WorkQueueJobState_Completed      || */
    /*             Job->State == WorkQueueJobState_AwaitComplete ); */
    /* } break; */

  }

  Assert(Result);
  Job->State = NextState;
  return Result;
}

link_internal void
AllocateJobsArray(platform *Plat, s32 TotalJobs)
{
  Assert(Plat->TaskMemory == 0);
  Assert(Plat->Jobs == 0);
  Assert(Plat->JobCount == 0);

  Plat->TaskMemory = AllocateArena(Megabytes(4));
  Plat->Jobs = Allocate(work_queue_job, Plat->TaskMemory, TotalJobs);

  Plat->JobCount = u32(TotalJobs);
  Plat->FreeJobs  = u32(TotalJobs);

  auto Freelist = Cast(volatile freelist_entry **, &Plat->JobsFreelist);
  RangeIterator_t(u32, Index, u32(TotalJobs))
  {
    work_queue_job *Job = StripVolatile(work_queue_job *, Plat->Jobs+Index);

    Job->Index.Index = SafeTruncateToU16(Index);
    StateTransition(Job, WorkQueueJobState_Free);

    Job->Tasks.Memory = Plat->TaskMemory;

    Link_TS(Freelist, Cast(freelist_entry *, Job));
  }

  /* Plat->JobStatsTable = Allocate_work_queue_job_stats_hashtable(4096, Plat->TaskMemory); */
}

link_internal void
HandleJob(work_queue_job *Job, thread_local_state *Thread, application_api *AppApi)
{
  if ( AppApi->WorkerMain &&
       AppApi->WorkerMain(Job, Thread))
  {
    // App exported a WorkerMain, and it handled the job
  }
  else
  {
        auto *Plat = GetPlatform();
    auto LoRenderQ = &Plat->LoRenderQ;
    auto HiRenderQ = &Plat->HiRenderQ;

    auto WrappedTask = PopNextTask(Job);
    tswitch (WrappedTask)
    {
      { tmatch(work_queue_task_await_continuation, WrappedTask, Task)
        InvalidCodePath();
      } break;

      { tmatch(work_queue_task_async_function_call, WrappedTask, Task)
        DispatchAsyncFunctionCall(Task);
      } break;
    }
  }

  MaybeResubmitJob(Job);
}


poof(
  for_datatypes(struct)
  func (struct_t)
  {
    struct_t.has_tag(async_function_params)?
    {
      struct struct_t.name;
      link_internal work_queue_task
      WorkQueueEntryAsyncFunction( work_queue *Queue, (struct_t.name) *Params )
      {
        work_queue_task Result = {};
        Result.Queue = Queue;
        Result.Type = type_work_queue_task_async_function_call;
        Result.work_queue_task_async_function_call.Type = type_(struct_t.name);
        Result.work_queue_task_async_function_call.(struct_t.name) = *Params;
        return Result;
      }
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$rcup4r0h.h>

poof(
  for_datatypes(func)
  func (func_t)
  {
    func_t.has_tag(async)?
    {
      asyncify_function_c(func_t)
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$WAmjkG0M.h>



link_internal void
DispatchAsyncFunctionCall(work_queue_task_async_function_call *WrappedTask)
{
  tswitch(WrappedTask)
  {
    poof(
      func (async_function_call_type tag_t) @code_fragment
      {
        tag_t.map(tag_v)
        {
          {
            tmatch( tag_v.name.strip_single_prefix, WrappedTask, FuncParams );
            ExecFunction(FuncParams);
          } break;
        }
      }
    )
#include <generated/poof_func.anonymous$async_function_call_type$DwG4hpHD.h>
  }
}

link_internal work_queue_job *
GetJobFromGlobal(platform *Plat, global_job_index GlobalJobIndex)
{
  work_queue_job *Job = StripVolatile(work_queue_job*, Plat->Jobs+GlobalJobIndex.Index);

  work_queue_job *Result = 0;

  // @await_0_returns_invalid_global_job_index
  if (GlobalJobIndex.Generation != 0)
  {
    if (Job->Index.Generation == GlobalJobIndex.Generation)
    {
      Result = Job;
    }
    else
    {
      Error("Requested a job that's been retired and is no longer available!");
    }
  }
  else
  {
    SoftError("@await_0_returns_invalid_global_job_index");
    Error("Requested global_job_index::Generation == 0, which is invalid!  Did you Submit a Job without specifiying a WaitCount?");
  }

  return Result;
}


link_internal void
Await(work_queue_job *Job)
{
  // NOTE(Jesse): It's a bug waiting to happen if you submit a job, then await it.
  // The bug is that the job completes before the submition code hits the await,
  // and the awaiter never gets notified.
  if (Job->AwaitCount == 0)
  {
    Assert(Job->State == WorkQueueJobState_Reserved);
  }
  AtomicIncrement(&Job->AwaitCount);
}

link_internal void
Unawait(platform *Plat, work_queue_job *Job)
{
  AtomicDecrement(&Job->AwaitCount);
  if (Job->AwaitCount == 0)
  {
    StateTransition(Job, WorkQueueJobState_AwaitComplete);
    RetireWorkQueueJob(Plat, Job);
  }
}



link_internal work_queue_task *
PeekNextTask(work_queue_job *Job)
{
  work_queue_task* Result = TryGetPtr(&Job->Tasks, Job->NextTaskIndex);
  return Result;
}

link_internal work_queue_task *
PopNextTask(work_queue_job *Job)
{
  /* StateTransition(Job, WorkQueueJobState_Allocated); */

  work_queue_task* Result = GetPtr(&Job->Tasks, Job->NextTaskIndex++);
  return Result;
}

link_internal void
PushTask(work_queue_job *Job, work_queue_task *Task)
{
  Push(&Job->Tasks, Task);
}

link_internal void
ValidateTaskForQueue(work_queue *Queue, work_queue_task *Task)
{
  platform *Plat = GetPlatform();
  if (Queue == &Plat->LoRenderQ || Queue == &Plat->HiRenderQ)
  {
    tswitch(Task)
    {
      { tmatch(work_queue_task_await_continuation, Task, _)
        InvalidCodePath();
      } break;

      { tmatch(work_queue_task_async_function_call, Task, RPC)
        // nocheckin
        /* Ensure(ValidateRPCForRenderQ(RPC) == True); */
      } break;
    }
  }
  else if (Queue == &Plat->HighPriority || Queue == &Plat->LowPriority)
  {
    tswitch(Task)
    {
      { tmatch(work_queue_task_await_continuation, Task, _)
        InvalidCodePath();
      } break;

      { tmatch(work_queue_task_async_function_call, Task, RPC)
        // nocheckin
        /* Ensure(ValidateRPCForRenderQ(RPC) == False); */
      } break;
    }
  }
  else
  {
    Error("Invalid Queue Pointer (0x%x) passed to ValidateJobForQueue", Queue);
  }

}

// @assert_job_queue
//
link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job )
{
  Assert(Queue);

  TIMED_FUNCTION();

  if (work_queue_task *Task = PeekNextTask(Job))
  {
    {
      ValidateTaskForQueue(Queue, Task);
      Assert(Task->Queue == Queue);
    }

    platform *Plat = GetPlatform();

    StateTransition(Job, WorkQueueJobState_Submitted);

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
  else
  {
    Warn("Attempted to submit job with 0 tasks remaining!");
  }
}

link_internal void
RetireWorkQueueJob(platform *Plat, work_queue_job *Job)
{
  if (Job->Stats)
  {
    Job->Stats->RetireTime = GetCycleCount();
    // nocheckin
    /* Job->Stats->RetireFrameIndex = GetEngineResources()->FrameIndex; */
  }

  Job->NextTaskIndex = 0;
  Job->Stats = 0;

  // TODO(Jesse): This is fucking gnarly .. we should poof a freelist type ..?
  Link_TS(
    Cast(volatile freelist_entry **, &Plat->JobsFreelist),
    Cast(freelist_entry *, Job)
  );

  StateTransition(Job, WorkQueueJobState_Free);
  AtomicIncrement(&Plat->FreeJobs);
}

link_internal work_queue_job *
ReserveWorkQueueJob( platform *Plat, work_queue_job_reserve_flags Flags )
{
  Assert(Plat->Jobs);
  Assert(Plat->JobsFreelist);
  Assert(Plat->FreeJobs > 0);

  AtomicDecrement(&Plat->FreeJobs);

  // TODO(Jesse): This is fucking gnarly .. we should poof a freelist type ..?
  work_queue_job *Result = Cast(work_queue_job*,
                             Unlink_TS(
                               Cast(volatile freelist_entry **, &Plat->JobsFreelist)
                             )
                           );

  ClearList(&Result->Tasks);

  StateTransition(Result, WorkQueueJobState_Reserved);

  if (Flags & WorkQueueJobReserveFlag_Await)
  {
    Await(Result);
  }

  Assert(Result->NextTaskIndex == 0);
  Assert(Result->Tasks.ElementCount == 0);
  if (Flags & WorkQueueJobReserveFlag_TrackPerformance)
  {
    work_queue_job_stats Record = {
      .Job = Result,
      .ReserveTime = GetCycleCount(),
      // nocheckin
      /* .ReserveFrameIndex = GetEngineResources()->FrameIndex, */
      .ReserveFrameIndex = 0,
      .RetireTime = 0,
      .RetireFrameIndex = 0,
    };

    Result->Stats = Insert(Record, &Plat->JobStatsTable, Plat->TaskMemory);
  }
  else
  {
    /* if (work_queue_job_stats *Stats = GetByKey(&Plat->JobStatsTable, Job)) */
    /* { */
    /*   Tombstone( Job, &Plat->JobStatsTable ); */
    /* } */
  }

  Assert(Result);
  return Result;
}


link_internal global_job_index
SubmitSingleTask( work_queue *Queue, work_queue_task *Entry, work_queue_job_reserve_flags Flags)
{
  TIMED_FUNCTION();

  Assert(Queue);
  // @assert_job_queue
  Assert(Entry->Queue == Queue);

  // TODO(Jesse): Pass in Platform
  work_queue_job *Job = ReserveWorkQueueJob(GetPlatform(), Flags);
  PushTask(Job, Entry);
  SubmitJob(Queue, Job);

  // @await_0_returns_invalid_global_job_index
  global_job_index Result = {};
  if (Flags & WorkQueueJobReserveFlag_Await) { Result = Job->Index; }

  return Result;
}

link_internal b32
MaybeResubmitJob(work_queue_job *Job)
{
  b32 Result = False;
  if (work_queue_task *Next = PeekNextTask(Job))
  {
    Result = True;
    SubmitJob(Next->Queue, Job);
  }
  else
  {
    StateTransition(Job, WorkQueueJobState_Complete);

    if (Job->AwaitCount)
    {
      StateTransition(Job, WorkQueueJobState_Await);
    }
    else
    {
      RetireWorkQueueJob(GetPlatform(), Job);
    }
  }
  return Result;
}
