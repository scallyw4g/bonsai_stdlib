
poof(block_array(global_job_index, {64}))
#include <generated/block_array$global_job_index.688853971$6LIdaKhl.h>

link_internal void
AwaitContinuation( global_job_index_block_array AwaitJobIds );


poof(hashtable_impl(work_queue_job_stats))
#include <generated/hashtable_impl$work_queue_job_stats$5cwEEtEf.h>

poof(hashtable_get_by_key(work_queue_job_stats))
#include <generated/hashtable_get_by_key$work_queue_job_stats$H3A23qAm.h>

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

enum work_queue_task_type
{
  type_work_queue_task_async_function_call,
  type_work_queue_task_await_continuation,
};

// NOTE(Jesse): At this point, I'm nearly 100% certain async_function_call
// is all you need, but I'm not so confident yet that I'm going to delete it..
//
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

struct work_queue_job
{
  work_queue_job_stats *Stats; // NOTE(Jesse): Next pointer in linked list overwrites this
  work_queue_task_block_array Tasks;

  // nocheckin debug
  s32 _OwningThreadId; // = INVALID_THREAD_LOCAL_THREAD_INDEX; Set in AllocateJobsArray
  u32 Pad_;

  u32 NextTaskIndex; // Index into Tasks for the next task to Pop

  u32 Pad__;
  global_job_index JoinContinuationJobId; // If we have awaiters, this job fires when they all hit 0

  global_job_index Index;         // global index for this job; indexes into platform::Jobs
  work_queue_job_state State;

  // cache line boundary
  volatile u32 JoinCount;
  u8 pad___[60];

