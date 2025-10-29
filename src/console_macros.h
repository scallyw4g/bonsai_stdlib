global_variable const umm TempStdoutFormatStringBufferSize = 64*1024;
global_variable char TempStdoutFormatStringBuffer[TempStdoutFormatStringBufferSize];

link_internal void
PrintToStdout(counted_string S);

link_internal void
SetupStdout(u32 ArgCount, const char** ArgStrings);

/* link_internal counted_string */
/* FormatCountedString_(char* Dest, umm DestSize, const char *FS, ...); */

#define CSz(NullTerminatedCString) \
  CS(NullTerminatedCString, sizeof(NullTerminatedCString)-1)

inline counted_string CS(cs *Cs) { return *Cs; }
inline counted_string CS(cs Cs) { return Cs; }

inline counted_string
CS(const char *S, umm Count)
{
  counted_string Result = {
    .Count = Count, .Start = S
  };
  return Result;
}


enum log_level
{
  LogLevel_Undefined,

  LogLevel_Verbose,
  LogLevel_Debug,
  LogLevel_Info,
  LogLevel_Error,
  LogLevel_Shush,
};

struct terminal_colors
{
  counted_string Red          = CSz("\x1b[31m");
  counted_string BrightRed    = CSz("\x1b[91m");
  counted_string DarkRed      = CSz("\x1b[38;5;88m");

  counted_string Blue         = CSz("\x1b[34m");
  counted_string BrightBlue   = CSz("\x1b[94m");
  counted_string Purple       = CSz("\x1b[35m");
  counted_string BrightPurple = CSz("\x1b[95m");
  counted_string Green        = CSz("\x1b[32m");
  counted_string BrightGreen  = CSz("\x1b[92m");
  counted_string Yellow       = CSz("\x1b[33m");
  counted_string BrightYellow = CSz("\x1b[93m");

  counted_string White        = CSz("\x1b[37m");
  counted_string Grey         = CSz("\x1b[38;5;242m");
};

global_variable terminal_colors TerminalColors = {};

#if 0
// TODO(Jesse): Metaprogram this?  ATM we only ever turn colors off
link_internal void
SetTerminalColorsOn()
{
  NotImplemented;
  /* TerminalColors.BrightRed    = CSz("\x1b[31m"); */
  /* TerminalColors.BrightBlue   = CSz("\x1b[34m"); */
  /* TerminalColors.BrightPurple = CSz("\x1b[35m"); */
  /* TerminalColors.BrightGreen  = CSz("\x1b[32m"); */
  TerminalColors.Red             = CSz("\x1b[31m");
  TerminalColors.Blue            = CSz("\x1b[34m");
  TerminalColors.Purple          = CSz("\x1b[35m");
  TerminalColors.Green           = CSz("\x1b[32m");
  TerminalColors.Yellow          = CSz("\x1b[33m");
  TerminalColors.BrightYellow    = CSz("\x1b[93m");
  TerminalColors.White           = CSz("\x1b[37m");
}
#endif

// TODO(Jesse): Metaprogram this
link_internal void
SetTerminalColorsOff()
{
#if 1
  TerminalColors.Red          = CSz("");
  TerminalColors.BrightRed    = CSz("");
  TerminalColors.DarkRed      = CSz("");

  TerminalColors.BrightBlue   = CSz("");
  TerminalColors.Blue         = CSz("");
  TerminalColors.Purple       = CSz("");
  TerminalColors.BrightPurple = CSz("");
  TerminalColors.Green        = CSz("");
  TerminalColors.BrightGreen  = CSz("");
  TerminalColors.Yellow       = CSz("");
  TerminalColors.BrightYellow = CSz("");
  TerminalColors.White        = CSz("");
  TerminalColors.Grey         = CSz("");
#endif
}

struct memory_arena;
link_internal memory_arena* GetTranArena();

link_internal counted_string
FormatCountedString_(char *Dest, umm DestSize, cs Fmt, ...);

link_internal char * GetTempFmtBuffer();

#define InvalidCase(C) case C: {Error("Invalid case value (" #C ") " __FILE__ ":" STRINGIZE(__LINE__));} break
#define InvalidDefaultCase default: {Error("Invalid case value (default) " __FILE__ ":" STRINGIZE(__LINE__));} break

#define DEFAULT_FILE_IDENTIFIER  __FILE__ ":" STRINGIZE(__LINE__)

#define DEBUG_CONSOLE_LOG_CALLING_LOCATION (0)
#if DEBUG_CONSOLE_LOG_CALLING_LOCATION
#define MaybeLogLocation() PrintToStdout(FormatCountedString_(GetTempFmtBuffer(), TempStdoutFormatStringBufferSize, CSz("%s:%d\n\n"), __FILE__, __LINE__))
#else
#define MaybeLogLocation()
#endif

