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


  auto Flags = bonsai_init_flags( BonsaiInit_OpenWindow            |
                                  BonsaiInit_LaunchThreadPool      |
                                  BonsaiInit_InitDebugSystem       );

  bonsai_stdlib Stdlib = {};
  Ensure( InitializeBonsaiStdlib( Flags, 0, &Stdlib, Memory) );

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

  return;
}
