/* #ifdef BONSAI_STDLIB_USE_CUSTOM_THREADPOOL */
/* #error "Included work_queue_default_impl.h when BONSAI_STDLIB_USE_CUSTOM_THREADPOOL was defined" */
/* #endif */

enum work_queue_task_type
{
  type_work_queue_task_async_function_call,
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
  };
};

poof(block_array_h(work_queue_task, {8}, {}))
#include <generated/block_array_h$work_queue_task.688856411.0$Dps6gqjO.h>
poof(block_array_c(work_queue_task, {8}))
#include <generated/block_array_c$work_queue_task.688856411$8iAEZ8gE.h>

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
    ReleaseWorkQueueJob(GetPlatform(), Job);
  }
  return Result;
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
      { tmatch(work_queue_task_async_function_call, WrappedTask, Task)
        DispatchAsyncFunctionCall(Task);
      } break;
    }
  }

  MaybeResubmitJob(Job);
}

// TODO(Jesse): Do alignment and padding for cache lines
#define WORK_QUEUE_JOB_MAGIC_NUMBER (0x1337)
struct work_queue_job
{
  work_queue_job *Next;
  work_queue_task_block_array Tasks;

  u16 Magic;              // WORK_QUEUE_JOB_MAGIC_NUMBER
  u16 NextTaskIndex;      // Index into Tasks for the next task to Pop

  global_job_index Index; // global index for this job; indexes into platform::Jobs
};

link_internal void
AllocateJobsArray(platform *Plat, s32 TotalJobs)
{
  Assert(Plat->TaskMemory == 0);

  Plat->TaskMemory = AllocateArena(Megabytes(4));
  Plat->Jobs = Allocate(work_queue_job, Plat->TaskMemory, TotalJobs);

  Plat->TotalJobs = u32(TotalJobs);
  Plat->FreeJobs  = u32(TotalJobs);

  auto Freelist = Cast(volatile freelist_entry **, &Plat->JobsFreelist);
  RangeIterator_t(u32, Index, u32(TotalJobs))
  {
    work_queue_job *Job = StripVolatile(work_queue_job *, Plat->Jobs+Index);

    Job->Index.Index = Index;
    Job->Magic = WORK_QUEUE_JOB_MAGIC_NUMBER;

    Job->Tasks.Memory = Plat->TaskMemory;

    Link_TS(Freelist, Cast(freelist_entry *, Job));
  }

  /* Plat->JobStatsTable = Allocate_work_queue_job_stats_hashtable(4096, Plat->TaskMemory); */
}
