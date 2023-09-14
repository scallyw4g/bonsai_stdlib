
global_variable counted_string_stream Global_ErrorStream = {};

link_internal peek_result PeekTokenRawCursor(peek_result *Peek, s32 TokenLookahead = 0);
link_internal peek_result PeekTokenRawCursor(c_token_cursor *Tokens, s32 TokenLookahead, b32 CanSearchDown = True);
link_internal peek_result PeekTokenRawCursor(parser *Parser, s32 TokenLookahead = 0);

/* link_internal peek_result PeekTokenCursor(peek_result *Peek, s32 TokenLookahead = 0); */
link_internal peek_result PeekTokenCursor(c_token_cursor *Tokens, s32 TokenLookahead = 0);
link_internal peek_result PeekTokenCursor(parser *Parser, s32 TokenLookahead = 0);

link_internal c_token * PeekTokenRawPointer(parser *Parser, u32 TokenLookahead);
link_internal c_token * PeekTokenRawPointer(parser *Parser, s32 TokenLookahead = 0);

link_internal c_token * PeekTokenPointer(c_token_cursor *Tokens, s32 TokenLookahead = 0);
link_internal c_token * PeekTokenPointer(parser *Parser, u32 TokenLookahead = 0);

link_internal c_token   PeekTokenRaw(parser *Parser, s32 Lookahead = 0);
link_internal c_token   PeekToken(parser *Parser, s32 Lookahead = 0);
link_internal c_token   PopTokenRaw(parser *Parser);
link_internal c_token * PopTokenRawPointer(parser *Parser);
link_internal c_token   PopToken(parser *Parser);
link_internal b32       OptionalTokenRaw(parser *Parser, c_token_type Type);
link_internal c_token * OptionalToken(parser *Parser, c_token T);
link_internal c_token * OptionalToken(parser *Parser, c_token_type Type);
link_internal c_token   RequireToken(parser *Parser, c_token *ExpectedToken);
link_internal c_token   RequireToken(parser *Parser, c_token ExpectedToken);
link_internal c_token   RequireToken(parser *Parser, c_token_type ExpectedType);
link_internal c_token   RequireTokenRaw(parser *Parser, c_token Expected);
link_internal c_token   RequireTokenRaw(parser *Parser, c_token *Expected);
link_internal c_token   RequireTokenRaw(parser *Parser, c_token_type ExpectedType);

link_internal b32       TokensRemain(parser *Parser, u32 TokenLookahead = 0);
link_internal b32       RawTokensRemain(parser *Parser, u32 TokenLookahead = 0);

link_internal void EraseToken(c_token *Token);
link_internal void EraseBetweenExcluding(parser *Parser, c_token *StartToken, c_token *OnePastLastToken);

link_internal void DumpLocalTokens(parser *Parser);
link_internal void PrintTray(char_cursor *Dest, c_token *T, u32 Columns, counted_string Color);

link_internal b32       TokenIsOperator(c_token_type T);
link_internal b32       NextTokenIsOperator(parser *Parser);
link_internal c_token * RequireOperatorToken(parser *Parser);

link_internal void      TrimFirstToken(parser* Parser, c_token_type TokenType);
link_internal void      TrimLastToken(parser* Parser, c_token_type TokenType);
link_internal void      TrimLeadingWhitespace(parser* Parser);

link_internal counted_string EatBetweenExcluding(ansi_stream*, char Open, char Close);

link_internal void           EatBetween(parser* Parser, c_token_type Open, c_token_type Close);
link_internal counted_string EatBetween_Str(parser* Parser, c_token_type Open, c_token_type Close);
link_internal b32            EatWhitespace(parser* Parser);
link_internal b32            EatSpacesTabsAndEscapedNewlines(parser *Parser);
link_internal void           EatWhitespaceAndComments(parser *Parser);

link_internal void      FullRewind(parser* Parser);

link_internal parser * DuplicateParserTokens(parser *Parser, memory_arena *Memory);
link_internal parser * DuplicateParser(parser *Parser, memory_arena *Memory);
link_internal c_token_cursor * DuplicateCTokenCursor(c_token_cursor *Tokens, memory_arena *Memory);
link_internal parser *         DuplicateCTokenCursor2(c_token_cursor *Tokens, memory_arena *Memory);


inline c_token
CToken(r32 FloatValue)
{
  c_token Result = {
    .Type = CTokenType_Float,
    .Value = FormatCountedString(GetTranArena(), CSz("%f"), r64(FloatValue)), // TODO(Jesse id: 350, tags: memory_leak)
    .FloatValue = r64(FloatValue)
  };
  return Result;
}

inline c_token
CToken(u32 UnsignedValue)
{
  c_token Result = {
    .Type = CTokenType_IntLiteral,
    .Value = FormatCountedString(GetTranArena(), CSz("%u"), UnsignedValue), // TODO(Jesse id: 351, tags: memory_leak)
    .UnsignedValue = UnsignedValue,
  };
  return Result;
}

c_token_cursor *
AllocateTokenCursor(memory_arena* Memory, counted_string Filename, umm Count, token_cursor_source Source, u32 LineNumber, c_token_cursor_up Up)
{
  c_token_cursor *Result = AllocateProtection(c_token_cursor, Memory, 1, False);
  CTokenCursor(Result, Count, Memory, Filename, Source, Up);
  return Result;
}

link_internal parser
AllocateParser(counted_string Filename, u32 LineNumber, u32 TokenCount, token_cursor_source Source, u32 OutputBufferTokenCount, c_token_cursor_up Up, memory_arena *Memory)
{
  TIMED_FUNCTION();

  parser Result = {};

  Result.Tokens = AllocateTokenCursor(Memory, Filename, TokenCount, Source, LineNumber, Up);
  if (!Result.Tokens->Start)
  {
    Error("Allocating Token Buffer");
    return Result;
  }

  return Result;
}

link_internal parser*
AllocateParserPtr(counted_string Filename, u32 LineNumber, u32 TokenCount, token_cursor_source Source, u32 OutputBufferTokenCount, c_token_cursor_up Up,  memory_arena *Memory)
{
  Assert(OutputBufferTokenCount == 0);
  parser *Result = AllocateProtection(parser, Memory, 1, False);
  *Result = AllocateParser(Filename, LineNumber, TokenCount, Source, OutputBufferTokenCount, Up, Memory);
  return Result;
}

