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
