#include <cstdio>

global_variable s32 TestsFailed = 0;
global_variable u32 TestsPassed = 0;

#define TestThat(condition)                                                                                                                  \
  if (!(condition)) {                                                                                                                        \
    ++TestsFailed;                                                                                                                           \
    LogDirect(" %S! Test   %S - %s" Newline, TerminalColors.Red, TerminalColors.White, #condition);                                          \
    LogDirect(" %S! Failed %S - %s() (%s:%u) " Newline Newline, TerminalColors.Red, TerminalColors.White, __FUNCTION__, __FILE__, __LINE__); \
    PlatformDebugStacktrace();                                                                                                               \
    RuntimeBreak();                                                                                                                          \
  } else {                                                                                                                                   \
    ++TestsPassed;                                                                                                                           \
  }

global_variable log_level PrevGlobalLogLevel = LogLevel_Debug;

bonsai_stdlib
TestSuiteBegin(const char *TestSuite, s32 ArgCount, const char** Args)
{
  memory_arena *Memory = AllocateArena();


  /* auto Flags = BonsaiInit_Default; */
  /* auto Flags = BonsaiInit_InitDebugSystem; */

  /* auto Flags = bonsai_init_flags( BonsaiInit_LaunchThreadPool      | */
  /*                                 BonsaiInit_InitDebugSystem       ); */

  auto Flags = bonsai_init_flags( BonsaiInit_OpenWindow             |
                                  BonsaiInit_Renderer2D             |
                                  BonsaiInit_ProfileContextSwitches |
                                  BonsaiInit_LaunchThreadPool       |
                                  BonsaiInit_InitDebugSystem  );

  application_api AppApi = {};
  bonsai_stdlib Stdlib = {};
  Ensure( InitializeBonsaiStdlib( Flags, &AppApi, &Stdlib, Memory) );

  /* Global_ThreadStates = Initialize_ThreadLocal_ThreadStates(1, Memory); */
  /* SetThreadLocal_ThreadIndex(0); */

  /* SetupStdout((u32)ArgCount, Args); */

  LogDirect("%S   Start   %S- %s Tests" Newline, TerminalColors.Blue, TerminalColors.White, TestSuite);

  /* if (!SearchForProjectRoot()) { Error("Couldn't find root dir."); } */

  return Stdlib;
}

void
TestSuiteEnd()
{
  LogDirect("%S   Passed  %S- %u Tests " Newline, TerminalColors.Green, TerminalColors.White, TestsPassed);
  if (TestsFailed) { LogDirect("%S   Failed  %S- %u Tests " Newline, TerminalColors.Red, TerminalColors.White, TestsFailed); }
  LogDirect(Newline);

  auto Stdlib = GetStdlib();
  UNPACK_STDLIB(Stdlib);

  renderer_2d *Ui = &Stdlib->Ui;
  if (Stdlib->InitFlags & BonsaiInit_OpenWindow)
  {
    PlatformMakeRenderContextCurrent(Os);
    while (Os->ContinueRunning)
    {

      // TODO(Jesse): Make a stdlib function that wraps all this stuff up
      // @stdlib_frame_preamble
      ResetInputForFrameStart(&Plat->Input);

      // @stdlib_frame_preamble
      v2 LastMouseP = Plat->MouseP;
      ProcessOsMessages(&Stdlib->Os, &Stdlib->Plat);
      Plat->MouseDP =  Plat->MouseP - LastMouseP;

      // @stdlib_frame_preamble?
      Ui->Input = &Plat->Input;
      Ui->MouseP = &Plat->MouseP;
      Ui->MouseDP = &Plat->MouseDP;
      Ui->ScreenDim = &Plat->ScreenDim;


      UiFrameBegin(Ui);

      // TODO(Jesse): move this!
      // Map GPU buffers for next frame
      MapGpuBuffer(&Ui->SolidQuadGeometryBuffer);
      MapGpuBuffer(&Ui->TextGroup->Buf);


      if (Stdlib->Plat.Input.Escape.Clicked) { break; }

      b32 Toggle = False, Toggle2 = False;
      if (Stdlib->Plat.Input.F1.Clicked) { Info("F1"); Toggle = True; }
      if (Stdlib->Plat.Input.F2.Clicked) { Info("F2"); Toggle2 = True; }

      /* DebugState->DisplayDebugMenu = True; */
      DEBUG_FRAME_BEGIN(Ui, Plat->dt, Toggle, Toggle2);

      PushTableStart(Ui);
      PushTableEnd(Ui);

      DEBUG_FRAME_END(Plat->dt);
      {
        layout DefaultLayout = {};
        render_state RenderState = {};
        RenderState.Layout = &DefaultLayout;

        SetWindowZDepths(Ui->CommandBuffer);
        FlushCommandBuffer(Ui, &RenderState, Ui->CommandBuffer, &DefaultLayout);
      }

      UiFrameEnd(Ui);

      BonsaiSwapBuffers(&Stdlib->Os);

      auto GL = GetGL();

      GL->BindFramebuffer(GL_FRAMEBUFFER, 0);
      GL->ClearColor(0.2f, 0.2f, 0.2f, 1.f);
      GL->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(GetDt()); */
      Ensure(RewindArena(GetTranArena()));
    }
  }

  return;
}