c_token
PeekToken(ansi_stream* Stream, u32 Lookahead = 0)
{
  c_token Result = {};

  if (Stream->At+Lookahead < Stream->End)
  {
    char At = *(Stream->At+Lookahead);
    // TODO(Jesse, id: 193, tags: metaprogramming): Metaprogram this.  I've had bugs multiple times because of it.
    switch (At)
    {
      case CT_ControlChar_Start_of_Heading:
      case CT_ControlChar_Start_of_Text:
      case CT_ControlChar_End_of_Text:
      case CT_ControlChar_End_of_Transmission:
      case CT_ControlChar_Enquiry:
      case CT_ControlChar_Acknowledgement:
      case CT_ControlChar_Bell:
      case CT_ControlChar_Backspace:
      case CT_ControlChar_Vertical_Tab:
      case CT_ControlChar_Form_Feed:
      case CT_ControlChar_Shift_Out:
      case CT_ControlChar_Shift_In:
      case CT_ControlChar_Data_Link_Escape:
      case CT_ControlChar_Device_Control_1:
      case CT_ControlChar_Device_Control_2:
      case CT_ControlChar_Device_Control_3:
      case CT_ControlChar_Device_Control_4:
      case CT_ControlChar_Negative_Acknowledgement:
      case CT_ControlChar_Synchronous_Idle:
      case CT_ControlChar_End_of_Transmission_Block:
      case CT_ControlChar_Cancel:
      case CT_ControlChar_End_of_Medium:
      case CT_ControlChar_Substitute:
      case CT_ControlChar_Escape:
      case CT_ControlChar_File_Separator:
      case CT_ControlChar_Group_Separator:
      case CT_ControlChar_Record_Separator:
      case CT_ControlChar_Unit_Separator:
      case CT_ControlChar_Delete:

      case CTokenType_OpenBracket:
      case CTokenType_CloseBracket:
      case CTokenType_OpenBrace:
      case CTokenType_CloseBrace:
      case CTokenType_OpenParen:
      case CTokenType_CloseParen:
      case CTokenType_Dot:
      case CTokenType_Comma:
      case CTokenType_Semicolon:
      case CTokenType_Colon:
      case CTokenType_Hash:
      case CTokenType_At:
      case CTokenType_Dollar:
      case CTokenType_Space:
      case CTokenType_Tab:
      case CTokenType_Star:
      case CTokenType_Ampersand:
      case CTokenType_SingleQuote:
      case CTokenType_DoubleQuote:
      case CTokenType_Equals:
      case CTokenType_LT:
      case CTokenType_GT:
      case CTokenType_Plus:
      case CTokenType_Minus:
      case CTokenType_Percent:
      case CTokenType_Bang:
      case CTokenType_Hat:
      case CTokenType_Question:
      case CTokenType_FSlash:
      case CTokenType_BSlash:
      case CTokenType_Tilde:
      case CTokenType_Backtick:
      case CTokenType_Pipe:
      case CTokenType_Newline:
      case CTokenType_CarrigeReturn:
      case CTokenType_EOF:
      {
        Result = { .Type = (c_token_type)At };
      } break;
    }
  }
  else
  {
    Warn("Attempted to get token past end of stream on file : %S", Stream->Filename);
  }

  return Result;
}

link_internal b32
Contains(parser *Parser, c_token *T)
{
  b32 Result = False;
  if (T >= Parser->Tokens->Start && T < Parser->Tokens->End)
  {
    Result = True;
  }
  return Result;
}

link_internal string_from_parser
StartStringFromParser(parser* Parser)
{
  c_token *T = PeekTokenRawPointer(Parser);

  string_from_parser Result = {};

  if (T)
  {
    Result.Parser = Parser;
    Result.StartToken = T;
  }

  return Result;
}

link_internal counted_string
FinalizeStringFromParser(string_from_parser* Builder)
{
  counted_string Result = {};

  parser *Parser = Builder->Parser;
  c_token *StartToken = Builder->StartToken;

  if (Parser)
  {
    if (Contains(Parser, StartToken))
    {
      umm Count = 0;
      // NOTE(Jesse): This would be better if it excluded the At token, but
      // unfortunately we wrote the calling code such that the At token is
      // implicitly included, so we have to have this weird check.
      if (Parser->Tokens->At == Parser->Tokens->End)
      {
        auto LastTokenValue = Parser->Tokens->At[-1].Value;
        Count = (umm)( (LastTokenValue.Start+LastTokenValue.Count) - Builder->StartToken->Value.Start );
      }
      else
      {
        Count = (umm)(Parser->Tokens->At->Value.Start - Builder->StartToken->Value.Start);
      }

      Result = CS(Builder->StartToken->Value.Start, Count);
    }
    else
    {
      Warn(CSz("Unable to call FinalizeStringFromParser due to having spanned a parser chain link."));
    }
  }

  return Result;
}

inline c_token_cursor *
HasValidDownPointer(c_token *T)
{
  c_token_cursor *Result = (T && T->Down && (T->Type == CT_MacroLiteral || T->Type == CT_InsertedCode)) ? T->Down : 0;
  return Result;
}

#if BONSAI_SLOW
link_internal void
SanityCheckCTokenCursor(c_token_cursor *Current)
{
  for (u32 TokenIndex = 0; TokenIndex < TotalElements(Current); ++TokenIndex)
  {
    c_token *T = Current->Start + TokenIndex;

    if (HasValidDownPointer(T))
    {
      Assert(T->Down->Up.Tokens == Current);
      SanityCheckCTokenCursor(T->Down);
    }
  }
}

link_internal void
SanityCheckParserChain(parser *Parser)
{
#if 1
  Assert(Parser->Tokens->Up.Tokens != Parser->Tokens);

  c_token_cursor *FirstInChain = Parser->Tokens;

  while (FirstInChain->Up.Tokens) FirstInChain = FirstInChain->Up.Tokens;
  SanityCheckCTokenCursor(FirstInChain);
#endif
}
#else
#define SanityCheckParserChain(...)
#define SanityCheckCTokenCursor(...)
#endif

#if 0
link_internal void
SinglyLinkedListSwapInplace(c_token_cursor *P0, c_token_cursor *P1)
{
  NotImplemented;
  Assert(P0->Up != P0);
  Assert(P1->Up != P1);

  Assert(P1->Up == P0);

  auto M0 = *P0; // Mnemonic M0 == Memory0
  auto M1 = *P1;

  *P0 = M1;
  *P1 = M0;

  P0->Up = P0;
  P1->Up = P0;

  Assert(P0->Up != P0);
  Assert(P1->Up != P1);
}
#endif

#if 1
struct d_list
{
  d_list *Prev;
  d_list *Next;
};

link_internal void
DoublyLinkedListSwap(d_list *P0, d_list *P1)
{
  Assert(P0 != P1);

  b32 Colocated = P0->Next == P1;
  b32 ColocatedReversed = P1->Next == P0;

  // TODO(Jesse): I'm fairly sure that we don't need this boolean and should
  // be able to just go off the M pointer values .. but I didn't want to sit
  // around and figure it out.  Same goes for ColocatedReversed.  Good news is
  // that this routine is well tested so future-me can fearlessly modify it.
#if 0
  if (Colocated)
  {
    if (P1->Prev != P0)
    {
      Assert(false);

      DumpLocalTokens(P0);
      DebugChars("\n");
      DumpLocalTokens(P1);
      DebugChars("\n");
      DumpLocalTokens(P1->Next);
      DebugChars("\n");
      DumpLocalTokens(P1->Prev);
      DebugChars("\n");
    }
  }
#endif

  if (ColocatedReversed)
  {
    Assert(P0->Prev == P1);
    Assert(!Colocated);

    Colocated = True;
    d_list *Temp = P1;
    P1 = P0;
    P0 = Temp;
  }

  d_list M0 = *P0; // Mnemonic M0 == Memory0
  d_list M1 = *P1;

  *P0 = M1;
  *P1 = M0;

  P0->Next = M1.Next;
  P0->Prev = Colocated ? P1 : M1.Prev;

  P1->Next = Colocated ? P0 : M0.Next;
  P1->Prev = M0.Prev;

  if (M1.Next)
  {
    M1.Next->Prev = P0;
  }

  if (M0.Prev)
  {
    M0.Prev->Next = P1;
  }

  if (!Colocated)
  {
    if (M1.Prev)
    {
      M1.Prev->Next = P0;
    }
    if (M0.Next)
    {
      M0.Next->Prev = P1;
    }
  }

  return;
}
#endif


















link_internal c_token *
RewindTo(parser* Parser, c_token *T)
{
  peek_result Current = {};
  if (T)
  {
    Current = PeekTokenRawCursor(Parser->Tokens, -1);
    while (Current.At && Current.At != T)
    {
      Current = PeekTokenRawCursor(&Current, -1);
    }

    if (IsValid(&Current))
    {
      Parser->Tokens = Current.Tokens;
      Parser->Tokens->At = Current.At;
    }
    else
    {
      FullRewind(Parser);
    }
  }
  else
  {
    Warn("ptr(0) passed to RewindTo");
  }

  return Current.At;
}

