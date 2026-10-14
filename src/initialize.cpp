#if BONSAI_DEBUG_SYSTEM_API
void Platform_EnableContextSwitchTracing();
#else
void Platform_EnableContextSwitchTracing() { SoftError("Context switch tracing not compiled in!"); }
#endif


link_internal b32
InitializeBonsaiStdlib( bonsai_init_flags  Flags,
                          application_api *AppApi,
                            bonsai_stdlib *Stdlib,
                             memory_arena *Memory,
                                     void *ThreadState_UserData      = 0,
         thread_main_callback_type_buffer *WorkerThreadCallbackProcs = 0)
{
  Global_Stdlib = Stdlib;

  GetDt();

  TIMED_FUNCTION();

  Info("Initializing Bonsai");

  UNPACK_STDLIB(Stdlib);

  Stdlib->Plat.Memory = Memory;
  Stdlib->InitFlags = Flags;

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

      s32 TotalJobs = u16_MAX;
      AllocateJobsArray(Plat, TotalJobs);

      Plat->JobStatsTable = Allocate_work_queue_job_stats_hashtable(u32(1024), WorkQueueMemory);

      InitQueue(&Plat->HighPriority, WorkQueueMemory);
      InitQueue(&Plat->LowPriority,  WorkQueueMemory);
      InitQueue(&Plat->HiRenderQ,    WorkQueueMemory);
      InitQueue(&Plat->LoRenderQ,    WorkQueueMemory);
    }
  }
  else
  {
    Stdlib->ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, 1, ThreadState_UserData, Memory);
  }

  // Must come after ThreadStates are valid
  SetThreadLocal_ThreadIndex(0);
  PlatformPinCurrentThreadToCore(0);

  if (Flags & BonsaiInit_InitDebugSystem)
  {
#if BONSAI_DEBUG_SYSTEM_API
    Ensure( InitDebugState(&Stdlib->DebugState) );
    Assert(GetDebugState());
    MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(0.0f, 0);
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

  // TODO(Jesse): Does it ever make sense to request a window without
  // initializing the 2D renderer?
  if (Flags & BonsaiInit_OpenWindow)
  {
    if ((Flags & BonsaiInit_Renderer2D) == False)
    {
      Error("Opening a window requires also initializing the 2D renderer.");
    }
  }

  // NOTE(Jesse): Initializing the renderer requires an opengl context, so we
  // actually have to open a window.  It might make sense to just get rid of
  // the renderer2d flag and make it implicit, but I'm going to leave it like
  // this for now.
  if (Flags & BonsaiInit_Renderer2D)
  {
    if ((Flags & BonsaiInit_OpenWindow) == False)
    {
      Error("Initializing the 2D Renderer requires opening a window.");
    }
  }


  if ( (Flags & BonsaiInit_Renderer2D) && (Flags & BonsaiInit_OpenWindow) )
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

  // Intentionally last such that the render thread has a window to make the render context current on.
  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    if (AppApi)
    {
      if (AppApi->WorkerInit) { AppApi->WorkerInit(GetThreadLocalState(ThreadLocal_ThreadIndex)); }
      SignalFutex(&Plat->WorkerThreadsReady);
      LaunchWorkerThreads(Plat, AppApi, WorkerThreadCallbackProcs);

      WaitForWorkers(&Plat->WorkerThreadsReady);
      UnsignalFutex(&Plat->WorkerThreadsReady);
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
