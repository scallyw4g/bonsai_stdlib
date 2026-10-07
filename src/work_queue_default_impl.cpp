
enum work_queue_task_type
{
  type_work_queue_task_async_function_call,
  type_work_queue_task_await,
  // NOTE(Jesse): @custom_task_type
#if 0
  poof(
    for_datatypes(struct) @code_fragment
    func (struct_t)
    {
      struct_t.has_tag(work_queue_task)?
      {
        type_work_queue_task_(struct_t.name),
      }
    }
  )
#include <generated/poof_builtin.for_datatypes$$WP0uwQsK.h>
#endif
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
    work_queue_task_await work_queue_task_await;

  // NOTE(Jesse): @custom_task_type
#if 0
    for_datatypes(struct)
    func (struct_t)
    {
      struct_t.has_tag(async_function_call)?
      {
        struct_t.name struct_t.name,
      }
    }
#endif
  };
};

poof(block_array_h(work_queue_task, {8}, {}))
#include <generated/block_array_h$work_queue_task.688856411.0$Dps6gqjO.h>
poof(block_array_c(work_queue_task, {8}))
#include <generated/block_array_c$work_queue_task.688856411$8iAEZ8gE.h>

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
      { tmatch(work_queue_task_await, WrappedTask, Task)
        InvalidCodePath();
      } break;

      { tmatch(work_queue_task_async_function_call, WrappedTask, Task)
        DispatchAsyncFunctionCall(Task);
      } break;
    }
  }

  MaybeResubmitJob(Job);
}

struct work_queue_job_stats;

enum work_queue_job_state
{
  WorkQueueJobState_Undefined,      // Initial, cleared state.  Should never be hit except during init

  WorkQueueJobState_Free,           // Is on the freelist
  WorkQueueJobState_Reserved,       // Has been reserved by someone intending to submit it
  WorkQueueJobState_Submitted,      // Has been submitted
  /* WorkQueueJobState_Active,      // Has been popped by a worker thread and has a task in-flight */

                                    // TODO(Jesse): Should we actually have this?
                                    //
  WorkQueueJobState_Complete,       // All tasks complete.  This is here mainly for safety..
                                    // there's an assert in StateTransition that there are no remaining tasks
                                    // and we want to know if we're going to await, that we completed
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

  u32 AwaitCount;
  u16 Magic;              // WORK_QUEUE_JOB_MAGIC_NUMBER
  u16 NextTaskIndex;      // Index into Tasks for the next task to Pop

  global_job_index Index; // global index for this job; indexes into platform::Jobs
  work_queue_job_state State;

  u8 Pad[CACHE_LINE_SIZE - 8 - 8 - sizeof(work_queue_task_block_array) -4 - 2 - 2 - sizeof(global_job_index) - 4];
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

    Job->Index.Index = Index;
    Job->Magic = WORK_QUEUE_JOB_MAGIC_NUMBER;
    StateTransition(Job, WorkQueueJobState_Free);

    Job->Tasks.Memory = Plat->TaskMemory;

    Link_TS(Freelist, Cast(freelist_entry *, Job));
  }

  /* Plat->JobStatsTable = Allocate_work_queue_job_stats_hashtable(4096, Plat->TaskMemory); */
}