link_internal c_token *
RewindTo(parser* Parser, c_token_type Type, u32 Count = 0)
{
  peek_result Current = {};
  if (Type)
  {
    u32 Hits = 0;

    Current = PeekTokenRawCursor(Parser->Tokens, -1);
    while (Current.At)
    {
      if (Current.At->Type == Type)
      {
        if (Hits++ == Count)
        {
          break;
        }
      }

      Current = PeekTokenRawCursor(&Current, -1);
    }

    if (IsValid(&Current))
    {
      Parser->Tokens = Current.Tokens;
      Parser->Tokens->At = Current.At;
    }
    else
    {
      FullRewind(Parser);
    }
  }
  else
  {
    Warn("CTokenType_Unknown passed to RewindTo");
  }

  return Current.At;
}

// NOTE(Jesse): This function should actually just be able to walk the "Up" chain
// since we never actually modify cursors
link_internal void
FullRewind(parser* Parser)
{
  TIMED_FUNCTION();

  SanityCheckParserChain(Parser);

  c_token_cursor *Current = Parser->Tokens;
  Rewind(Current);

  while (Current->Up.Tokens)
  {
    Current = Current->Up.Tokens;
    Rewind(Current);
  }
  Assert(Current->Up.Tokens == 0);

  if (Current != Parser->Tokens)
  {
    Parser->Tokens = Current;
  }
  Assert(Parser->Tokens->Up.Tokens == 0);

  SanityCheckParserChain(Parser);
}

link_internal c_token *
AdvanceTo(parser *Parser, peek_result *Peek)
{
  c_token *Result = 0;
  if (Parser->Tokens->At < Parser->Tokens->End)
  {
    // NOTE(Jesse): Yes, it's really weird we return the thing we were on here
    // instead of what we're advancing to.  Yes, we should change it.
    Result = Parser->Tokens->At;

    if (IsValid(Peek))
    {
      Assert(Peek->Tokens);
      Parser->Tokens = Peek->Tokens;
      Parser->Tokens->At = Peek->At;
      Assert(Peek->Tokens->At == Peek->At);
    }
    else
    {
      Parser->Tokens->At = Parser->Tokens->End;
    }
  }

  return Result;
}

link_internal c_token *
AdvanceTo(parser* Parser, c_token* T)
{
  SanityCheckParserChain(Parser);
  peek_result Peek = PeekTokenRawCursor(Parser);

  while (IsValid(&Peek) && Peek.At != T)
  {
    Peek = PeekTokenRawCursor(&Peek, 1);
  }

  if (IsValid(&Peek))
  {
    Assert(Peek.At == T);
    AdvanceTo(Parser, &Peek);
    Assert(Parser->Tokens->At == T);
  }

  SanityCheckParserChain(Parser);
  return Peek.At;
}

link_internal void
EatUntilExcluding(parser* Parser, c_token_type Close)
{
  // TODO(Jesse, performance, slow): This is slow AF
  while (PeekTokenRawPointer(Parser))
  {
    if(PeekTokenRaw(Parser).Type == Close)
    {
      break;
    }
    else
    {
      PopTokenRaw(Parser);
    }
  }
  return;
}

link_internal c_token *
EatUntilIncluding(parser* Parser, c_token_type Close)
{
  c_token *Result = 0;
  while (c_token *T = PopTokenRawPointer(Parser))
  {
    if(T->Type == Close)
    {
      Result = T;
      break;
    }
  }
  return Result;
}

link_internal b32
AdvanceParser(parser *Parser)
{
  NotImplemented;
  return False;
}



/****************************                   ******************************/
/****************************  Error Reporting  ******************************/
/****************************                   ******************************/



// TODO(Jesse): Remove this?  Maybe put it onto the parser?   I'm not crazy
// about that because it' bloats that struct and we create those things like
// crazy.. but I don't really like that it's a global either.
//
// UPDATE(Jesse): Actually now we create parsers pretty sparingly so we should
// be able to do one per parser without too much bloat..
static const u32 Global_ParseErrorBufferSize = 64*1024;
static char Global_ParseErrorBuffer[Global_ParseErrorBufferSize] = {};

link_internal void
Highlight(char_cursor *Dest, counted_string Str, counted_string Color)
{
  CopyToDest(Dest, Color);
  CopyToDest(Dest, Str);
  CopyToDest(Dest, TerminalColors.White);
}

link_internal void
Highlight(char_cursor *Dest, char C, counted_string Color)
{
  CopyToDest(Dest, Color);
  CopyToDest(Dest, C);
  CopyToDest(Dest, TerminalColors.White);
}

link_internal void
Indent(char_cursor *Dest, u32 TabCount, u32 SpaceCount)
{
  for (u32 ColumnIndex = 0;
      ColumnIndex < TabCount;
      ++ColumnIndex)
  {
    CopyToDest(Dest, '\t');
  }

  for (u32 ColumnIndex = 0;
      ColumnIndex < SpaceCount;
      ++ColumnIndex)
  {
    CopyToDest(Dest, ' ');
  }
}

link_internal void
OutputIdentifierUnderline(char_cursor *Dest, u32 IdentifierLength, counted_string Color)
{
  u32 IdentifierLengthSubOne = IdentifierLength > 0 ? IdentifierLength-1 : IdentifierLength;
  b32 DoPipes = IdentifierLength > 3;

  CopyToDest(Dest, Color);

  for (u32 ColumnIndex = 0;
      ColumnIndex < IdentifierLength;
      ++ColumnIndex)
  {
    if (DoPipes)
    {
      if (ColumnIndex == 0)
      {
        CopyToDest(Dest, '|');
      }
      else if (ColumnIndex == IdentifierLengthSubOne)
      {
        CopyToDest(Dest, '|');
      }
      else
      {
        CopyToDest(Dest, '~');
      }
    }
    else
    {
      CopyToDest(Dest, '^');
    }
  }

  CopyToDest(Dest, TerminalColors.White);
}

link_internal void
PrintTray(char_cursor *Dest, c_token *T, u32 Columns, counted_string Color = TerminalColors.White)
{
  if (T)
  {
    FormatCountedString_(Dest->Start, TotalElements(Dest), CSz("%*d |"), Columns, T->LineNumber);
  }
  else
  {
    FormatCountedString_(Dest->Start, TotalElements(Dest), CSz("%*c %S|%S"), Columns, ' ', Color, TerminalColors.White);
  }
}

link_internal u64
GetLongestLineInCursor(char_cursor *Cursor)
{
  Assert(Cursor->At == Cursor->Start);
  u64 Result = 0;

  u64 CurrentLineLength = 0;
  while (Remaining(Cursor))
  {
    if (Cursor->At[0] == '\n')
    {
      if (CurrentLineLength > Result)
      {
        Result = CurrentLineLength;
      }
      CurrentLineLength = 0;
    }
    else if (Cursor->At[0] == '\x1b')
    {
      while (Remaining(Cursor))
      {
        Advance(Cursor);

        if (Cursor->At[0] == 'm')
        {
          break;
        }
      }
    }
    else
    {
      CurrentLineLength++;
    }

    Advance(Cursor);
  }

  if (CurrentLineLength > Result)
  {
    Result = CurrentLineLength;
  }

  return Result;
}

link_internal void
PrintContext(c_token_cursor_up *Up)
{
  if (Up->Tokens)
  {
    if (Up->Tokens->Up.Tokens)
    {
      PrintContext(&Up->Tokens->Up);
    }

    c_token_cursor Tokens = *Up->Tokens;
                   Tokens.At = Up->At;


    u32 LinesOfContext = 5;
    /* peek_result Current = PeekTokenCursor(Parser, 0, False); */
    /* while (IsValid(&Current)) */

    DumpCursorSlice(&Tokens, 35, 35);
  }
}

