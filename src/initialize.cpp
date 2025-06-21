
enum bonsai_init_flags
{
  BonsaiInit_Default = 0,

  BonsaiInit_LaunchThreadPool = (1 << 0),
  BonsaiInit_OpenWindow       = (1 << 1),
  BonsaiInit_InitDebugSystem  = (1 << 2),
};


link_internal b32
InitializeBonsaiStdlib( bonsai_init_flags  Flags,
                          application_api *AppApi,
                            bonsai_stdlib *Stdlib,
                             memory_arena *Memory,
                                     void *ThreadState_UserData,
         thread_main_callback_type_buffer *WorkerThreadCallbackProcs = 0)
{
  Info("Initializing Bonsai");

  UNPACK_STDLIB(Stdlib);

  Stdlib->Plat.Memory = Memory;

  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    u32 LogicalCoreCount  = PlatformGetLogicalCoreCount();
    u32 WorkerThreadCount = GetWorkerThreadCount();
    u32 TotalThreadCount  = GetTotalThreadCount();
    Info("Detected (%u) Logical cores, creating (%u) worker threads of (%u) total threads", LogicalCoreCount, WorkerThreadCount, TotalThreadCount);

    Global_ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, AppApi, s32(TotalThreadCount), ThreadState_UserData, Memory);
    Stdlib->ThreadStates = Global_ThreadStates;
  }
  else
  {
    Global_ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, AppApi, 1, ThreadState_UserData, Memory);
  }

  // NOTE(Jesse): This has to be set after Global_ThreadStates is set so we
  // can do GetTranArena during printing
  SetThreadLocal_ThreadIndex(0);

  if (Flags & BonsaiInit_InitDebugSystem)
  {
#if BONSAI_DEBUG_SYSTEM_API
    Ensure( InitDebugState(&Stdlib->DebugState) );
    MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(0.0f);
#else
    Error("Asked to init debug system when BONSAI_DEBUG_SYSTEM_API was not compiled in!");
#endif
  }

  if (Flags & BonsaiInit_OpenWindow)
  {
#if PLATFORM_WINDOW_IMPLEMENTATIONS
    s32 VSyncFrames = 0;
    if (!OpenAndInitializeWindow(Os, Plat, VSyncFrames)) { Error("Initializing Window :( "); return False; }

    PlatformMakeRenderContextCurrent(Os);
    Ensure( InitializeOpenglFunctions() );
    PlatformReleaseRenderContext(Os);
#else
    Error("Asked to open a window when window implementations were not compiled in!");
#endif
  }

  // Intentionally last such that the render thread has a window to make the render context current on.
  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    if (AppApi->WorkerInit) { AppApi->WorkerInit(Global_ThreadStates, 0); }
    LaunchWorkerThreads(Plat, AppApi, WorkerThreadCallbackProcs);
  }

  return True;
}
