#if 1

typedef void (*bonsai_main_thread_callback)              (BONSAI_API_MAIN_THREAD_CALLBACK_PARAMS);
typedef void (*bonsai_worker_thread_init_callback)       (BONSAI_API_WORKER_THREAD_INIT_CALLBACK_PARAMS);
typedef void (*bonsai_worker_thread_before_job_callback) (BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_PARAMS);

typedef bool        (*bonsai_worker_thread_callback)    (BONSAI_API_WORKER_THREAD_CALLBACK_PARAMS);
typedef game_state* (*bonsai_main_thread_init_callback) (BONSAI_API_MAIN_THREAD_INIT_CALLBACK_PARAMS);

typedef b32 (*bonsai_drain_queue_callback)            (engine_resources*, work_queue*, bonsai_futex*);

typedef b32 (*bonsai_engine_callback)            (engine_resources*);
typedef b32 (*bonsai_engine_init_callback)       (engine_resources*, thread_local_state*);

struct application_api
{
  bonsai_main_thread_init_callback GameInit;
  bonsai_main_thread_callback GameMain;
  bonsai_main_thread_callback GameDeInit;

  bonsai_worker_thread_init_callback WorkerInit;
  thread_main_callback_type        DefaultWorkerThread;
  bonsai_worker_thread_before_job_callback WorkerBeforeJob;

  bonsai_main_thread_callback OnLibraryLoad; // Anytime the game library is loaded
};
#endif

link_internal b32
InitializeApplicationApi(application_api *GameApi, shared_lib GameLib)
{
  GameApi->GameInit        = (bonsai_main_thread_init_callback)        GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_MAIN_THREAD_INIT_CALLBACK_NAME));
  GameApi->GameMain        = (bonsai_main_thread_callback)             GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_MAIN_THREAD_CALLBACK_NAME));
  GameApi->GameDeInit      = (bonsai_main_thread_callback)             GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_MAIN_THREAD_DEINIT_CALLBACK_NAME));
  GameApi->WorkerInit      = (bonsai_worker_thread_init_callback)      GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_WORKER_THREAD_INIT_CALLBACK_NAME));
  GameApi->DefaultWorkerThread = (thread_main_callback_type)           GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_WORKER_THREAD_CALLBACK_NAME) );
  GameApi->WorkerBeforeJob = (bonsai_worker_thread_before_job_callback)GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_WORKER_THREAD_BEFORE_JOB_CALLBACK_NAME));
  GameApi->OnLibraryLoad   = (bonsai_main_thread_callback)             GetProcFromLib(GameLib, STRINGIZE(BONSAI_API_ON_GAME_LIB_LOAD_CALLBACK_NAME) );

  b32 Result = GameApi->GameMain            != 0 &&
               GameApi->DefaultWorkerThread != 0;
  return Result;
}

