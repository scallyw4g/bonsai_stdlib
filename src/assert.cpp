void assert_fail(cs FmtString, const char *Function)
{
  LogDirect(CSz("%S ! FAILED%S  - "), TerminalColors.Red, TerminalColors.White);
  LogDirect(FmtString, Function);

  RuntimeBreak();
}