link_internal void
OutputContextMessage(parser* Parser, parse_error_code ErrorCode, counted_string MessageContext, counted_string Message, c_token* ErrorToken)
{
  /* DumpCursorSimple(Parser->Tokens); */

  parse_error_code PrevErrorCode = Parser->ErrorCode;
  Parser->ErrorCode = ParseErrorCode_None;

  c_token *OriginalAtToken = PeekTokenRawPointer(Parser);

  if (Parser->Tokens->Up.Tokens)
  {
    PrintContext(&Parser->Tokens->Up);
  }

  // @optimize_call_advance_instead_of_being_dumb
  if (!ErrorToken)
  {
    ErrorToken = PeekTokenRawPointer(Parser);
    if (ErrorToken == 0)
    {
      ErrorToken = PeekTokenRawPointer(Parser, -1);
    }
    AdvanceTo(Parser, ErrorToken);
  }

  char_cursor ParseErrorCursor_ = {};
  char_cursor *ParseErrorCursor = &ParseErrorCursor_;
  ParseErrorCursor->Start = Global_ParseErrorBuffer;
  ParseErrorCursor->At = Global_ParseErrorBuffer;
  ParseErrorCursor->End = Global_ParseErrorBuffer+Global_ParseErrorBufferSize;

  u32 LinesOfContext = 7;

  counted_string ParserName = {};

  b32 DoErrorOutput = False;
  if (AdvanceTo(Parser, ErrorToken))
  {
    DoErrorOutput = True;
  }
  else if (RewindTo(Parser, ErrorToken))
  {
    DoErrorOutput = True;
  }
  else
  {
    Warn("Couldn't find specified token in parser chain.");
  }


  if (DoErrorOutput)
  {
    u32 ErrorLineNumber = ErrorToken->LineNumber;
    u32 MaxTrayWidth = 1 + GetColumnsFor(ErrorLineNumber + LinesOfContext);

    // Indent info messages a bunch so we can visually tell the difference
    if (ErrorCode == ParseErrorCode_None)
    {
      MaxTrayWidth += 4;
    }

    ParserName = ErrorToken->Filename;

    Assert(PeekTokenRawPointer(Parser) == ErrorToken);

    RewindTo(Parser, CTokenType_Newline, LinesOfContext);

    u32 SpaceCount = 0;
    u32 TabCount = 0;

    u32 ErrorIdentifierLength = (u32)ErrorToken->Value.Count;

    //
    // Print context until we hit the error token.  Also keep track of spaces
    // and tabs while we go such that we know how much to indent error message
    //
    PrintTray(ParseErrorCursor, PeekTokenRawPointer(Parser), MaxTrayWidth);
    while (c_token *T = PopTokenRawPointer(Parser))
    {
      PrintToken(T, ParseErrorCursor);

      // TODO(Jesse, easy): These serial ifs could probably be cleaned up with a switch.
      if (T == ErrorToken)
      {
        break;
      }
      else if ( IsNewline(T->Type) )
      {
        TabCount = 0;
        SpaceCount = 0;

        // TODO(Jesse, easy): Pretty sure this can go away and just call PrintTray w/ Peek
        if (TokenShouldModifyLineCount(T, Parser->Tokens->Source))
        {
          PrintTray(ParseErrorCursor, PeekTokenRawPointer(Parser), MaxTrayWidth);
        }
        else
        {
          PrintTray(ParseErrorCursor, 0, MaxTrayWidth);
        }

      }
      else if (T->Type == CTokenType_Tab)
      {
        ++TabCount;
      }
      else if (T->Value.Count)
      {
        SpaceCount += T->Value.Count;
      }
      else
      {
        ++SpaceCount;
      }

      continue;
    }

    //
    // Print the rest of the line the error token was found on.
    //
    {
      c_token *T = PopTokenRawPointer(Parser);
      while (T)
      {
        PrintToken(T, ParseErrorCursor);

        if (IsNewline(T))
        {
          break;
        }

        T = PopTokenRawPointer(Parser);
      }
    }

    //
    // Output the error message
    //
    u64 MinLineLen = 80;
    {
      c_token *NextT = PeekTokenRawPointer(Parser);
      c_token *PrevT = PeekTokenRawPointer(Parser, -1);
      if (NextT == 0 && PrevT && PrevT->Type != CTokenType_Newline)
      {
        CopyToDest(ParseErrorCursor, '\n');
      }

      Indent(ParseErrorCursor, 0, MaxTrayWidth);
      Highlight(ParseErrorCursor, CSz(" >"), TerminalColors.DarkRed);
      Indent(ParseErrorCursor, TabCount, SpaceCount);
      OutputIdentifierUnderline(ParseErrorCursor, ErrorIdentifierLength, TerminalColors.DarkRed);
      CopyToDest(ParseErrorCursor, '\n');

      PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
      CopyToDest(ParseErrorCursor, '\n');

      PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
      CopyToDest(ParseErrorCursor, ' ');
      CopyToDest(ParseErrorCursor, ' ');

      if (MessageContext.Count)
      {
        Highlight(ParseErrorCursor, MessageContext, TerminalColors.Yellow);
        CopyToDest(ParseErrorCursor, CSz(" :: "));
      }

      for (u32 ECharIndex = 0;
          ECharIndex < Message.Count;
          ++ECharIndex)
      {
        char C = Message.Start[ECharIndex];
        CopyToDest(ParseErrorCursor, C);
        if (C == '\n')
        {
          PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
          CopyToDest(ParseErrorCursor, ' ');
          CopyToDest(ParseErrorCursor, ' ');
        }
      }

      if (MessageContext.Count)
      {
        if (ErrorCode)
        {
          CopyToDest(ParseErrorCursor, '\n');
          PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
          CopyToDest(ParseErrorCursor, ' ');
          CopyToDest(ParseErrorCursor, ' ');

          CopyToDest(ParseErrorCursor, '\n');
          PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
          CopyToDest(ParseErrorCursor, ' ');
          CopyToDest(ParseErrorCursor, ' ');
          CopyToDest(ParseErrorCursor, ToString(ErrorCode));
        }
      }


      { // Output the final underline
        char_cursor ErrorCursor = CharCursor(Message);
        u64 LongestLine = Max(MinLineLen, GetLongestLineInCursor(&ErrorCursor));

        CopyToDest(ParseErrorCursor, '\n');
        PrintTray(ParseErrorCursor, 0, MaxTrayWidth, TerminalColors.Yellow);
        CopyToDest(ParseErrorCursor, TerminalColors.Yellow);
        for (u32 DashIndex = 0;
            DashIndex < LongestLine + 2;
            ++DashIndex)
        {
          CopyToDest(ParseErrorCursor, '_');
        }
        CopyToDest(ParseErrorCursor, '\n');
        CopyToDest(ParseErrorCursor, TerminalColors.White);
      }
    }









    {
      c_token *NextT = PeekTokenRawPointer(Parser);
      if (LinesOfContext && NextT) { PrintTray(ParseErrorCursor, NextT, MaxTrayWidth); }
    }

    u32 LinesToPrint = LinesOfContext;
    while ( c_token *T = PopTokenRawPointer(Parser) )
    {
      if (LinesToPrint == 0) break;
      PrintToken(T, ParseErrorCursor);

      if ( IsNewline(T->Type) )
      {
        if (--LinesToPrint == 0) break;

        c_token *NextT = PeekTokenRawPointer(Parser);
        if (NextT) { PrintTray(ParseErrorCursor, NextT, MaxTrayWidth); }
      }
    }
    /* CopyToDest(ParseErrorCursor, '\n'); */

    TruncateToCurrentElements(ParseErrorCursor);
    Rewind(ParseErrorCursor);

    // TODO(Jesse, tags: bug): This isn't working for some reason.  I think
    // GetLongestLineInCursor is busted here.
    counted_string NameLine = FormatCountedString(GetTranArena(), CSz("  %S:%u  "), ParserName, ErrorLineNumber);
    u64 LongestLine = Max(MinLineLen, GetLongestLineInCursor(ParseErrorCursor));
    LongestLine = Max(MinLineLen, (u64)NameLine.Count+4);

    string_builder Builder = {};

    u64 HalfDashes = (LongestLine-NameLine.Count)/2;
    for (u32 DashIndex = 0; DashIndex < HalfDashes; ++DashIndex) { Append(&Builder, CSz("-")); }
    Append(&Builder, NameLine);
    for (u32 DashIndex = 0; DashIndex < HalfDashes; ++DashIndex) { Append(&Builder, CSz("-")); }
    Append(&Builder, CSz("\n"));

    // Intentional copy
    counted_string ErrorText = CopyString(CS(ParseErrorCursor), GetTranArena());
    Append(&Builder, ErrorText);
    /* LogDirect("%S", CS(ParseErrorCursor)); */

    counted_string FullErrorText = Finalize(&Builder, GetTranArena());
    Push(&Global_ErrorStream, FullErrorText);

    if (Global_LogLevel <= LogLevel_Error)
    {
      LogDirect("%S", FullErrorText);
    }


    if (RewindTo(Parser, OriginalAtToken))
    {
    }
    else if (AdvanceTo(Parser, OriginalAtToken))
    {
    }

    Assert(PeekTokenRawPointer(Parser) == OriginalAtToken);

    // Don't overwrite the error code if we call this with an info message (and pass ParseErrorCode_None)
    if (ErrorCode)
    {
      Parser->ErrorCode = ErrorCode;
    }
    else if (PrevErrorCode)
    {
      Parser->ErrorCode = PrevErrorCode;
    }
  }
  else
  {
    LogDirect(CSz("%S"), CSz("Error determining where the error occurred \n"));
    LogDirect(CSz("%S"),  CSz("Error messsage was : %S \n"), Message);
  }

  return;
}

