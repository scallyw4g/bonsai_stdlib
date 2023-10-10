
link_internal void
PlatformInit(platform *Plat, memory_arena *Memory)
{
  Plat->Memory = Memory;

  u32 LogicalCoreCount = PlatformGetLogicalCoreCount();
  u32 WorkerThreadCount = GetWorkerThreadCount();
  s32 TotalThreadCount  = (s32)GetTotalThreadCount();
  Info("Detected %u Logical cores, creating %u threads", LogicalCoreCount, WorkerThreadCount);

  Plat->Threads = Allocate(thread_startup_params, Plat->Memory, TotalThreadCount);

#if BONSAI_NETWORK_IMPLEMENTATION
  Plat->ServerState = ServerInit(GameMemory);
#endif

  return;
}

enum bonsai_init_flags
{
  BonsaiInit_Default = 0,

  BonsaiInit_LaunchThreadPool = (1 << 0),
  BonsaiInit_OpenWindow       = (1 << 1),
  BonsaiInit_InitDebugSystem  = (1 << 2),
};


link_internal b32
InitializeBonsaiStdlib( bonsai_init_flags Flags,
                        application_api  *AppApi,
                        bonsai_stdlib    *Stdlib,
                        memory_arena *Memory )
{
  Info("Initializing Bonsai");

  UNPACK_STDLIB(Stdlib);

  SetThreadLocal_ThreadIndex(0);

  PlatformInit(&Stdlib->Plat, Memory);

  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    Global_ThreadStates = Initialize_ThreadLocal_ThreadStates((s32)GetTotalThreadCount(), Memory);
    Stdlib->ThreadStates = Global_ThreadStates;
  }

  if (Flags & BonsaiInit_InitDebugSystem)
  {
#if DEBUG_SYSTEM_API && DEBUG_SYSTEM_LOADER_API
    auto DebugSystem = &Stdlib->DebugSystem;
    {
      DebugSystem->Lib = OpenLibrary("./bin/lib_debug_system_loadable" PLATFORM_RUNTIME_LIB_EXTENSION);
      if (!DebugSystem->Lib) { Error("Loading DebugLib :( "); return 1; }

      if (!InitializeBootstrapDebugApi(DebugSystem->Lib, &DebugSystem->Api)) { Error("Initializing Debug Bootstrap Api :( "); return 1; }
    }

    DebugSystem->Api.BonsaiDebug_OnLoad(GetDebugState(), Global_ThreadStates, BONSAI_INTERNAL);
    Ensure( DebugSystem->Api.InitDebugState(Global_DebugStatePointer) );
#else
    Error("Asked to init debug system when DEBUG_SYSTEM_API was not compiled in!");
#endif
  }


  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    if (AppApi->WorkerInit) { AppApi->WorkerInit (Global_ThreadStates, 0); }
    LaunchWorkerThreads(Plat, AppApi);
  }

  if (Flags & BonsaiInit_OpenWindow)
  {
    s32 VSyncFrames = 0;
    if (!OpenAndInitializeWindow(Os, Plat, VSyncFrames)) { Error("Initializing Window :( "); return False; }
    Assert(Os->GlContext);

    Ensure( InitializeOpenglFunctions() );
  }

  return True;
}
