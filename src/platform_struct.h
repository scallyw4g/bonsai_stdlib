struct graphics;
struct server_state;

struct platform
poof(@do_editor_ui)
{
  work_queue LowPriority;
  work_queue HighPriority;

  work_queue LoRenderQ;
  work_queue HiRenderQ;

  volatile u32 HighPriorityWorkerCount;

  bonsai_futex HighPriorityModeFutex;
  bonsai_futex WorkerThreadsSuspendFutex;
  bonsai_futex WorkerThreadsExitFutex;

  // NOTE(Jesse): The work_queues store indices into this array such that the
  // queue doesn't act as the backing store for the jobs.  They just hold a ref
  // and the task itself can decide when it completes if it wants to append
  // another task to the job, or complete it.
  //
  // @work_queue_job_backing_store
  volatile work_queue_job *Jobs;
  volatile work_queue_job *JobsFreelist;

  v2 MouseP;
  v2 MouseDP;

  memory_arena *Memory;

  r32 GameTime;

  r32 dt;

  // TODO(Jesse): Rename this WindowDim, because that's what it is.
  v2 ScreenDim;

  input Input;
  audio Audio;

  server_state *ServerState;
};