link_internal void
ParseInfoMessage(parser* Parser, counted_string Message, c_token* T)
{
  OutputContextMessage(Parser, ParseErrorCode_None, CSz(""), Message, T);
}

link_internal void
ParseWarn(parser* Parser, parse_warn_code WarnCode, counted_string ErrorMessage, c_token* ErrorToken)
{
  Parser->WarnCode = WarnCode;
  OutputContextMessage(Parser, ParseErrorCode_None, CSz("Poof Warning"), ErrorMessage, ErrorToken);
}

link_internal void
ParseError(parser* Parser, parse_error_code ErrorCode, counted_string ErrorMessage, c_token* ErrorToken)
{
  OutputContextMessage(Parser, ErrorCode, CSz("Parse Error"), ErrorMessage, ErrorToken);
}

link_internal void
ParseError(parser* Parser, counted_string ErrorMessage, c_token* ErrorToken)
{
  ParseError(Parser, ParseErrorCode_Unknown, ErrorMessage, ErrorToken);
}


#if 0
link_internal counted_string
ParseError_StreamEndedUnexpectedly(parser *Parser)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("Stream ended unexpectedly in file : %S"), ???->Filename);
  ParseError(Parser, ParseErrorCode_StreamEndedUnexpectedly, Result);
  return Result;
}
#endif

#define ParseErrorTokenHelper(T) \
  (T) ? ToString((T)->Type) : CSz(""), (T) ? (T)->Value : CSz("null")

link_internal counted_string
ParseError_ExpectedSemicolonEqualsCommaOrOpenBrace(parser *Parser, c_token *T)
{
  counted_string Result =
    FormatCountedString_(GetTranArena(), CSz("Got %S(%S)\n\nExpected %S(%c), %S(%c), %S(%c) or %S(%c) while parsing variable declaration."),
        ParseErrorTokenHelper(T),
        ToString(CTokenType_Semicolon), CTokenType_Semicolon,
        ToString(CTokenType_Equals), CTokenType_Equals,
        ToString(CTokenType_Comma), CTokenType_Comma,
        ToString(CTokenType_OpenBrace), CTokenType_OpenBrace
        );

  ParseError(Parser, ParseErrorCode_ExpectedSemicolonOrEquals, Result, PeekTokenPointer(Parser) );
  return Result;
}

link_internal counted_string
ParseError_RequireTokenFailed(parser *Parser, counted_string FuncName, c_token *Got, c_token *Expected)
{
  counted_string Result = FormatCountedString( GetTranArena(),
                                               CSz("%S Failed \n\nExpected %S(%S)\nGot      %S(%S)"),
                                               FuncName,
                                               ParseErrorTokenHelper(Expected),
                                               ParseErrorTokenHelper(Got));

  ParseError(Parser, ParseErrorCode_RequireTokenFailed, Result, Got);
  return Result;
}

/*****************************                 *******************************/
/*****************************  Token Control  *******************************/
/*****************************                 *******************************/



#define DEBUG_CHECK_FOR_BREAK_HERE(Result) \
  ( (Result) && ((Result)->Type == CTokenType_Identifier && StringsMatch(CSz("break_here"), (Result)->Value)) )


inline u32
RemainingForDir(c_token_cursor *Tokens, s32 Direction)
{
  u32 TokensRemaining = Direction > -1 ? (u32)Remaining(Tokens) : (u32)AtElements(Tokens);
  return TokensRemaining;
}

link_internal c_token *
GetNext(c_token_cursor *Tokens, s32 Direction)
{
  u32 TokensRemaining = RemainingForDir(Tokens, Direction);

  c_token *Result = 0;

  // TODO(Jesse)(speed optimize): These serially-dependant if-s should be able
  // to be written independantlly such that the CPU can execute them
  // out-of-order.  We might actually care about that.

  if (Direction == 0 && TokensRemaining > 0)
  {
    Result = Tokens->At;
  }
  else if ( Direction == -1 && TokensRemaining > 0 )
  {
    Tokens->At += Direction;
    Assert(IsValidForCursor(Tokens, Tokens->At));
    Result = Tokens->At;
  }
  else if ( Direction == 1 && TokensRemaining > 1 )
  {
    Tokens->At += Direction;
    Assert(IsValidForCursor(Tokens, Tokens->At));
    Result = Tokens->At;
  }
  else
  {
    Invalidate(Tokens);
  }

  return Result;
}

#if 0
link_internal c_token*
GetToken(c_token_cursor *Tokens, s32 PeekIndex)
{
  c_token* Result = 0;

  if (Tokens)
  {
    s32 Direction = GetSign(PeekIndex);
    Direction = Direction ? Direction : 1;
    u32 TokensRemaining = RemainingForDir(Tokens, Direction);
    if (TokensRemaining > Abs(PeekIndex))
    {
      Result = Tokens->At + PeekIndex;
      Assert(IsValidForCursor(Tokens, Result));
    }
    else
    {
      /* s32 Next = PeekIndex - (TokensRemaining*Direction); */
      /* Assert(GetSign(Next) == GetSign(PeekIndex) || Next == 0); */
      /* Result = GetToken(Tokens->Up, Next); */
    }
  }

  return Result;
}
#endif

