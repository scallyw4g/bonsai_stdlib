
#define WORK_QUEUE_SIZE (4096*16)

// Note(Jesse): The userland code must define work_queue_entry
struct work_queue_entry;
struct work_queue
{
  bonsai_futex EnqueueFutex;

  volatile u32 EnqueueIndex;
  volatile u32 DequeueIndex;
  volatile work_queue_entry *Entries;
  /* semaphore *GlobalQueueSemaphore; */
};

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

    link_weak             void  PushWorkQueueEntry(work_queue *Queue, work_queue_entry *Entry);
link_internal work_queue_entry*  PopWorkQueueEntry(work_queue* Queue);
link_weak void LaunchWorkerThreads(platform *Plat, application_api *AppApi, thread_main_callback_type_buffer *WorkerThreadCallbacks);

link_weak void WorkerThread_ApplicationDefaultImplementation(BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
link_weak void WorkerThread_BeforeSleep();

link_internal void WorkerThread_BeforeJobStart(thread_startup_params *StartupParams);

BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK()
{
  WorkerThread_BeforeJobStart(StartupParams);
}
