void assert_fail(cs FmtString, cs Condition, const char *Function)
{
  LogDirect(CSz("%S ! FAILED%S  - "), TerminalColors.Red, TerminalColors.White);
  LogDirect(FmtString, Condition, Function);

#ifndef BONSAI_SOFT_ASSERT
  RuntimeBreak();
#endif
}