#if 0
link_internal c_token*
PeekTokenRawPointer(c_token_cursor *Tokens, s32 MaxPeek)
{
  Assert(Tokens->At >= Tokens->Start);
  Assert(Tokens->At <= Tokens->End);
  Assert(Tokens->Start <= Tokens->End);

  c_token_cursor Tmp = *Tokens;

  s32 Direction = GetSign(MaxPeek);
  s32 PeekIndex = Direction;

  if (Direction == 0) Direction = 1;

  c_token* Result = 0;
  while ( (Result = GetToken(&Tmp, PeekIndex)) )
  {
    // We need to break out of the loop here if we've hit the last token we're
    // going to peek because if we wait till the end of the loop we'd peek into
    // CT_InsertedCode->Down.  We also have to check at the end of the loop
    // such that when we do peek down we don't overwrite the Result when we hit
    // the while() at the top.  Pretty crap, I know.. probably a better way of
    // phrasing this function.
    if (PeekIndex == MaxPeek)
    {
      break;
    }

    switch (Result->Type)
    {
      case CT_MacroLiteral:
      case CT_InsertedCode:
      {
        if (Result->Down)
        {
          // NOTE(Jesse): I ordered the 'macro_expansion' struct such that the
          // pointer to the expanded macro will be at the same place as the
          // `Down` poninter.  This is sketchy as fuck, but it'll work, and
          // this assertion should catch the bug if we reorder the pointers.
          Assert(Result->Down == Result->Macro.Expansion);

          Assert(Result->Down->At == Result->Down->Start);
          c_token_cursor TmpDown = *Result->Down;
          TmpDown.Up = &Tmp;
          Result = PeekTokenRawPointer(&TmpDown, MaxPeek - (PeekIndex + Direction));

          // NOTE(Jesse): This loop is gettin pretty tortured .. probably time
          // to rewrite it and clean up.
          /* PeekIndex = MaxPeek; */
        }
      } break;

      default: { } break;
    }

    if (PeekIndex == MaxPeek)
    {
      break;
    }
    else
    {
      PeekIndex += Direction;
    }
  }

#if BONSAI_INTERNAL
  if (Result && DEBUG_CHECK_FOR_BREAK_HERE(Result))
  {
    RuntimeBreak();
    Result = PeekTokenRawPointer(&Tmp, MaxPeek + 1);
  }

  if (Result && Result->Type == CTokenType_Identifier) { Assert(Result->Value.Start); Assert(Result->Value.Count);  }
  if (Result) { Assert(Result->Type); }
#endif

  Assert(Tmp.At    >= Tmp.Start);
  Assert(Tmp.At    <= Tmp.End);
  Assert(Tmp.Start <= Tmp.End);

  return Result;
}

#else

link_internal peek_result
PeekTokenRawCursor(c_token_cursor *Tokens, s32 Direction, b32 CanSearchDown)
{
  Assert(Direction > -2 && Direction < 2);

  c_token_cursor Cached = *Tokens;

  peek_result Result = {};
  Result.Tokens = Tokens;

  if (Tokens->At < Tokens->End)
  {
    Result.At = Tokens->At;
  }

  switch  (Direction)
  {
    case 0: {} break;

    case 1:
    {
      c_token_cursor *Down = HasValidDownPointer(Result.At);
      b32 SearchDown = CanSearchDown && Down && RemainingForDir(Down, Direction) > 0;
      if (SearchDown)
      {
        Assert(Result.At->Down == Result.At->Macro.Expansion); // @janky-macro-expansion-struct-ordering

        c_token *PrevDownAt = Down->At;
        Down->At = Down->Start;
        Result = PeekTokenRawCursor(Down, Min(0, Direction));
        Down->At = PrevDownAt;

        // NOTE(Jesse): This is not strictly valid, but for well-formed code it
        // should always pass
        Assert(GetNext(Tokens, 0) != 0);
      }

      if ( (SearchDown && IsValid(&Result) == False) || // Didn't get a token peeking down
            SearchDown == False                       )
      {
        Result.At = GetNext(Tokens, Direction);
      }

      if (!IsValid(&Result) && Tokens->Up.Tokens) // Down buffer(s) and current buffer had nothing, pop up
      {
        c_token *UpAt = Tokens->Up.Tokens->At;
        Tokens->Up.Tokens->At = Tokens->Up.At;
        Assert(Tokens->Up.At->Type == CT_InsertedCode ||
               Tokens->Up.At->Type == CT_MacroLiteral);
        Result = PeekTokenRawCursor(Tokens->Up.Tokens, Direction);
        Tokens->Up.Tokens->At = UpAt;
        Result.DoNotDescend = True;
      }
    } break;

    case -1:
    {
      Result.At = GetNext(Tokens, Direction);

      c_token_cursor *Down = HasValidDownPointer(Result.At);
      b32 SearchDown = CanSearchDown && Down; // && RemainingForDir(Down, Direction) > 0;
      if (SearchDown)
      {
        Assert(Result.At->Down == Result.At->Macro.Expansion); // @janky-macro-expansion-struct-ordering

        c_token *PrevDownAt = Down->At;
        Down->At = Down->End;
        Result = PeekTokenRawCursor(Down, Min(0, Direction));
        Down->At = PrevDownAt;

        // NOTE(Jesse): This is not strictly valid, but for well-formed code it
        // should always pass
        Assert(GetNext(Tokens, 0) != 0);
      }

      if (!IsValid(&Result) && Tokens->Up.Tokens) // Down buffer(s) and current buffer had nothing, pop up
      {
        c_token *UpAt = Tokens->Up.Tokens->At;
        Tokens->Up.Tokens->At = Tokens->Up.At;
        Assert(Tokens->Up.At->Type == CT_InsertedCode ||
               Tokens->Up.At->Type == CT_MacroLiteral);
        Result = PeekTokenRawCursor(Tokens->Up.Tokens, 0, False);
        Tokens->Up.Tokens->At = UpAt;
        Result.DoNotDescend = True;
      }

    } break;

    InvalidDefaultCase;
  }

  *Tokens = Cached;

  if (IsValid(&Result))
  {
    Assert(Result.At);
    Assert(Result.Tokens);
    Assert(IsValidForCursor(Result.Tokens, Result.At));

    Assert(Result.Tokens->Start);
    Assert(Result.Tokens->At);
    Assert(Result.Tokens->End);
  }


#if 0
#if BONSAI_INTERNAL
  if (Result && DEBUG_CHECK_FOR_BREAK_HERE(*Result))
  {
    Result = PeekTokenPointer(Tokens, Direction);
  }

  if (Result) { Assert(!StringsMatch(Result->Value, CSz("break_here"))); }
#endif
#endif

  return Result;
}

link_internal peek_result
PeekTokenRawCursor(peek_result *Peek, s32 Direction)
{
  peek_result Result = {};

  if (IsValid(Peek))
  {
    c_token *PrevAt = Peek->Tokens->At;
    Peek->Tokens->At = Peek->At;
    peek_result TmpResult = PeekTokenRawCursor(Peek->Tokens, Direction, Peek->DoNotDescend == False);
    Peek->Tokens->At = PrevAt;

    if (IsValid(&TmpResult))
    {
      Result = TmpResult;
    }
  }
  return Result;
}

link_internal peek_result
PeekTokenRawCursor(parser *Parser, s32 Direction)
{
  peek_result Result = {};
  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    Result = PeekTokenRawCursor(Parser->Tokens, Direction);
  }
  return Result;
}

#endif

link_internal c_token*
PeekTokenRawPointer(parser* Parser, s32 Lookahead)
{
  c_token *T = {};

  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    s32 Direction = GetSign(Lookahead);
    s32 Count = Direction;

    peek_result Current = PeekTokenRawCursor(Parser->Tokens, Direction);
    while (Count != Lookahead)
    {
      Current = PeekTokenRawCursor(&Current, Direction);
      Count += Direction;
    }

    if (IsValid(&Current))
    {
      T = Current.At;
    }
  }
  return T;
}

link_internal c_token*
PeekTokenRawPointer(parser* Parser, u32 Lookahead)
{
  c_token *T = PeekTokenRawPointer(Parser, (s32)Lookahead);
  return T;
}

link_internal c_token
PeekTokenRaw(parser* Parser, s32 Direction)
{
  c_token *T = PeekTokenRawPointer(Parser, Direction);
  c_token Result = {};
  if (T) Result = *T;
  return Result;
}

