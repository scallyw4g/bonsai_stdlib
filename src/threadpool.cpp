#include <bonsai_stdlib/src/work_queue_magic.cpp>
#include <bonsai_stdlib/src/work_queue.cpp>




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
struct work_queue_task_await
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

#include <bonsai_stdlib/src/work_queue_default_impl.cpp>
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
    Assert(Job->State < WorkQueueJobState_Submitted);
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
      { tmatch(work_queue_task_await, Task, _)
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
      { tmatch(work_queue_task_await, Task, _)
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
ReserveWorkQueueJob( platform *Plat, u32 AwaitCount, b32 TrackStats /* = False */ )
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

  Result->AwaitCount = AwaitCount;

  ClearList(&Result->Tasks);

  StateTransition(Result, WorkQueueJobState_Reserved);

  Assert(Result->NextTaskIndex == 0);
  Assert(Result->Tasks.ElementCount == 0);
  if (TrackStats)
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
SubmitSingleTask( work_queue *Queue, work_queue_task *Entry, u32 AwaitCount, b32 PerfTrackJob)
{
  TIMED_FUNCTION();

  Assert(Queue);
  // @assert_job_queue
  Assert(Entry->Queue == Queue);

  // TODO(Jesse): Pass in Platform
  work_queue_job *Job = ReserveWorkQueueJob(GetPlatform(), AwaitCount, PerfTrackJob);
  PushTask(Job, Entry);
  SubmitJob(Queue, Job);

  // @await_0_returns_invalid_global_job_index
  global_job_index Result = {};
  if (AwaitCount) { Result = Job->Index; }

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
