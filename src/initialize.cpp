#if BONSAI_DEBUG_SYSTEM_API
void Platform_EnableContextSwitchTracing();
#else
void Platform_EnableContextSwitchTracing() { SoftError("Context switch tracing not compiled in!"); }
#endif

enum bonsai_init_flags
{
  BonsaiInit_Default = 0,

  BonsaiInit_LaunchThreadPool       = (1 << 0),
  BonsaiInit_OpenWindow             = (1 << 1),
  BonsaiInit_Renderer2D             = (1 << 2),
  BonsaiInit_InitDebugSystem        = (1 << 3),
  BonsaiInit_ProfileContextSwitches = (1 << 4),
  BonsaiInit_Audio                  = (1 << 5),
};


link_internal b32
InitializeBonsaiStdlib( bonsai_init_flags  Flags,
                          application_api *AppApi,
                            bonsai_stdlib *Stdlib,
                             memory_arena *Memory,
                                     void *ThreadState_UserData      = 0,
         thread_main_callback_type_buffer *WorkerThreadCallbackProcs = 0)
{
#if BONSAI_STDLIB_NO_THREADPOOL
  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    Error("Threadpool initialization requested with BONSAI_STDLIB_NO_THREADPOOL");
    return False;
  }
#endif

  Global_Stdlib = Stdlib;

  Info("Initializing Bonsai");

  UNPACK_STDLIB(Stdlib);

  Stdlib->Plat.Memory = Memory;

#if !BONSAI_STDLIB_NO_THREADPOOL
  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    TIMED_NAMED_BLOCK(LaunchThreadPool);
    u32 LogicalCoreCount  = PlatformGetLogicalCoreCount();
    u32 WorkerThreadCount = GetWorkerThreadCount();
    u32 TotalThreadCount  = GetTotalThreadCount();
    // TODO(Jesse): Move this into the actual creation code?
    Info("Detected (%u) Logical cores, creating (%u) worker threads of (%u) total threads", LogicalCoreCount, WorkerThreadCount, TotalThreadCount);

    Stdlib->ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, s32(TotalThreadCount), ThreadState_UserData, Memory);

    {
      memory_arena *WorkQueueMemory = AllocateArena();

      s32 TotalJobs = WORK_QUEUE_SIZE*6;
      AllocateJobsArray(Plat, TotalJobs);

      Plat->JobStatsTable = Allocate_work_queue_job_stats_hashtable(u32(TotalJobs), WorkQueueMemory);

      InitQueue(&Plat->HighPriority, WorkQueueMemory);
      InitQueue(&Plat->LowPriority,  WorkQueueMemory);
      InitQueue(&Plat->HiRenderQ,    WorkQueueMemory);
      InitQueue(&Plat->LoRenderQ,    WorkQueueMemory);
    }
  }
  else
#endif
  {
    Stdlib->ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, 1, ThreadState_UserData, Memory);
  }

  // Must come after ThreadStates are valid
  SetThreadLocal_ThreadIndex(0);
#if BONSAI_WIN32
  PlatformPinCurrentThreadToCore(0);
#endif

  if (Flags & BonsaiInit_InitDebugSystem)
  {
#if BONSAI_DEBUG_SYSTEM_API
    Ensure( InitDebugState(&Stdlib->DebugState) );
    Assert(GetDebugState());
    MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(0.0f);
#else
    Error("Asked to init debug system when BONSAI_DEBUG_SYSTEM_API was not compiled in!");
#endif
  }

  if (Flags & BonsaiInit_OpenWindow)
  {
#if PLATFORM_WINDOW_IMPLEMENTATIONS
    s32 VSyncFrames = 1;
    if (!OpenAndInitializeWindow(Os, Plat, VSyncFrames)) { Error("Initializing Window :( "); return False; }

    PlatformMakeRenderContextCurrent(Os);
    Ensure( InitializeOpenglFunctions() );
    /* SetVSync(Os, VSyncFrames); */
    PlatformReleaseRenderContext(Os);
#else
    Error("Asked to open a window when window implementations were not compiled in!");
#endif
  }

  if (Flags & BonsaiInit_Renderer2D)
  {
    if (Flags & BonsaiInit_OpenWindow)
    {
      // TODO(Jesse): Make this configurable?
      heap_allocator RendererHeap = InitHeap(Gigabytes(1), False);
      renderer_2d *Ui = &Stdlib->Ui;

#if BONSAI_DEBUG_SYSTEM_API
      SetDebugRenderer(Ui);
#endif

      v2 MouseP, MouseDP, ScreenDim;

      PlatformMakeRenderContextCurrent(&Stdlib->Os);
      InitRenderer2D(Ui, &RendererHeap, Memory, &Stdlib->Plat.MouseP, &Stdlib->Plat.MouseDP, &Stdlib->Plat.ScreenDim, &Stdlib->Plat.Input);
      PlatformReleaseRenderContext(Os);
    }
    else
    {
      Error("Unable to do BonsaiInit_Renderer2D without BonsaiInit_OpenWindow.");
    }
  }

  // Intentionally last such that the render thread has a window to make the render context current on.
  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    if (AppApi)
    {
      if (AppApi->WorkerInit) { AppApi->WorkerInit(GetThreadLocalState(ThreadLocal_ThreadIndex)); }
      LaunchWorkerThreads(Plat, AppApi, WorkerThreadCallbackProcs);
    }
    else
    {
      Error("Asked to launch worker threads when AppApi wasn't initialized!");
    }
  }

  if (Flags & BonsaiInit_ProfileContextSwitches)
  {
    Platform_EnableContextSwitchTracing();
  }

  if (Flags & BonsaiInit_Audio)
  {
    PlatformInitializeAudio(Plat);
  }

  return True;
}

#if PLATFORM_WINDOW_IMPLEMENTATIONS
link_internal void
OpenAndInitializeWindow(u32 VSyncFrames = 0)
{
  auto Stdlib = GetStdlib();
  OpenAndInitializeWindow(&Stdlib->Os, &Stdlib->Plat, 0);
}
#endif

#if 0
link_internal void
BonsaiFrameBegin(bonsai_stdlib *Stdlib, renderer_2d *Ui, b32 DebugToggleMenu, b32 DebugToggleProfile)
{
  UNPACK_STDLIB(Stdlib);

  Plat->dt = GetDt();

  auto GL = GetGL();
  GL->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GL->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ResetInputForFrameStart(&Plat->Input);

  v2 LastMouseP = Plat->MouseP;
  ProcessOsMessages(&Stdlib->Os, &Stdlib->Plat);
  Plat->MouseDP = Plat->MouseP - LastMouseP;

  UiFrameBegin(Ui);


  DEBUG_FRAME_BEGIN(Ui, Plat->dt, DebugToggleMenu, DebugToggleProfile);
}

link_internal void
BonsaiFrameEnd(bonsai_stdlib *Stdlib, renderer_2d *Ui)
{
  UNPACK_STDLIB(Stdlib);

  // TODO(Jesse)(yikes): the following produces flickering artifacts when moved
  // inside UiFrameEnd.. why??
#if 1
  {
    layout DefaultLayout = {};
    render_state RenderState = {};
    RenderState.Layout = &DefaultLayout;

    SetWindowZDepths(Ui->CommandBuffer);
    FlushCommandBuffer(Ui, &RenderState, Ui->CommandBuffer, &DefaultLayout);
  }
#endif

  UiFrameEnd(Ui);

  BonsaiSwapBuffers(&Stdlib->Os);
  DEBUG_FRAME_END(Plat->dt);
  MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(Plat->dt);
}
#endif