link_internal peek_result
PeekTokenCursor(c_token_cursor *Tokens, s32 Skip)
{
  SanityCheckCTokenCursor(Tokens);

  c_token *Result = {};

  s32 Direction = GetSign(Skip);
  if (Direction == 0) Direction = 1;
  Assert(Direction == 1 || Direction == -1);


  s32 Hits = Min(0, Direction);
  peek_result Current = PeekTokenRawCursor(Tokens, Hits);
  while ( IsValid(&Current) )
  {
    if ( Current.At->Erased )
    {
    }
    else if ( Current.At->Type == CTokenType_CommentSingleLine)
    {
    }
    else if ( Current.At->Type == CTokenType_CommentMultiLine)
    {
    }
    else if (IsWhitespace(Current.At))
    {
    }
    else
    {
      if (Hits == Skip)
      {
        break;
      }
      else
      {
        Hits += Direction;
      }
    }

    Current = PeekTokenRawCursor(&Current, Direction);
  }

  // TODO(Jesse): Pack the conditional up above into a function we can call
  // here.. ?
  if (IsValid(&Current) && (Current.At->Erased || IsWhitespace(Current.At) || IsComment(Current.At))) { Invalidate(&Current); } // Fires if the stream ends with whitespace/comment


#if 0
#if BONSAI_INTERNAL
  if (Current.At && DEBUG_CHECK_FOR_BREAK_HERE(*Current.At))
  {
    Current.At = PeekTokenPointer(Tokens, Lookahead+1);
  }

  if (Current.At) { Assert(!StringsMatch(Current.At->Value, CSz("break_here"))); }
#endif
#endif

  SanityCheckCTokenCursor(Tokens);
  return Current;
}

link_internal peek_result
PeekTokenCursor(parser *Parser, s32 Skip)
{
  peek_result Result = {};
  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    Result = PeekTokenCursor(Parser->Tokens, Skip);
  }
  return Result;
}

link_internal c_token*
PeekTokenPointer(parser* Parser, s32 Skip)
{
  c_token *Result = 0;
  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    peek_result C = PeekTokenCursor(Parser->Tokens, Skip);
    if (IsValid(&C)) Result = C.At;
  }
  return Result;
}

link_internal c_token*
PeekTokenPointer(parser* Parser, u32 Skip)
{
  c_token *Result = PeekTokenPointer(Parser, (s32)Skip);
  return Result;
}

#if 0
link_internal c_token*
PeekTokenPointer(parser* Parser, u32 Lookahead)
{
  u32 TokenHits = 0;
  u32 LocalLookahead = 0;

  c_token* Result = PeekTokenRawPointer(Parser, LocalLookahead);
  while (Result)
  {
    if ( Result->Erased )
    {
    }
    else if ( Result->Type == CTokenType_CommentSingleLine)
    {
    }
    else if ( Result->Type == CTokenType_CommentMultiLine)
    {
    }
    else if (IsWhitespace(Result))
    {
    }
    else
    {
      if (TokenHits++ == Lookahead)
      {
        break;
      }
    }

    ++LocalLookahead;
    Result = PeekTokenRawPointer(Parser, LocalLookahead);
  }

  if (Result && (IsWhitespace(Result) || IsComment(Result))) { Result = 0; } // Fires if the stream ends with whitespace/comment

#if 0
#if BONSAI_INTERNAL
  if (Result && DEBUG_CHECK_FOR_BREAK_HERE(*Result))
  {
    Result = PeekTokenPointer(Parser, Lookahead+1);
  }

  if (Result) { Assert(!StringsMatch(Result->Value, CSz("break_here"))); }
#endif
#endif

  return Result;
}
#endif

link_internal c_token
PeekToken(parser* Parser, s32 Lookahead)
{
  c_token* Pointer = PeekTokenPointer(Parser, Lookahead);
  c_token Result = {};
  if (Pointer) Result = *Pointer;
  return Result;
}

link_internal c_token *
PopTokenRawPointer(parser* Parser)
{
  peek_result This = PeekTokenRawCursor(Parser);
  c_token *Result = 0;

  if (IsValid(&This))
  {
    Result = AdvanceTo(Parser, &This);
  }

  peek_result Next = PeekTokenRawCursor(Parser, 1);
  if (IsValid(&Next))
  {
    Result = AdvanceTo(Parser, &Next);
  }
  else
  {
    Invalidate(Parser->Tokens);
  }

#if BONSAI_INTERNAL
  if (Result && DEBUG_CHECK_FOR_BREAK_HERE(Result))
  {
    RuntimeBreak();
    AdvanceParser(Parser);
    Result = PopTokenRawPointer(Parser);
  }

  if (Result) { Assert(!StringsMatch(Result->Value, CSz("break_here"))); }
#endif

  return Result;
}

link_internal c_token
PopTokenRaw(parser* Parser)
{
  c_token *T = 0;

  peek_result Peek = PeekTokenRawCursor(Parser, 1);
  T = AdvanceTo(Parser, &Peek);

  c_token Result = {};
  if (T) Result = *T;

#if BONSAI_INTERNAL
  if (DEBUG_CHECK_FOR_BREAK_HERE(&Result))
  {
    RuntimeBreak();
    if (PeekTokenRawPointer(Parser)) { AdvanceParser(Parser); }
    Result = PopTokenRaw(Parser);
  }

  Assert(!StringsMatch(Result.Value, CSz("break_here")));
#endif

  return Result;
}

link_internal b32
RawTokensRemain(parser *Parser, u32 Count)
{
  b32 Result = PeekTokenRawPointer(Parser, Count) != 0;
  return Result;
}

link_internal b32
TokensRemain(parser *Parser, u32 Count)
{
  b32 Result = PeekTokenPointer(Parser, Count) != 0;
  return Result;
}

link_internal c_token *
PopTokenPointer(parser* Parser)
{
  // TODO(Jesse): This is kinda tortured .. should probably work on the API
  // here. In particular it's not obvious what AdvanceTo returns, and it
  // actually returns what I wouldn't expect if I guessed.
  //
  // Furthermore the first AdvanceTo is necessary because of the return value
  // weirdness, while it shouldn't be necessary if the API was better.
  //
  // UPDATE(Jesse): Now we `should` be able to just do `AdvanceTo(NextRawT)`
  // without the intermediate AdvanceTo .. I think
  //

  peek_result NextT = PeekTokenCursor(Parser);
  AdvanceTo(Parser, &NextT);

  peek_result NextRawT = PeekTokenRawCursor(&NextT, 1);
  c_token *Result = AdvanceTo(Parser, &NextRawT);

#if BONSAI_INTERNAL
  if (DEBUG_CHECK_FOR_BREAK_HERE(Result))
  {
    RuntimeBreak();
    if (PeekTokenRawPointer(Parser)) { AdvanceParser(Parser); }
    Result = PopTokenPointer(Parser);
  }
  else if (Result)
  {
    Assert(!StringsMatch(Result->Value, CSz("break_here")));
  }

#endif

  return Result;
}
link_internal c_token
PopToken(parser* Parser)
{
  c_token *Pop = PopTokenPointer(Parser);

  c_token Result = {};
  if (Pop) Result = *Pop;

  return Result;
}

link_internal b32
OptionalTokenRaw(parser* Parser, c_token_type Type)
{
  b32 Result = False;
  c_token Peeked = PeekTokenRaw(Parser);
  if (Peeked.Type == Type)
  {
    Result = True;
    c_token Popped = PopTokenRaw(Parser);
    Assert(Popped.Type == Peeked.Type);
    Assert(Popped.Type == Type);
  }

  return Result;
}

link_internal b32
OptionalTokenRaw(parser* Parser, c_token T)
{
  b32 Result = False;
  c_token Peeked = PeekTokenRaw(Parser);
  if (Peeked == T)
  {
    Result = True;
    c_token Popped = PopTokenRaw(Parser);
    Assert(Popped == Peeked);
  }

  return Result;
}

// TODO(Jesse): Optimize by calling Advance() instead of RequireToken()
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token *
OptionalToken(parser* Parser, c_token T)
{
  c_token *Result = PeekTokenPointer(Parser);
  if (Result && *Result == T) { RequireToken(Parser, *Result); } else { Result = 0;}
  return Result;
}

// TODO(Jesse): Optimize by calling Advance() instead of RequireToken()
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token *
OptionalToken(parser* Parser, c_token_type Type)
{
  c_token *Result = PeekTokenPointer(Parser);
  if (Result && Result->Type == Type) { RequireToken(Parser, *Result); } else { Result = 0; }
  return Result;
}

