
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
    Info("Detected %u Logical cores, creating %u threads", LogicalCoreCount, WorkerThreadCount);

    Plat->Threads = Allocate(thread_startup_params, Plat->Memory, TotalThreadCount);
    Global_ThreadStates = Initialize_ThreadLocal_ThreadStates((s32)GetTotalThreadCount(), Memory);
    Stdlib->ThreadStates = Global_ThreadStates;

    // NOTE(Jesse): This has to be set after Global_ThreadStates is set so we
    // can do GetTranArena during printing
    SetThreadLocal_ThreadIndex(0);
  }

  if (Flags & BonsaiInit_InitDebugSystem)
  {
#if BONSAI_DEBUG_SYSTEM_API
    /* auto DebugSystem = &Stdlib->DebugSystem; */
    /* { */
      /* DebugSystem->Lib = OpenLibrary("./bin/lib_debug_system_loadable" PLATFORM_RUNTIME_LIB_EXTENSION); */
      /* if (!DebugSystem->Lib) { Error("Loading DebugLib :( "); return 1; } */

      /* if (!InitBonsaiDebug(DebugSystem->Lib, &DebugSystem->Api, &DebugSystem->DebugState)) { Error("Initializing Debug Bootstrap Api :( "); return 1; } */
    /* } */

    /* BonsaiDebug_OnLoad(GetDebugState(), Global_ThreadStates, BONSAI_INTERNAL); */
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
