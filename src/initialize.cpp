void Platform_EnableContextSwitchTracing();

enum bonsai_init_flags
{
  BonsaiInit_Default = 0,

  BonsaiInit_LaunchThreadPool       = (1 << 0),
  BonsaiInit_OpenWindow             = (1 << 1),
  BonsaiInit_InitDebugSystem        = (1 << 2),
  BonsaiInit_ProfileContextSwitches = (1 << 3),
};


link_internal b32
InitializeBonsaiStdlib( bonsai_init_flags  Flags,
                          application_api *AppApi,
                            bonsai_stdlib *Stdlib,
                             memory_arena *Memory,
                                     void *ThreadState_UserData,
         thread_main_callback_type_buffer *WorkerThreadCallbackProcs = 0)
{
  Global_Stdlib = Stdlib;

  Info("Initializing Bonsai");

  UNPACK_STDLIB(Stdlib);

  Stdlib->Plat.Memory = Memory;

  if (Flags & BonsaiInit_LaunchThreadPool)
  {
    u32 LogicalCoreCount  = PlatformGetLogicalCoreCount();
    u32 WorkerThreadCount = GetWorkerThreadCount();
    u32 TotalThreadCount  = GetTotalThreadCount();
    // TODO(Jesse): Move this into the actual creation code?
    Info("Detected (%u) Logical cores, creating (%u) worker threads of (%u) total threads", LogicalCoreCount, WorkerThreadCount, TotalThreadCount);

    Stdlib->ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, s32(TotalThreadCount), ThreadState_UserData, Memory);
  }
  else
  {
    Stdlib->ThreadStates = Initialize_ThreadLocal_ThreadStates(&Stdlib->Plat, 1, ThreadState_UserData, Memory);
  }

  // Must come after ThreadStates are valid
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
    if (AppApi->WorkerInit) { AppApi->WorkerInit(GetThreadLocalState(ThreadLocal_ThreadIndex)); }
    LaunchWorkerThreads(Plat, AppApi, WorkerThreadCallbackProcs);
  }

  if (Flags & BonsaiInit_ProfileContextSwitches)
  {
    Platform_EnableContextSwitchTracing();
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

link_internal void
BonsaiFrameBegin(bonsai_stdlib *Stdlib, renderer_2d *Ui)
{
  UNPACK_STDLIB(Stdlib);

  Plat->dt = GetDt();

  auto GL = GetGL();
  GL->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GL->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ResetInputForFrameStart(&Plat->Input, 0);

  v2 LastMouseP = Plat->MouseP;
  ProcessOsMessages(&Stdlib->Os, &Stdlib->Plat);
  Plat->MouseDP = Plat->MouseP - LastMouseP;

  UiFrameBegin(Ui);

  DEBUG_FRAME_BEGIN(Ui, Plat->dt, Plat->Input.F1.Clicked, Plat->Input.F2.Clicked);
}

link_internal void
BonsaiFrameEnd(bonsai_stdlib *Stdlib, renderer_2d *Ui)
{
  UNPACK_STDLIB(Stdlib);

  {
    layout DefaultLayout = {};
    render_state RenderState = {};
    RenderState.Layout = &DefaultLayout;

    SetWindowZDepths(Ui->CommandBuffer);
    FlushCommandBuffer(Ui, &RenderState, Ui->CommandBuffer, &DefaultLayout);
    UiFrameEnd(Ui);
  }

  BonsaiSwapBuffers(&Stdlib->Os);
  DEBUG_FRAME_END(Plat->dt);
  MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(Plat->dt);
}