  // cache line boundary
  volatile u32 AwaitCount;
  u8 pad____[60];
};
CAssert(sizeof(work_queue_job) == CACHE_LINE_SIZE*3);

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
      /* Assert(Job->OwningThreadId == INVALID_THREAD_LOCAL_THREAD_INDEX); Job->OwningThreadId = ThreadLocal_ThreadIndex; */

      Result = (Job->State == WorkQueueJobState_Free);
      Job->Index.Generation += 1;
    } break;

    case WorkQueueJobState_Submitted:
    {
      /* Assert(Job->OwningThreadId == ThreadLocal_ThreadIndex); Job->OwningThreadId = INVALID_THREAD_LOCAL_THREAD_INDEX; */

      Result = (Job->State == WorkQueueJobState_Reserved);
    } break;

    case WorkQueueJobState_Complete:
    {
      /* Assert(Job->OwningThreadId == ThreadLocal_ThreadIndex); Job->OwningThreadId = INVALID_THREAD_LOCAL_THREAD_INDEX; */

      Assert(PeekNextTask(Job) == 0);
      Result = (Job->State == WorkQueueJobState_Submitted);
    } break;

    case WorkQueueJobState_Await:
    {
      /* Assert(Job->OwningThreadId == INVALID_THREAD_LOCAL_THREAD_INDEX); */

      Result = (Job->State == WorkQueueJobState_Complete);
    } break;

    case WorkQueueJobState_AwaitComplete:
    {
      /* Assert(Job->OwningThreadId == INVALID_THREAD_LOCAL_THREAD_INDEX); */

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
OverwriteQueueJob(work_queue *Queue, queue_job_index Index)
{
  Queue->JobIndices[Index.Index] = {};
}


link_internal global_job_index
GetGlobalJobIndex(work_queue *Queue, queue_job_index QueueIndex)
{
  global_job_index Result = Queue->JobIndices[QueueIndex.Index];
  return Result;
}

link_internal work_queue_job *
PopNextQueuedJob(platform *Plat, work_queue *Queue, queue_job_index QueueJobIndex)
{
  auto GlobalIndex = GetGlobalJobIndex(Queue, QueueJobIndex);
  work_queue_job *Result = GetJobFromGlobal(Plat, GlobalIndex);
  OverwriteQueueJob(Queue, QueueJobIndex);
  return Result;
}

link_internal work_queue_task *
PopNextTaskForNextQueuedJob(platform *Plat, work_queue *Queue, queue_job_index QueueIndex)
{
  auto Job = PopNextQueuedJob(Plat, Queue, QueueIndex);
  work_queue_task *Result = PopNextTask(Job);
  return Result;
}

link_internal work_queue_job *
PopNextJob(platform *Plat, work_queue* Queue, u32 CompareExchangeRetryCount /* = u32_MAX */)
{
  TIMED_FUNCTION();

  work_queue_job *Result = {};

  AcquireFutex(&Queue->DequeueFutex);

  // NOTE(Jesse): These are just here for debugging .. they should probably go
  // back in the loop ..?
  u32 DequeueIndex;
  u32 NextIndex;
  RangeIterator_t(u32, RetryIndex, CompareExchangeRetryCount)
  {
    /* WORKER_THREAD_ADVANCE_DEBUG_SYSTEM(); */

    // NOTE(Jesse): Must read and comared DequeueIndex instead of calling QueueIsEmpty
    DequeueIndex = Queue->DequeueIndex;
    if (DequeueIndex == Queue->EnqueueIndex)
    {
      break;
    }

    NextIndex = GetNextQueueIndex(DequeueIndex);
    b32 Exchanged = AtomicCompareExchange( &Queue->DequeueIndex,
                                           NextIndex,
                                           DequeueIndex );
    Assert ( Exchanged );
    {
      Result = PopNextQueuedJob(Plat, Queue, {DequeueIndex});
      break;
    }
  }

  if (Result)
  {
    Assert(Result->State == WorkQueueJobState_Submitted);
  }

  ReleaseFutex(&Queue->DequeueFutex);

  return Result;
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
  WaitOnFutex(&Plat->WorkerThreadsReady);

  while (FutexNotSignaled(WorkerThreadsExitFutex))
  {
    // Ready-Wait loop for work to come in 
    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      if (!QueueIsEmpty(HighPriority)) break;

      if ( ! FutexIsSignaled(HighPriorityModeFutex) &&
           ! QueueIsEmpty(LowPriority) ) break;

      if ( FutexIsSignaled(WorkerThreadsSuspendFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsExitFutex) ) break;

      if (WorkerThread_BeforeSleep) WorkerThread_BeforeSleep();

      SleepMs(1);
    }

    WaitOnFutex(WorkerThreadsSuspendFutex);

    // NOTE(Jesse): This is here to ensure the game lib (and, by extesion, the debug lib)
    // has ThreadLocal_ThreadIndex set.  This is super annoying and I want a better solution.
    WorkerThread_BeforeJobStart(Thread);
    if (Stdlib->AppApi.WorkerBeforeJob) { Stdlib->AppApi.WorkerBeforeJob(Thread); }

    // Drain hi-priority queue
    {
      AtomicIncrement(HighPriorityWorkerCount);

      /* DrainQueue(Plat, HighPriority, Thread, &GetStdlib()->AppApi ); */
      while (auto Job = PopNextJob(Plat, HighPriority))
      {
        // TODO(Jesse): Seems like we should call this here?
        //
        // WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

        HandleJob( Job, Thread, &GetStdlib()->AppApi );

        if ( FutexIsSignaled(WorkerThreadsExitFutex) ) break;

        if ( FutexIsSignaled(WorkerThreadsSuspendFutex) ) break;
      }

      AtomicDecrement(HighPriorityWorkerCount);
    }

    // Drain lo-priority queue until we get preempted by hi-priority, or a
    // request to suspend/exit
    for (;;)
    {
      WORKER_THREAD_ADVANCE_DEBUG_SYSTEM();

      if ( ! QueueIsEmpty(HighPriority)) break;

      if ( FutexIsSignaled(HighPriorityModeFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsExitFutex) ) break;

      if ( FutexIsSignaled(WorkerThreadsSuspendFutex) ) break;

      if (work_queue_job *Job = PopNextJob(Plat, LowPriority, 1))
      {
        HandleJob(Job, Thread, &GetStdlib()->AppApi);
        Ensure( RewindArena(Thread->TempMemory) ); // TODO(Jesse): Do we want this here ..?
      }

    }

    if ( ! FutexIsSignaled(HighPriorityModeFutex) )
    {
      Ensure( RewindArena(Thread->TempMemory) );

      // Can't do this because the debug system needs a static handle to the base
      // address of the arena, which VaporizeArena unmaps
      //
      /* Ensure( VaporizeArena(Thread.TempMemory) ); */
      /* Ensure( Thread.TempMemory = AllocateArena() ); */
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




link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job );

link_internal void
DispatchAsyncFunctionCall(work_queue_task_async_function_call *WrappedTask);

link_internal void
AllocateJobsArray(platform *Plat, s32 TotalJobs)
{
  Assert(Plat->TaskMemory == 0);
  Assert(Plat->Jobs == 0);
  Assert(Plat->JobCount == 0);

  Plat->TaskMemory = AllocateArena(Megabytes(4));

  // Ensure alignment to cache lines so we don't get false sharing, or tearing
  // on reads straddling the boudnary
  Plat->Jobs = AllocateAligned(work_queue_job, Plat->TaskMemory, TotalJobs, CACHE_LINE_SIZE);

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
  Assert(Job->State == WorkQueueJobState_Submitted);

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
poof(@async)
AwaitContinuation( global_job_index_block_array AwaitJobIds )
{
  Info("yaaaaaaaaaay");
}

link_internal void
Join(work_queue_job *Job)
{
  // NOTE(Jesse): It's a bug waiting to happen if you submit a job, then await it.
  // The bug is that the job completes before the submition code hits the await,
  // and the awaiter never gets notified.
  //
  // @await_join_reserved_job_bug
  if (Job->JoinCount == 0)
  {
    Assert(Job->State == WorkQueueJobState_Reserved);
  }
  AtomicIncrement(&Job->JoinCount);
}

link_internal void
OnComplete(work_queue_job *Job, work_queue_job *Continuation)
{
  // It's an error to attach an OnComplete to a job that's been submitted
  // because the job could have already run to completion, in which case it
  // won't reach out to the continuation and decrement it's await counter
  //
  // Similar story with the Continuation, except it'll execute early
  Assert(Job->State == WorkQueueJobState_Reserved);
  Assert(Continuation->State == WorkQueueJobState_Reserved);

  Join(Continuation);
  /* Await(Continuation); */
  Job->JoinContinuationJobId = Continuation->Index;
}

link_internal void
Await(work_queue_job *Job)
{
  // NOTE(Jesse): It's a bug waiting to happen if you submit a job, then await it.
  // The bug is that the job completes before the submition code hits the await,
  // and the awaiter never gets notified.
  //
  // @await_join_reserved_job_bug
  if (Job->AwaitCount == 0)
  {
    Assert(Job->State == WorkQueueJobState_Reserved);
  }
  AtomicIncrement(&Job->AwaitCount);
}

link_internal u32
UnawaitAndRetire(platform *Plat, work_queue_job *Job)
{
  Assert(Job->State == WorkQueueJobState_Await);

  u32 Result = AtomicDecrement(&Job->AwaitCount);
  if (Result == 0)
  {
    StateTransition(Job, WorkQueueJobState_AwaitComplete);
    /* Assert(Job->OwningThreadId == INVALID_THREAD_LOCAL_THREAD_INDEX); */
    /* Job->OwningThreadId = ThreadLocal_ThreadIndex; */
    RetireWorkQueueJob(Plat, Job);
  }
  return Result;
}

link_internal u32
JoinAndMaybeSubmit(work_queue_job *Job)
{
  u32 Result = AtomicDecrement(&Job->JoinCount);

  // NOTE(Jesse): It is valid for the reserver to submit a job, so we have to check
  // for that case
  Assert(
      Job->State == WorkQueueJobState_Reserved  ||
      Job->State == WorkQueueJobState_Submitted ||
      Job->State == WorkQueueJobState_Await     );

  if (Result == 0)
  {
    Assert(Job->State == WorkQueueJobState_Reserved);
    SubmitJob(Job);
  }
  return Result;
}
link_internal u32
UnawaitAndSubmit(work_queue_job *Job)
{
  u32 Result = AtomicDecrement(&Job->AwaitCount);

  // NOTE(Jesse): It is valid for the reserver to submit a job, so we have to check
  // for that case
  Assert(
      Job->State == WorkQueueJobState_Reserved  ||
      Job->State == WorkQueueJobState_Submitted ||
      Job->State == WorkQueueJobState_Await     );

  if (Result == 0)
  {
    Assert(Job->State == WorkQueueJobState_Reserved);
    SubmitJob(Job);
  }
  return Result;
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
  Assert(Job->NextTaskIndex == 0);
  work_queue_task* Result = GetPtr(&Job->Tasks, Job->NextTaskIndex);
  AtomicIncrement(&Job->NextTaskIndex); // I'm almost certain we don't need to atomically increment this
  return Result;
}

link_internal void
PushTask(work_queue_job *Job, work_queue_task *Task)
{
  Assert(Job->NextTaskIndex == 0); // nocheckin
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

// TODO(Jesse): This was just to conform with the old task-only API and can be
// safely removed
link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job )
{
  Assert(Job->NextTaskIndex == 0); // nocheckin
  auto *Task = PeekNextTask(Job);
  Assert(Task);
  Assert(Task->Queue == Queue);
  SubmitJob(Job);
}

// @assert_job_queue
//
link_internal void
SubmitJob( work_queue_job *Job )
{
  TIMED_FUNCTION();

  /* Assert(Job->OwningThreadId == ThreadLocal_ThreadIndex); */
  /* Job->OwningThreadId = INVALID_THREAD_LOCAL_THREAD_INDEX; */

  Assert(Job->NextTaskIndex == 0);

  if (work_queue_task *Task = PeekNextTask(Job))
  {
    auto Queue = Task->Queue;

    // Debug
    ValidateTaskForQueue(Queue, Task);

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

      /* Perf("Queue full!"); */
      SpinlockNs(10);
      /* SleepMs(1); */

      if (HighPriorityMode) { SignalFutex(&Plat->HighPriorityModeFutex); }
    }

    FullBarrier;

    u32 EnqueueIndex = Queue->EnqueueIndex;
    Queue->JobIndices[EnqueueIndex] = Job->Index;

    u32 NewIndex = GetNextQueueIndex(EnqueueIndex);
    Assert(NewIndex != Queue->DequeueIndex); // QueueIsFull check
    Ensure( AtomicCompareExchange(&Queue->EnqueueIndex, NewIndex, EnqueueIndex) );

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
  Assert(Job->JoinCount == 0);
  Assert(Job->AwaitCount == 0);

  if (Job->Stats)
  {
    Job->Stats->RetireTime = GetCycleCount();
    // nocheckin
    /* Job->Stats->RetireFrameIndex = GetEngineResources()->FrameIndex; */
  }

  Job->NextTaskIndex = 0;
  Job->Stats = 0;

  /* Assert(Job->OwningThreadId == ThreadLocal_ThreadIndex); */
  /* Job->OwningThreadId = INVALID_THREAD_LOCAL_THREAD_INDEX; */

  StateTransition(Job, WorkQueueJobState_Free);

  // TODO(Jesse): This is fucking gnarly .. we should poof a freelist type ..?
  Link_TS(
    Cast(volatile freelist_entry **, &Plat->JobsFreelist),
    Cast(freelist_entry *, Job)
  );

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

  /* Assert(Result->OwningThreadId == INVALID_THREAD_LOCAL_THREAD_INDEX); */
  /* Result->OwningThreadId = ThreadLocal_ThreadIndex; */

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

  // pass this in
  platform *Plat = GetPlatform();

  // We've got a new task, resubmit
  if (work_queue_task *Next = PeekNextTask(Job))
  {
    Assert(False); // nocheckin
    Result = True;
    SubmitJob(Next->Queue, Job);
  }
  else
  {
    // Complete the task
    // Retire if not awaited
    // Also check for an AwaitContinuation, and fire it off if we're the last waiter
    //
    StateTransition(Job, WorkQueueJobState_Complete);


    if (IsValid(Job->JoinContinuationJobId))
    {
      work_queue_job *Continuation = GetJobFromGlobal(Plat, Job->JoinContinuationJobId);
      JoinAndMaybeSubmit(Continuation);
    }

    FullBarrier;


    if (Job->AwaitCount)
    {
      // NOTE(Jesse): Must come after the next job gets submitted because of a
      // race condition. The bug, if we transition to await before we dispatch
      // the next job:
      //
      // 1. 
      /* Assert(Job->OwningThreadId == ThreadLocal_ThreadIndex); */
      /* Job->OwningThreadId = INVALID_THREAD_LOCAL_THREAD_INDEX; */
      StateTransition(Job, WorkQueueJobState_Await);
    }
    else
    {
      RetireWorkQueueJob(Plat, Job);
    }
  }
  return Result;
}
