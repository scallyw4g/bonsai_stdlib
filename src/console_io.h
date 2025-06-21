#define CLI_SUCCESS_EXIT_CODE 0
#define CLI_FAILURE_EXIT_CODE 1

#define RED_TERMINAL "\x1b[31m"
#define BLUE_TERMINAL "\x1b[34m"
#define PURPLE_TERMINAL "\x1b[35m"
#define GREEN_TERMINAL "\x1b[32m"
#define YELLOW_TERMINAL "\x1b[33m"
#define WHITE_TERMINAL "\x1b[37m"

// NOTE(Jesse): This is here such that we can see "StringsMatch()"
//
poof(string_and_value_tables(log_level));
#include <generated/string_and_value_tables_log_level.h>

// TODO(Jesse, globals_cleanup): Put these on stdlib ..?
global_variable native_file Stdout = {};
global_variable native_file Global_StdoutLogfile = {};

link_internal void
DumpValidLogLevelOptions()
{
  DebugChars("[");
  for (u32 LevelIndex = LogLevel_Undefined+1;
      LevelIndex <= LogLevel_Shush;
      ++LevelIndex)
  {
    DebugChars(" %S", ToString((log_level)LevelIndex));
    if (LevelIndex < LogLevel_Shush) { DebugChars(","); } 
  }
  DebugChars(" ]\n");
}

link_internal void
SetupStdout(u32 ArgCount, const char** ArgStrings)
{
  setvbuf(stdout, 0, _IONBF, 0);
  setvbuf(stderr, 0, _IONBF, 0);

  if (PlatformStdoutIsRedirected())
  {
    SetTerminalColorsOff();
  }

  for ( u32 ArgIndex = 1;
        ArgIndex < ArgCount;
        ++ArgIndex)
  {
    counted_string Arg = CS(ArgStrings[ArgIndex]);

    if (StringsMatch(CS("-c0"), Arg) ||
        StringsMatch(CS("--colors-off"), Arg) )
    {
      SetTerminalColorsOff();
    }
    else if ( StringsMatch(CS("--log-level"), Arg) )
    {
      if (ArgIndex+1 < ArgCount)
      {
        ArgIndex += 1;
        counted_string LevelString = CS(ArgStrings[ArgIndex]);
        log_level Level = LogLevel(LevelString);
        if (Level > LogLevel_Undefined)
        {
          Global_LogLevel = Level;
          Info("Setting Global_LogLevel to %S", LevelString);
        }
        else
        {
          Warn("Invalid --log-level switch value (%S)", LevelString);
          DebugChars("           - Valid values are ");
          DumpValidLogLevelOptions();
        }

      }
      else
      {
        Warn("Log Level required when using the --log-level switch.");
        DebugChars("           - Valid values are ");
        DumpValidLogLevelOptions();
      }
    }
  }

  if (Global_LogLevel == LogLevel_Undefined)
  {
    Global_LogLevel = LogLevel_Error;
  }

  return;
}
