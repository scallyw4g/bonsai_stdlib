#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);


#define INVALID_THREAD_LOCAL_THREAD_INDEX (-1)
#define FUTEX_UNSIGNALLED_VALUE (u32_MAX) // The signal value is the thread holding the futex so we have to use the max

#define FUTEX_INITIALIZE(F) if ((F)->Initialized == False) { InitializeFutex(F); }

struct bonsai_futex
{
  volatile u32 SignalValue = FUTEX_UNSIGNALLED_VALUE;
  volatile u32 ThreadsWaiting;
  b32 Initialized;
};


link_internal void WaitOnFutex(bonsai_futex *Futex, b32 DoSleep = True);
link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void WaitForWorkerThreads(volatile u32 *);
link_internal void SignalAndWaitForWorkers(bonsai_futex *);



global_variable thread_local s32
ThreadLocal_ThreadIndex = INVALID_THREAD_LOCAL_THREAD_INDEX;


link_internal void
InitializeFutex(bonsai_futex *Futex)
{
  Futex->SignalValue = FUTEX_UNSIGNALLED_VALUE;
  Futex->Initialized = True;
}

link_internal b32
UnsignalFutex(bonsai_futex *Futex, s32 SignallingThread)
{
  FUTEX_INITIALIZE(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  Assert(Futex->SignalValue == u32(SignallingThread));
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, FUTEX_UNSIGNALLED_VALUE, u32(SignallingThread));
  return Result;
}

link_internal b32
UnsignalFutex(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  Assert(Futex->SignalValue == (u32)ThreadLocal_ThreadIndex);
  b32 Result = UnsignalFutex(Futex, ThreadLocal_ThreadIndex);
  return Result;
}

link_internal b32
SignalFutex(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  Assert(ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX);
  b32 Result = AtomicCompareExchange(&Futex->SignalValue, (u32)ThreadLocal_ThreadIndex, FUTEX_UNSIGNALLED_VALUE);
  return Result;
}

link_internal b32
FutexNotSignaled(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  b32 Result = Futex->SignalValue == FUTEX_UNSIGNALLED_VALUE;
  return Result;
}

link_internal b32
FutexIsSignaled(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  b32 Result = Futex->SignalValue != FUTEX_UNSIGNALLED_VALUE;
  return Result;
}

link_internal void
AcquireFutex(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  while (SignalFutex(Futex) == False)
  {
    WaitOnFutex(Futex, False);
  }
  /* printf("(%d) acquired %p\n", ThreadLocal_ThreadIndex, Futex); */
}

link_internal void
ReleaseFutex(bonsai_futex *Futex)
{
  FUTEX_INITIALIZE(Futex);
  Ensure( UnsignalFutex(Futex) );
  /* printf("(%d) released %p\n", ThreadLocal_ThreadIndex, Futex); */
}







#define BONSAI_API_MAIN_THREAD_CALLBACK_NAME          MainThreadCallback
#define BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS        engine_resources *Resources, thread_local_state *MainThread

#define BONSAI_API_MAIN_THREAD_INIT_CALLBACK_NAME     InitMainThreadCallback
#define BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS   engine_resources *Resources, thread_local_state *MainThread

#define BONSAI_API_WORKER_THREAD_CALLBACK_NAME        WorkerThreadCallback
#define BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS      volatile work_queue_entry* Entry, thread_local_state* Thread

#define BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_NAME      WorkerThreadBeforeJobCallback
#define BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_PARAMS    thread_local_state* Thread, thread_startup_params *StartupParams

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

#define BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK() \
  link_export void BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_NAME(BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_PARAMS)


#define BONSAI_API_ON_LIBRARY_RELOAD() \
  link_export void BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_NAME(BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_PARAMS)



struct platform;
struct game_state;
struct engine_resources;
struct thread_local_state;
struct thread_startup_params;

struct work_queue;
struct work_queue_entry;

typedef void (*bonsai_main_thread_callback)              (BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS);
typedef void (*bonsai_worker_thread_init_callback)       (BONSAI_API_WORKER_THREAD_INIT_CALLBACK_PARAMS);
typedef void (*bonsai_worker_thread_before_job_callback) (BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_PARAMS);

typedef bool        (*bonsai_worker_thread_callback)    (BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
typedef game_state* (*bonsai_main_thread_init_callback) (BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS);

typedef b32 (*bonsai_engine_callback)            (engine_resources*);
typedef b32 (*bonsai_engine_init_callback)       (engine_resources*, thread_startup_params*);

struct application_api
{
  bonsai_main_thread_init_callback GameInit;
  bonsai_main_thread_callback GameMain;

  bonsai_worker_thread_init_callback WorkerInit;
  bonsai_worker_thread_before_job_callback WorkerBeforeJob;
  bonsai_worker_thread_callback WorkerMain;

  bonsai_main_thread_callback OnLibraryLoad; // Anytime the game library is loaded
};

struct bonsai_stdlib;

struct thread_startup_params
{
  bonsai_stdlib   *Stdlib;

  volatile u32 *HighPriorityWorkerCount;

  bonsai_futex *HighPriorityModeFutex;
  bonsai_futex *WorkerThreadsSuspendFutex;
  bonsai_futex *WorkerThreadsExitFutex;

  work_queue *LowPriority;
  work_queue *HighPriority;

  volatile s32 ThreadIndex;
};

struct thread_main_callback_type_buffer;

struct memory_arena;

struct thread_local_state
{
  thread_startup_params StartupParams;

  memory_arena *PermMemory;
  memory_arena *TempMemory;

  char *TempStdoutFormatStringBuffer;

  s32 Index;
  s32 Pad[5]; poof(@ui_skip)

  void *UserData;
};
// TODO(Jesse): Reenable this nopush
/* CAssert( (sizeof(thread_local_state) % CACHE_LINE_SIZE) == 0 ); */

// TODO(Jesse): Remove this asap.
global_variable
thread_local_state *Global_ThreadStates;

link_internal void
SetThreadLocal_ThreadIndex(s32 Index)
{
  Assert(ThreadLocal_ThreadIndex == -1);
  ThreadLocal_ThreadIndex = Index;

  /* printf("Setting ThreadLocal_ThreadIndex (%d)\n", Index); */
}

link_internal thread_local_state *
GetThreadLocalState(s32 ThreadIndex)
{
  Assert(ThreadIndex >= 0);
  Assert(ThreadIndex < (s32)GetTotalThreadCount());

  Assert(Global_ThreadStates);

  return Global_ThreadStates + ThreadIndex;
}

link_internal memory_arena*
GetTranArena()
{
  thread_local_state *Thread = GetThreadLocalState(ThreadLocal_ThreadIndex);
  return Thread->TempMemory;
}
