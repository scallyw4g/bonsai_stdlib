#include <bonsai_stdlib/src/work_queue_magic.cpp>
#include <bonsai_stdlib/src/work_queue.cpp>




poof(
  for_datatypes(func)
  func (func_t)
  {
    func_t.has_tag(async)?
    {
      asyncify_function_h(func_t)
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$W4ZZm5lY.h>

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
struct work_queue_task_async_function_call
{
  async_function_call_type Type;
  union
  {
    compile_shader_pair_async_params compile_shader_pair_async_params;
    output_async_params output_async_params;
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

// @bonsai_stdlib_use_custom_threadpool
/* #if BONSAI_STDLIB_USE_CUSTOM_THREADPOOL */
/* #error "not implemented" */
/* #else */
#include <bonsai_stdlib/src/work_queue_default_impl.cpp>
/* #endif */
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
#include <generated/poof_builtin.for_datatypes$$dwoEEYjr.h>



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
  work_queue_job *Result = StripVolatile(work_queue_job*, Plat->Jobs+GlobalJobIndex.Index);
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
  work_queue_task* Result = GetPtr(&Job->Tasks, Job->NextTaskIndex++);
  return Result;
}

link_internal void
PushTask(work_queue_job *Job, work_queue_task *Task)
{
  Push(&Job->Tasks, Task);
}

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
SubmitSingleTask( work_queue *Queue, work_queue_task *Entry, b32 PerfTrackJob)
{
  TIMED_FUNCTION();

  Assert(Queue);
  // @assert_job_queue
  Assert(Entry->Queue == Queue);

  // TODO(Jesse): Pass in Platform
  work_queue_job *Job = ReserveWorkQueueJob(GetPlatform(), PerfTrackJob);
  PushTask(Job, Entry);
  SubmitJob(Queue, Job);
}