// TODO(Jesse): Could be optimized significanly utilizing new routines.
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token
RequireToken(parser* Parser, c_token ExpectedToken)
{
  c_token Result = {};

  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    c_token* PeekedToken = PeekTokenPointer(Parser);
    if (PeekedToken)
    {
      Result = *PeekedToken;
    }
    else
    {
      PeekedToken = PeekTokenPointer(Parser, -1);
    }


    // TODO(Jesse, id: 348, tags: immediate, id_347) : This should go into an AreEqual bonsai_function I think..
    if ( Result.Type != ExpectedToken.Type ||
         (ExpectedToken.Value.Count > 0 && !StringsMatch(ExpectedToken.Value, Result.Value) ))
    {
      ParseError_RequireTokenFailed( Parser, CSz("RequireToken"), PeekedToken, &ExpectedToken );
    }
    else
    {
      Result = PopToken(Parser);
    }

#if BONSAI_INTERNAL
    Assert(!StringsMatch(Result.Value, CSz("break_here")));
#endif
  }

  return Result;
}

// TODO(Jesse): This function should likely be primal for the sake of efficiency
link_internal c_token
RequireToken(parser* Parser, c_token *ExpectedToken)
{
  c_token Result = {};
  if (ExpectedToken)
  {
    Result = RequireToken(Parser, *ExpectedToken);
  }
  else
  {
    Error("Internal Compiler Error : RequireToken was passed ptr(0)");
  }
  return Result;
}

link_internal c_token
RequireToken(parser* Parser, c_token_type ExpectedType)
{
  c_token Result = RequireToken(Parser, CToken(ExpectedType));
  return Result;
}

link_internal c_token *
RequireTokenPointer(parser* Parser, c_token *ExpectedToken)
{
  c_token *Result = {};

  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    c_token* PeekedToken = PeekTokenPointer(Parser);

    // TODO(Jesse, id: 348, tags: immediate, id_347) : This should go into an AreEqual bonsai_function I think..
    if (PeekedToken &&
         (PeekedToken->Type != ExpectedToken->Type ||
         (ExpectedToken->Value.Count > 0 && !StringsMatch(ExpectedToken->Value, PeekedToken->Value) ))
       )
    {
      ParseError_RequireTokenFailed( Parser, CSz("RequireToken"), PeekedToken, ExpectedToken );
    }
    else
    {
      Result = PopTokenPointer(Parser);
      Assert(Result == PeekedToken);
    }

#if BONSAI_INTERNAL
    if (Result) { Assert(!StringsMatch(Result->Value, CSz("break_here"))); }
#endif
  }

  return Result;
}

link_internal c_token*
RequireTokenPointer(parser* Parser, c_token Expected)
{
  c_token *Result = RequireTokenPointer(Parser, &Expected);
  return Result;
}

link_internal c_token*
RequireTokenPointer(parser* Parser, c_token_type ExpectedType)
{
  c_token *Result = RequireTokenPointer(Parser, CToken(ExpectedType));
  return Result;
}

// @duplicated_require_token_raw
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token
RequireTokenRaw(parser *Parser, c_token Expected )
{
  c_token *Peek = PeekTokenRawPointer(Parser);
  if (Peek && *Peek == Expected)  // TODO(Jesse id: 349, tags: id_347) : Change to a function call instead of == operator
  {
    PopTokenRaw(Parser);
  }
  else
  {
    ParseError_RequireTokenFailed(Parser, CSz("RequireTokenRaw"), Peek, &Expected);
  }

  c_token Result = {};
  if (Peek) Result = *Peek;
  return Result;
}

link_internal c_token
RequireTokenRaw(parser *Parser, c_token *Expected )
{
  Assert(Expected);
  c_token Result = RequireTokenRaw(Parser, *Expected);
  return Result;
}

// @duplicated_require_token_raw
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token
RequireTokenRaw(parser *Parser, c_token_type Expected )
{
  c_token *Peek = PeekTokenRawPointer(Parser);
  if (Peek && Peek->Type == Expected)  // TODO(Jesse tags:id_347) : Change to a function call instead of == operator
  {
    PopTokenRaw(Parser);
  }
  else
  {
    c_token Temp = CToken(Expected);
    ParseError_RequireTokenFailed(Parser, CSz("RequireTokenRaw"), Peek, &Temp);
  }

  c_token Result = {};
  if (Peek) Result = *Peek;
  return Result;
}

link_internal b32
TokenIsOperator(c_token_type T)
{
  b32 Result = False;
  switch (T)
  {
    case CTokenType_Ampersand:
    case CTokenType_Tilde:
    case CTokenType_Hat:
    case CTokenType_LT:
    case CTokenType_GT:
    case CTokenType_Bang:
    case CTokenType_LeftShift:
    case CTokenType_RightShift:
    case CTokenType_LessEqual:
    case CTokenType_GreaterEqual:
    case CTokenType_AreEqual:
    case CTokenType_NotEqual:
    case CTokenType_PlusEquals:
    case CTokenType_MinusEquals:
    case CTokenType_TimesEquals:
    case CTokenType_DivEquals:
    case CTokenType_ModEquals:
    case CTokenType_AndEquals:
    case CTokenType_OrEquals:
    case CTokenType_XorEquals:
    case CTokenType_Increment:
    case CTokenType_Decrement:
    case CTokenType_LogicalAnd:
    case CTokenType_LogicalOr:
    case CTokenType_Percent:
    case CTokenType_Pipe:
    case CTokenType_Plus:
    case CTokenType_Minus:
    case CTokenType_FSlash:
    case CTokenType_Question:
    case CTokenType_Star:
    case CTokenType_OpenBracket:
    case CTokenType_Arrow:
    case CTokenType_Dot:
    {
      Result = True;
    } break;

    default: {} break;
  }

  return Result;
}

link_internal b32
NextTokenIsOperator(parser* Parser)
{
  c_token T = PeekToken(Parser);
  b32 Result = TokenIsOperator(T.Type);
  return Result;
}

link_internal c_token *
RequireOperatorToken(parser* Parser)
{
  c_token *Result = PeekTokenPointer(Parser);
  if (Result)
  {
    switch (Result->Type)
    {
      case CTokenType_Ampersand:
      case CTokenType_Tilde:
      case CTokenType_Hat:
      case CTokenType_LT:
      case CTokenType_GT:
      case CTokenType_Bang:
      case CTokenType_LeftShift:
      case CTokenType_RightShift:
      case CTokenType_LessEqual:
      case CTokenType_GreaterEqual:
      case CTokenType_AreEqual:
      case CTokenType_NotEqual:
      case CTokenType_PlusEquals:
      case CTokenType_MinusEquals:
      case CTokenType_TimesEquals:
      case CTokenType_DivEquals:
      case CTokenType_ModEquals:
      case CTokenType_AndEquals:
      case CTokenType_OrEquals:
      case CTokenType_XorEquals:
      case CTokenType_Increment:
      case CTokenType_Decrement:
      case CTokenType_LogicalAnd:
      case CTokenType_LogicalOr:
      case CTokenType_Percent:
      case CTokenType_Pipe:
      case CTokenType_Plus:
      case CTokenType_Minus:
      case CTokenType_FSlash:
      case CTokenType_Question:
      case CTokenType_Star:
      case CTokenType_Arrow:
      case CTokenType_Dot:
      case CTokenType_Bool:
      case CTokenType_Equals: // Assignment
      {
        RequireToken(Parser, Result->Type);
      } break;

      case CTokenType_OpenBracket:
      {
        RequireToken(Parser, Result->Type);
        RequireToken(Parser, CTokenType_CloseBracket);
      } break;

      default:
      {
        ParseError(Parser, FormatCountedString(GetTranArena(), CSz("Expected operator, got %S(%S)"), ToString(Result->Type), Result->Value), Result);
        Result = 0;
      } break;
    }
  }

  return Result;
}



