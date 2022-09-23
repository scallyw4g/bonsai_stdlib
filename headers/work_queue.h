// TODO(Jesse, id: 93, tags: over_allocation): overkill .. not ..?
#define WORK_QUEUE_SIZE (65536)

// Note(Jesse): The userland code must define work_queue_entry
struct work_queue_entry;
struct work_queue
{
  volatile u32 EnqueueIndex;
  volatile u32 DequeueIndex;
  volatile work_queue_entry *Entries;
  semaphore *GlobalQueueSemaphore;
};

#if 1
b32
QueueIsEmpty(work_queue* Queue)
{
  b32 Result = Queue->DequeueIndex == Queue->EnqueueIndex;
  return Result;
}
#endif
