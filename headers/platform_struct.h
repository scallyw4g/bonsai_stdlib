struct thread_startup_params;
struct graphics;
struct server_state;
struct platform
{
  work_queue LowPriority;
  work_queue HighPriority;

  volatile u32 HighPriorityWorkerCount;

  bonsai_futex WorkerThreadsSuspendFutex;
  bonsai_futex WorkerThreadsExitFutex;

  thread_startup_params *Threads;

  v2 MouseP;
  v2 MouseDP;

  memory_arena *Memory;

  r32 dt;
  s32 WindowWidth;
  s32 WindowHeight;

  input Input;

  void *GetDebugStateProc;

  graphics *Graphics;
  os *Os;

  server_state *ServerState;
};

