/* #ifdef BONSAI_STDLIB_USE_CUSTOM_THREADPOOL */
/* #error "Included work_queue_default_impl.h when BONSAI_STDLIB_USE_CUSTOM_THREADPOOL was defined" */
/* #endif */

enum work_queue_task_type
{
  type_work_queue_task_async_function_call
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
#include <generated/block_array_h$work_queue_task.688856411.0$beKxfoBA.h>
poof(block_array_c(work_queue_task, {8}))
#include <generated/block_array_c$work_queue_task.688856411$d5DK8Z9q.h>

link_internal void
HandleJob(work_queue_job *Job, thread_local_state *Thread, application_api *AppApi)
{
  NotImplemented;
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

