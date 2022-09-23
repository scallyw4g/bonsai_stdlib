struct thread_startup_params;
struct platform
{
  work_queue LowPriority;
  work_queue HighPriority;
  semaphore QueueSemaphore;
  volatile u32 WorkerThreadsWaiting;

  thread_startup_params *Threads;

  v2 MouseP;
  v2 MouseDP;

  memory_arena *Memory;

  r32 dt;
  s32 WindowWidth;
  s32 WindowHeight;

  input Input;

  void *GetDebugStateProc;
};

