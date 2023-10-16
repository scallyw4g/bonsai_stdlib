
#define BONSAI_API_MAIN_THREAD_CALLBACK_NAME          MainThreadCallback
#define BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS        engine_resources *Resources, thread_local_state *MainThread

#define BONSAI_API_MAIN_THREAD_INIT_CALLBACK_NAME     InitMainThreadCallback
#define BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS   engine_resources *Resources, thread_local_state *MainThread

#define BONSAI_API_WORKER_THREAD_CALLBACK_NAME        WorkerThreadCallback
#define BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS      volatile work_queue_entry* Entry, thread_local_state* Thread

#define BONSAI_API_WORKER_THREAD_INIT_CALLBACK_NAME   InitWorkerThreadCallback
#define BONSAI_API_WORKER_THREAD_INIT_CALLBACK_PARAMS thread_local_state* AllThreads, s32 ThreadIndex

#define BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_NAME     OnGameLibLoad
#define BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_PARAMS   BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS



#define BONSAI_API_MAIN_THREAD_CALLBACK() \
  link_export void BONSAI_API_MAIN_THREAD_CALLBACK_NAME(BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS)

#define BONSAI_API_MAIN_THREAD_INIT_CALLBACK() \
  link_export game_state* BONSAI_API_MAIN_THREAD_INIT_CALLBACK_NAME(BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS)

#define BONSAI_API_WORKER_THREAD_CALLBACK() \
  link_export bool BONSAI_API_WORKER_THREAD_CALLBACK_NAME(BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS)

#define BONSAI_API_WORKER_THREAD_INIT_CALLBACK() \
  link_export void BONSAI_API_WORKER_THREAD_INIT_CALLBACK_NAME(BONSAI_API_WORKER_THREAD_INIT_CALLBACK_PARAMS)

#define BONSAI_API_ON_LIBRARY_RELOAD() \
  link_export void BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_NAME(BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_PARAMS)



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

struct platform;
struct game_state;
struct engine_resources;
struct thread_local_state;
struct thread_startup_params;

typedef void (*bonsai_main_thread_callback)        (BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS);
typedef void (*bonsai_worker_thread_init_callback) (BONSAI_API_WORKER_THREAD_INIT_CALLBACK_PARAMS);

typedef bool        (*bonsai_worker_thread_callback)    (BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
typedef game_state* (*bonsai_main_thread_init_callback) (BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS);

typedef b32 (*bonsai_engine_callback)(engine_resources*);
typedef b32 (*bonsai_engine_init_callback)(engine_resources*, thread_startup_params*);

struct application_api
{
  bonsai_main_thread_init_callback GameInit;
  bonsai_main_thread_callback GameMain;

  bonsai_worker_thread_init_callback WorkerInit;
  bonsai_worker_thread_callback WorkerMain;

  bonsai_main_thread_callback OnLibraryLoad; // Anytime the game library is loaded
};


struct thread_startup_params
{
  application_api *AppApi;

  volatile u32 *HighPriorityWorkerCount;

  bonsai_futex *HighPriorityModeFutex;
  bonsai_futex *WorkerThreadsSuspendFutex;
  bonsai_futex *WorkerThreadsExitFutex;

  work_queue *LowPriority;
  work_queue *HighPriority;

  volatile s32 ThreadIndex;
};


link_weak void PushWorkQueueEntry(work_queue *Queue, work_queue_entry *Entry);
link_weak void WorkerThread_ApplicationDefaultImplementation(BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
link_weak void LaunchWorkerThreads(platform *Plat, application_api *AppApi);

link_internal void WorkerThread_BeforeJobStart(thread_startup_params *StartupParams);

