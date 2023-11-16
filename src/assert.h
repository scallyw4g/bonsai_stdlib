#define CAssert(condition) static_assert((condition), #condition )
#define OffsetOf(member_name, type_name) offsetof(type_name, member_name)

#if BONSAI_INTERNAL

#  define Ensure(condition) Assert((condition))

#  define Assert(condition)                                                    \
    do {                                                                       \
      if (!(condition)) {                                                      \
        LogDirect(CSz("%S ! FAILED%S  - `Assert(%s)` during %s() %s:%u:0" Newline), \
                  TerminalColors.Red,                                          \
                  TerminalColors.White,                                        \
                  #condition,                                                  \
                  __FUNCTION__,                                                \
                  __FILE__,                                                    \
                  __LINE__);                                                   \
                                                                               \
        RuntimeBreak();                                                        \
      }                                                                        \
    } while (false)

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
#define Assert(...)
#define InvalidCodePath(...)
#define RuntimeBreak(...)

#define NotImplemented
/* #define BUG */

/* #define NotImplemented implement_me_plox */
/* #define BUG(...) KnOwN_BuGgY_CoDePaTh_DeTeCtEd */

#endif // BONSAI_INTERNAL


#if DEBUG_SYSTEM_API
#define TriggeredRuntimeBreak(BoolPointer) do { if (*(BoolPointer)) { *(BoolPointer) = False; RuntimeBreak(); } } while (0)

#else

#define TriggeredRuntimeBreak(...)
#endif