#define LogDirect(...) \
  PrintToStdout(FormatCountedString_(GetTempFmtBuffer(), TempStdoutFormatStringBufferSize, ##__VA_ARGS__)); \
  MaybeLogLocation()


#define DebugChars(fmt, ...) do {               \
                                                \
  if (Global_LogLevel <= LogLevel_Debug) {      \
    LogDirect(CSz(fmt), ##__VA_ARGS__);         \
  }                                             \
                                                \
} while (false)


#define DebugLine(fmt, ...) do {                \
                                                \
  if (Global_LogLevel <= LogLevel_Debug) {      \
    LogDirect(CSz(fmt Newline), ##__VA_ARGS__); \
  }                                             \
                                                \
} while (false)


#define DebugMessage(fmt, ...) do {                                                                            \
                                                                                                               \
  if (Global_LogLevel <= LogLevel_Debug) {                                                                     \
    LogDirect(CSz("%S   Debug   %S- " fmt Newline), TerminalColors.Blue, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                            \
                                                                                                               \
} while (false)



#define Info(fmt, ...) do {                                                                                    \
  if (Global_LogLevel <= LogLevel_Info) {                                                                      \
    LogDirect(CSz("%S   Info    %S- " fmt Newline), TerminalColors.Blue, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                            \
} while (false)

#define RenderInfo(fmt, ...) do {                                                                                    \
  if (Global_LogLevel <= LogLevel_Info) {                                                                      \
    LogDirect(CSz("%S   Render  %S- " fmt Newline), TerminalColors.BrightPurple, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                            \
} while (false)




#define SoftError(fmt, ...) do {                                                                              \
                                                                                                              \
  if (Global_LogLevel <= LogLevel_Error) {                                                                    \
    LogDirect(CSz("%S ! Error   %S- " fmt Newline), TerminalColors.Red, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                           \
                                                                                                              \
} while (false)

#define Error(fmt, ...) do {                       \
                                                   \
  if (Global_LogLevel <= LogLevel_Error) {         \
    SoftError(fmt, ##__VA_ARGS__); RuntimeBreak(); \
  }                                                \
                                                   \
} while (false)


#define Leak(fmt, ...) do {                                                                                      \
                                                                                                                 \
  if (Global_LogLevel <= LogLevel_Error) {                                                                       \
    LogDirect(CSz("%S * Leaking %S- " fmt Newline), TerminalColors.Yellow, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                              \
                                                                                                                 \
} while (false)


#define BUG(fmt, ...) do {                                                                                    \
                                                                                                              \
  if (Global_LogLevel <= LogLevel_Error) {                                                                    \
    LogDirect(CSz("%S * BUG     %S- " fmt Newline), TerminalColors.Red, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                           \
                                                                                                              \
} while (false)


#define Shader(fmt, ...) do {                                                                                      \
                                                                                                                 \
  if (Global_LogLevel <= LogLevel_Info) {                                                                        \
    LogDirect(CSz("%S   Shader  %S- " fmt Newline), TerminalColors.Purple, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                              \
                                                                                                                 \
} while (false)

#define Perf(fmt, ...) do {                                                                                      \
                                                                                                                 \
  if (Global_LogLevel <= LogLevel_Info) {                                                                        \
    LogDirect(CSz("%S   Perf    %S- " fmt Newline), TerminalColors.Yellow, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                              \
                                                                                                                 \
} while (false)

#define Warn(fmt, ...) do {                                                                                      \
                                                                                                                 \
  if (Global_LogLevel <= LogLevel_Info) {                                                                        \
    LogDirect(CSz("%S * Warning %S- " fmt Newline), TerminalColors.Yellow, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                              \
                                                                                                                 \
} while (false)

#define OpenGlDebugMessage(fmt, ...) do {                                                                                     \
                                                                                                                              \
  if (Global_LogLevel <= LogLevel_Debug) {                                                                                    \
    LogDirect(CSz("%S * OpenGl Debug Message %S- " fmt Newline), TerminalColors.Yellow, TerminalColors.White, ##__VA_ARGS__); \
  }                                                                                                                           \
                                                                                                                              \
} while (false)

#define Deprecated() do {                                                                                                   \
                                                                                                                            \
  if (Global_LogLevel <= LogLevel_Info) {                                                                                   \
    LogDirect(CSz("%S * Function (%s) Deprecated %S " Newline), TerminalColors.Yellow, __FUNCTION__, TerminalColors.White); \
  }                                                                                                                         \
                                                                                                                            \
} while (false)

#define Asset(fmt, ...) do {                                                                                   \
                                                                                                               \
  if (Global_LogLevel <= LogLevel_Info) {                                                                      \
    LogDirect(CSz("   Asset   - " fmt Newline), ##__VA_ARGS__); \
  }                                                                                                            \
                                                                                                               \
} while (false)

#define TODO(fmt, ...) do {                                                                                   \
                                                                                                               \
  if (Global_LogLevel <= LogLevel_Info) {                                                                      \
    LogDirect(CSz("    TODO   - " fmt Newline), ##__VA_ARGS__); \
  }                                                                                                            \
                                                                                                               \
} while (false)


#define IndentMessage(fmt, ...) do {                                                                                   \
                                                                                                                \
  if (Global_LogLevel <= LogLevel_Info) {                                                                       \
    LogDirect(CSz("           - " fmt Newline), ##__VA_ARGS__);                                                 \
  }                                                                                                             \
                                                                                                                \
} while (false)
