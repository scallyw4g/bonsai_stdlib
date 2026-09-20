
#define WORK_QUEUE_SIZE (4096)

// Note(Jesse): The userland code must define work_queue_entry and work_queue_entry_block_array
struct work_queue_entry_block_array;
struct work_queue_entry;
struct work_queue_job;

struct global_job_index
{
  u32 Index;
};

struct queue_job_index
{
  u32 Index;
};


struct work_queue
poof(@do_editor_ui)
{
  bonsai_futex EnqueueFutex;

  volatile u32 EnqueueIndex;
  volatile u32 DequeueIndex;

  // @work_queue_job_backing_store
  global_job_index *JobIndices;
};

typedef work_queue* work_queue_ptr;

link_internal u32
GetNextQueueIndex(umm CurrentIndex)
{
  u32 Result = (CurrentIndex+1) % WORK_QUEUE_SIZE;
  return Result;
}

link_internal b32
QueueIsEmpty(work_queue *Queue)
{
  b32 Result = Queue->DequeueIndex == Queue->EnqueueIndex;
  return Result;
}

link_internal b32
QueueIsFull(work_queue *Queue)
{
  u32 NextEnqueueIndex = GetNextQueueIndex(Queue->EnqueueIndex);
  b32 Result = NextEnqueueIndex == Queue->DequeueIndex;
  return Result;
}
// TODO(Jesse): Should any of these actually be link_weak?  I think we should
// probably have a default override that takes over if the user didn't
// implement these, instead of crashing at runtime.  link_weak was always kind
// of a dirty hack to get this working, and we should clean this up at some point
//



link_internal work_queue_job* AllocateWorkQueueJob(platform *Plat);

link_internal work_queue_job* ReserveWorkQueueJob(platform *Plat);
link_internal           void  ReleaseWorkQueueJob(platform *Plat, work_queue_job *Job);

link_internal           void  PushTask ( work_queue_job *Job,   work_queue_entry *Task);

link_internal           void  SubmitJob( work_queue     *Queue, work_queue_job   *Job);
link_internal           void  SubmitJob( work_queue     *Queue, work_queue_entry *Entry);

link_internal work_queue_job* PopNextJob(platform *Plat, work_queue* Queue);

    link_weak           void   LaunchWorkerThreads(platform *Plat, application_api *AppApi, thread_main_callback_type_buffer *WorkerThreadCallbacks);

link_internal void WorkerThread_ApplicationDefaultImplementation(BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
    link_weak void WorkerThread_BeforeSleep();
link_internal void WorkerThread_BeforeJobStart(thread_local_state *StartupParams);

BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK()
{
  WorkerThread_BeforeJobStart(Thread);
}

link_internal global_job_index
GetGlobalJobIndex(work_queue *Queue, queue_job_index QueueIndex);

link_internal work_queue_job *
GetJobFromGlobal(platform *Plat, global_job_index GlobalJobIndex);

link_internal work_queue_job *
GetJobFromQueue(platform *Plat, work_queue *Queue, queue_job_index QueueJobIndex);

link_internal work_queue_entry *
PopNextTaskForNextQueuedJob(platform *Plat, work_queue *Queue, queue_job_index QueueIndex);

link_internal work_queue_entry *
PopNextTask(work_queue_job *Job);

link_internal work_queue_entry *
PeekNextTask(work_queue_job *Job);
