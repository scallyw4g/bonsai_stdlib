struct graphics;
struct server_state;

struct platform
{
  work_queue LowPriority;
  work_queue HighPriority;
  work_queue RenderQ;
  work_queue WorldUpdateQ;

  volatile u32 HighPriorityWorkerCount;

  bonsai_futex HighPriorityModeFutex;
  bonsai_futex WorkerThreadsSuspendFutex;
  bonsai_futex WorkerThreadsExitFutex;

  v2 MouseP;
  v2 MouseDP;

  memory_arena *Memory;

  r32 GameTime;

  r32 dt;

  // TODO(Jesse): Rename this WindowDim, because that's what it is.
  v2 ScreenDim;

  input Input;

  server_state *ServerState;
};


