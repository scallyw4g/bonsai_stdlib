#if BONSAI_INTERNAL

#  define Ensure(condition) Assert((condition))



void assert_fail(cs, cs, const char *);

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)
#define LINE_STRING STRINGIZE(__LINE__)

#  define Assert(condition)                                                    \
    do { if (!(condition)) { assert_fail(CSz("Assert(%S) during %s()" __FILE__ ":" LINE_STRING), CSz(#condition), __FUNCTION__); } } while (false)

#  define InvalidCodePath() Error("Invalid Code Path - Panic! " __FILE__ ":" STRINGIZE(__LINE__)); Assert(False)

#  define RuntimeBreak() do {                                                             \
  if (Global_DoRuntimeBreak) {                                                            \
    LogDirect(CSz("\n%S  # Runtime Break #%S\n\n"), TerminalColors.Red, TerminalColors.White); \
    PLATFORM_RUNTIME_BREAK();                                                             \
  } else {                                                                                \
    DebugLine("   Break   - Skipped");                                                    \
  }                                                                                       \
} while (false)

    //PlatformDebugStacktrace();

#define NotImplemented Error("Implement Me! %s:%d ", __FILE__, __LINE__); Assert(False)

#else // Release Build

#define Ensure(condition) condition
#define Assert(condition)  do { if (!(condition)) { PrintToStdout(CSz(" ! FAILED -   " #condition) ); } } while (0)
#define InvalidCodePath(...)
#define RuntimeBreak(...)

#define NotImplemented
/* #define BUG */

/* #define NotImplemented implement_me_plox */
/* #define BUG(...) KnOwN_BuGgY_CoDePaTh_DeTeCtEd */

#endif // BONSAI_INTERNAL


#if BONSAI_DEBUG_SYSTEM_API
#define TriggeredRuntimeBreak(BoolPointer) do { if (*(BoolPointer)) { *(BoolPointer) = False; RuntimeBreak(); } } while (0)

#else

#define TriggeredRuntimeBreak(...)
#endif
