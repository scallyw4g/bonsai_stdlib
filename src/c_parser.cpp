poof(hashtable(parser))
#include <generated/hashtable_parser.h>

global_variable counted_string_stream Global_ErrorStream = {};

//
// Error Reporting
//
link_internal void ParseError(parser* Parser, parse_error_code ErrorCode, counted_string ErrorMessage, c_token* ErrorToken = 0);
link_internal void ParseError(parser* Parser, counted_string ErrorMessage, c_token* ErrorToken = 0);


//
// Utility
//

// NOTE(Jesse): This transmute is special; applications can choose to override
// this transmute function.  Additional transmute functions specified in poof.cpp
link_weak b32 TryTransmuteKeywordToken(c_token *T, c_token *LastTokenPushed);

link_internal parser *         DuplicateParserTokens(parser *Parser, memory_arena *Memory);
link_internal parser *         DuplicateParser(parser *Parser, memory_arena *Memory);
link_internal c_token_cursor * DuplicateCTokenCursor(c_token_cursor *Tokens, memory_arena *Memory);
link_internal parser *         DuplicateCTokenCursor2(c_token_cursor *Tokens, memory_arena *Memory);

link_internal void DumpLocalTokens(parser *Parser);
link_internal void PrintTray(char_cursor *Dest, c_token *T, u32 Columns, counted_string Color);

link_internal b32       TokenIsOperator(c_token *T);
link_internal b32       NextTokenIsOperator(parser *Parser);
link_internal c_token * RequireOperatorToken(parser *Parser);


//
// Token Control
//

link_internal void FullRewind(parser* Parser);

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

link_internal void TrimFirstToken(parser* Parser, c_token_type TokenType);
link_internal void TrimLastToken(parser* Parser, c_token_type TokenType);
link_internal void TrimLeadingWhitespace(parser* Parser);

link_internal counted_string EatBetweenExcluding(ansi_stream*, char Open, char Close);
link_internal void           EatBetween(parser* Parser, c_token_type Open, c_token_type Close);
link_internal counted_string EatBetween_Str(parser* Parser, c_token_type Open, c_token_type Close);
link_internal b32            EatWhitespace(parser* Parser);
link_internal b32            EatSpacesTabsAndEscapedNewlines(parser *Parser);
link_internal void           EatWhitespaceAndComments(parser *Parser);


inline c_token
CToken(r32 FloatValue)
{
  c_token Result = {};
  Result.Type = CTokenType_Float;
  Result.Value = FormatCountedString(GetTranArena(), CSz("%f"), r64(FloatValue)); // TODO(Jesse id: 350, tags: memory_leak)
  Result.FloatValue = r64(FloatValue);
  return Result;
}

inline c_token
CToken(s32 Value)
{
  c_token Result = {};
  Result.Type = CTokenType_IntLiteral;
  Result.Value = FormatCountedString(GetTranArena(), CSz("%s"), Value); // TODO(Jesse id: 351, tags: memory_leak)
  Result.as_s64 = s64(Value);
  Result.Flags = CTFlags_Signed;
  return Result;
}

inline c_token
CToken(u32 Value)
{
  c_token Result = {};
  Result.Type = CTokenType_IntLiteral;
  Result.Value = FormatCountedString(GetTranArena(), CSz("%u"), Value); // TODO(Jesse id: 351, tags: memory_leak)
  Result.as_u64 = u64(Value);
  return Result;
}

link_internal c_token_cursor
CTokenCursor(c_token_buffer *Buf, counted_string Filename, token_cursor_source Source, c_token_cursor_up Up)
{
  c_token_cursor Result = CTokenCursor(Buf->Start, Buf->Start + Buf->Count, Filename, Source, Up);
  return Result;
}

inline void
CTokenCursor(c_token_cursor *Result, umm Count, memory_arena *Memory, counted_string Filename, token_cursor_source Source, c_token_cursor_up Up)
{
  c_token *Buffer = AllocateProtection(c_token, Memory, Count, False);
  CTokenCursor(Result, Buffer, Count, Filename, Source, Up);
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
        Result.Type = (c_token_type)At;
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
      Warn("Unable to call FinalizeStringFromParser due to having spanned a parser chain link.");
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








link_internal string_from_parser
StartParserFromParser(parser* Parser)
{
  string_from_parser Result = StartStringFromParser(Parser);
  return Result;
}

link_internal parser
FinalizeParserFromParser(string_from_parser *StringFromParser, memory_arena *Memory)
{
  c_token *Start = StringFromParser->StartToken;
  c_token *End   = StringFromParser->Parser->Tokens->At;

  c_token_cursor *Tokens = Allocate(c_token_cursor, Memory, 1);

  CTokenCursor( Tokens, Start, 0, CSz("(anonymous parser)"), TokenCursorSource_IntermediateRepresentaton, {0,0} );

  parser Result = {};
  Result.Tokens = Tokens;
  Result.Tokens->End  = End;
  return Result;
}











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

link_internal parser
EatUntilExcluding_Parser(parser* Parser, c_token_type Close, memory_arena *Memory)
{
  auto Start = StartParserFromParser(Parser);
  EatUntilExcluding(Parser, Close);
  auto Result = FinalizeParserFromParser(&Start, Memory);
  return Result;
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
  // NOTE(Jesse): This is mega brain-damaged, but the FormatCountedString
  // overload that was responsible for this path got deleted because it was
  // confusing the compiler and causing a crash.  This kinda makes me want to 
  // put it back in, but not for just this single case..
  if (T)
  {
    cs S = FormatCountedString_(Dest->At, Remaining(Dest), CSz("%*d |"), Columns, T->LineNumber);
    Dest->At += S.Count;
  }
  else
  {
    cs S = FormatCountedString_(Dest->At, Remaining(Dest), CSz("%*c %S|%S"), Columns, ' ', Color, TerminalColors.White);
    Dest->At += S.Count;
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
    counted_string NameLine = FormatCountedString(GetTranArena(), CSz("./%S:%u:0: %S "), ParserName, ErrorLineNumber, Message);
    u64 LongestLine = Max(MinLineLen, GetLongestLineInCursor(ParseErrorCursor));
    LongestLine = Max(MinLineLen, (u64)NameLine.Count+4);

    string_builder Builder = {};

    /* u64 HalfDashes = (LongestLine-NameLine.Count)/2; */
    /* for (u32 DashIndex = 0; DashIndex < HalfDashes; ++DashIndex) { Append(&Builder, CSz("-")); } */
    Append(&Builder, NameLine);
    /* for (u32 DashIndex = 0; DashIndex < HalfDashes; ++DashIndex) { Append(&Builder, CSz("-")); } */
    Append(&Builder, CSz("\n"));

    // Intentional copy
    counted_string ErrorText = CopyString(CS(ParseErrorCursor), GetTranArena());
    Append(&Builder, ErrorText);
    /* LogDirect("%S", CS(ParseErrorCursor)); */

    counted_string FullErrorText = Finalize(&Builder, GetTranArena());
    Push(&Global_ErrorStream, FullErrorText);

    if (Global_LogLevel <= LogLevel_Error)
    {
      LogDirect(CSz("%S"), FullErrorText);
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
    LogDirect(CSz("Error determining where the error occurred \n"));
    LogDirect(CSz("Error message was : (%S) \n"), Message);
  }

  return;
}

link_internal void
ParseInfoMessage(parser* Parser, counted_string Message, c_token* T)
{
  OutputContextMessage(Parser, ParseErrorCode_None, CSz(""), Message, T);
}

link_internal void
ParseWarn(parser* Parser, counted_string ErrorMessage, c_token* ErrorToken)
{
  Parser->WarnCode = ParseWarnCode_Generic;
  OutputContextMessage(Parser, ParseErrorCode_None, CSz("Warning"), ErrorMessage, ErrorToken);
}

link_internal void
ParseWarn(parser* Parser, parse_warn_code WarnCode, counted_string ErrorMessage, c_token* ErrorToken)
{
  Parser->WarnCode = WarnCode;
  OutputContextMessage(Parser, ParseErrorCode_None, CSz("Warning"), ErrorMessage, ErrorToken);
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
  counted_string Result = FormatCountedString(GetTranArena(), CSz("Stream ended unexpectedly in file : %S"), ?->Filename);
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
      if (Down) { Assert(Result.At->Down == Result.At->Macro.Expansion); } // @janky-macro-expansion-struct-ordering

      b32 SearchDown = CanSearchDown && Down && RemainingForDir(Down, Direction) > 0;
      if (SearchDown)
      {

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
      if (Down) { Assert(Result.At->Down == Result.At->Macro.Expansion); } // @janky-macro-expansion-struct-ordering

      b32 SearchDown = CanSearchDown && Down; // && RemainingForDir(Down, Direction) > 0;
      if (SearchDown)
      {

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

  c_token *Result = 0;
  peek_result NextT = PeekTokenCursor(Parser);
  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    AdvanceTo(Parser, &NextT);

    peek_result NextRawT = PeekTokenRawCursor(&NextT, 1);
    Result = AdvanceTo(Parser, &NextRawT);

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
  }

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

link_internal c_token *
OptionalTokenRaw(parser *Parser, c_token T)
{
  c_token *Result = {};
  c_token *Peeked = PeekTokenRawPointer(Parser);
  if (Peeked && *Peeked == T)
  {
    Result = PopTokenRawPointer(Parser);
    Assert(Result == Peeked);
  }

  return Result;
}

// TODO(Jesse): Optimize by calling Advance() instead of RequireToken()
// @optimize_call_advance_instead_of_being_dumb
link_internal c_token *
OptionalToken(parser *Parser, c_token T)
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


// TODO(Jesse): Rewrite RequireTokenRaw in terms of this
link_internal c_token*
RequireTokenRawPointer(parser *Parser, c_token Expected )
{
  c_token *Peek = PeekTokenRawPointer(Parser);
  if (Peek && *Peek == Expected)  // TODO(Jesse)(id_349) : Change to a function call instead of == operator
  {
    PopTokenRaw(Parser);
  }
  else
  {
    ParseError_RequireTokenFailed(Parser, CSz("RequireTokenRaw"), Peek, &Expected);
  }

  c_token *Result = Peek;
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

// TODO(Jesse): Rewrite RequireTokenRaw in terms of this
link_internal c_token*
RequireTokenRawPointer(parser *Parser, c_token_type Expected )
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

  c_token *Result = Peek;
  return Result;
}

link_internal b32
TokenIsOperator(c_token *T)
{
  b32 Result = False;
  if (T)
  {
    switch (T->Type)
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

      case CTokenType_IntLiteral:
      {
        if (T->Flags & CTFlags_Signed) { Result = (T->as_s64 < 0); }
      } break;

      default: {} break;
    }
  }

  return Result;
}

link_internal b32
NextTokenIsOperator(parser* Parser)
{
  c_token *T = PeekTokenPointer(Parser);
  b32 Result = TokenIsOperator(T);
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

link_internal void
RequireToken(ansi_stream *Code, char C)
{
  Assert(*Code->At == C);
  Advance(Code);
}

link_internal counted_string
EatBetweenExcluding(ansi_stream *Code, char Open, char Close)
{
  u32 Depth = 0;
  RequireToken(Code, Open);

  const char* Start = Code->At;

  b32 Success = False;
  while (Remaining(Code))
  {
    char T = Advance(Code);

    if (T == Open)
    {
      ++Depth;
    }

    if (T == Close)
    {
      if (Depth == 0)
      {
        Success = True;
        break;
      }
      --Depth;
    }
  }

  if (!Success)
  {
    InvalidCodePath();
    /* ParseError(Code, FormatCountedString(GetTranArena(), CSz("Unable to find closing token %S"), ToString(Close))); */
  }

  Assert(Code->At-1 >= Code->Start);
  const char* End = Code->At-1;

  umm Count = (umm)(End-Start);

  counted_string Result = {
    .Count = Count,
    .Start = Start,
  };

  return Result;
}

// TODO(Jesse): Rewrite in terms of EatBetweenExcluding
link_internal void
EatBetween(parser* Parser, c_token_type Open, c_token_type Close)
{
  b32 Success = False;

  if (Parser->ErrorCode == ParseErrorCode_None)
  {
    u32 Depth = 0;
    c_token *TErr = RequireTokenPointer(Parser, Open);

    while ( c_token *T = PopTokenPointer(Parser) )
    {
      if (T->Type == Open)
      {
        ++Depth;
      }

      if (T->Type == Close)
      {
        if (Depth == 0)
        {
          Success = True;
          break;
        }
        --Depth;
      }
    }

    if (!Success)
    {
      ParseError(Parser, FormatCountedString(GetTranArena(), CSz("Unable to find closing token %S"), ToString(Close)), TErr);
    }
  }
}

link_internal counted_string
EatBetweenExcluding_Str(parser* Parser, c_token_type Open, c_token_type Close)
{
  counted_string Result = {};

  EatWhitespace(Parser);
  string_from_parser Builder = StartStringFromParser(Parser);
  EatBetween(Parser, Open, Close);
  Result = FinalizeStringFromParser(&Builder);

  if (Result.Count > 1)
  {
    Assert(Result.Start);
    Assert(Result.Start[0] == char(Open));
    Assert(Result.Start[Result.Count-1] == char(Close));
    Result.Count -= 2;
    Result.Start++;
  }

  return Result;
}

link_internal counted_string
EatBetween_Str(parser* Parser, c_token_type Open, c_token_type Close)
{
  counted_string Result = {};

  string_from_parser Builder = StartStringFromParser(Parser);
  EatBetween(Parser, Open, Close);
  Result = FinalizeStringFromParser(&Builder);

  return Result;
}


link_internal parser
EatBetween_Parser(parser *Parser, c_token_type Open, c_token_type Close, memory_arena *Memory)
{
  auto Start = StartParserFromParser(Parser);
  EatBetween(Parser, Open, Close);
  parser Result = FinalizeParserFromParser(&Start, Memory);
  return Result;
}

link_internal parser
EatBetweenExcluding_Parser(parser *Parser, c_token_type Open, c_token_type Close, memory_arena *Memory)
{
  parser Result = EatBetween_Parser(Parser, Open, Close, Memory);
  TrimFirstToken(&Result, Open);
  TrimLastToken(&Result, Close);
  return Result;
}

link_internal void
EatBetweenRaw(parser* Parser, c_token_type Open, c_token_type Close)
{
  u32 Depth = 0;
  RequireTokenRaw(Parser, Open);

  b32 Success = False;
  while (RawTokensRemain(Parser))
  {
    c_token T = PopTokenRaw(Parser);

    if (T.Type == Open)
    {
      ++Depth;
    }

    if (T.Type == Close)
    {
      if (Depth == 0)
      {
        Success = True;
        break;
      }
      --Depth;
    }
  }

  if (!Success)
  {
    ParseError(Parser, FormatCountedString(GetTranArena(), CSz("Unable to find closing token %S"), ToString(Close)));
  }

  return;
}

link_internal void
EatUntil_TrackingDepth(parser *Parser, c_token_type Open, c_token_type Close, c_token *StartToken)
{
  u32 Depth = 0;
  b32 Success = False;
  while ( c_token *T = PopTokenPointer(Parser) )
  {

    if (T->Type == Open)
    {
      ++Depth;
    }

    if (T->Type == Close)
    {
      if (Depth == 0)
      {
        Success = True;
        break;
      }
      --Depth;
    }
  }

  if (!Success)
  {
    ParseError(Parser, FormatCountedString(GetTranArena(), CSz("Unable to find closing token %S"), ToString(Close)), StartToken);
  }

  return;
}






/*************************                         ***************************/
/*************************  Comments & Whitespace  ***************************/
/*************************                         ***************************/


link_internal b32
EatComment(parser* Parser, c_token_type CommentT)
{
  b32 Result = False;
  switch (CommentT)
  {
    case CTokenType_CommentSingleLine:
    {
      Result = OptionalTokenRaw(Parser, CTokenType_CommentSingleLine);
    } break;

    case CTokenType_CommentMultiLine:
    {
      Result = OptionalTokenRaw(Parser, CTokenType_CommentMultiLine);
    } break;

    default:
    {
    } break;
  }

  return Result;
}

link_internal b32
EatComment(parser* Parser)
{
  b32 Result = EatComment(Parser, PeekTokenRaw(Parser).Type);
  return Result;
}

link_internal void
EatSpacesTabsEscapedNewlinesAndComments(parser *Parser)
{
  b32 Continue = true;
  while (Continue)
  {
    b32 AteWhitespace = EatSpacesTabsAndEscapedNewlines(Parser);
    b32 AteComment = EatComment(Parser);
    Continue = AteWhitespace || AteComment;
  }
}

// @optimize_call_advance_instead_of_being_dumb
link_internal void
EatWhitespaceAndComments(parser *Parser)
{
  TIMED_FUNCTION();

  while ( c_token *T = PeekTokenRawPointer(Parser) )
  {
    if (IsWhitespace(T))
    {
      PopTokenRawPointer(Parser);
    }
    else if (IsComment(T))
    {
      EatComment(Parser);
    }
    else
    {
      break;
    }
  }
}

// NOTE(Jesse): This is duplicated @duplicate_EatSpacesTabsAndEscapedNewlines
// NOTE(Jesse): This could also be optimized using PeekTokenRawPointer now
link_internal b32
EatSpacesTabsAndEscapedNewlines(parser *Parser)
{
  b32 Result = False;

  c_token_type Type = PeekTokenRaw(Parser).Type;
  while ( RawTokensRemain(Parser) &&
          ( Type == CTokenType_Space            ||
            Type == CTokenType_Tab              ||
            Type == CTokenType_EscapedNewline   ||
            Type == CTokenType_FSlash ) )
  {
    Result = True;
    RequireTokenRaw(Parser, Type);
    Type = PeekTokenRaw(Parser).Type;
  }

  return Result;
}

// NOTE(Jesse): We could metapgrogram these routines if we had a feature for it
// @meta_similar_whitespace_routines
//
// @optimize_call_advance_instead_of_being_dumb
link_internal b32
EatNBSP(parser* Parser)
{
  b32 Result = False;

  c_token *T = PeekTokenRawPointer(Parser);
  while (T && IsNBSP(T))
  {
    Result = True;
    PopTokenRawPointer(Parser);
    T = PeekTokenRawPointer(Parser);
  }

  return Result;
}

// @meta_similar_whitespace_routines
//
// @optimize_call_advance_instead_of_being_dumb
link_internal b32
EatWhitespace(parser* Parser)
{
  b32 Result = False;

  c_token *T = PeekTokenRawPointer(Parser);
  while (T && IsWhitespace(T))
  {
    Result = True;
    PopTokenRawPointer(Parser);
    T = PeekTokenRawPointer(Parser);
  }

  return Result;
}

// NOTE(Jesse): This is the same as EatWhitespace but it doesn't return a bool
link_internal void
TrimLeadingWhitespace(parser* Parser)
{
  c_token *T = PeekTokenRawPointer(Parser);
  while (T && IsWhitespace(T))
  {
    PopTokenRawPointer(Parser);
    T = PeekTokenRawPointer(Parser);
  }
}

link_internal c_token
LastNonNBSPToken(parser* Parser)
{
  c_token Result = {};
  c_token* CurrentToken = Parser->Tokens->End-1;

  while (CurrentToken >= Parser->Tokens->Start)
  {
    // TODO(Jesse)(correctness) This function fails if we hit one of these!
    // Rewrite it such that we properly traverse "Down" pointers.
    if( CurrentToken->Type == CT_InsertedCode ||
        CurrentToken->Type == CT_MacroLiteral  )
    {
      OutputContextMessage( Parser, ParseErrorCode_NotImplemented, CSz("Fffff"), CSz("Last NBSP token was a macro literal; we do not currently handle this case."), CurrentToken);
    }

    if ( IsNBSP(CurrentToken) )
    {
      --CurrentToken;
    }
    else
    {
      break;
    }
  }

  if (CurrentToken >= Parser->Tokens->Start && !IsNBSP(CurrentToken) )
  {
    Result = *CurrentToken;
  }
  return Result;
}

link_internal void
TrimTrailingNBSP(parser* Parser)
{
  c_token* CurrentToken = Parser->Tokens->End-1;

  while (CurrentToken > Parser->Tokens->Start)
  {
    // TODO(Jesse)(correctness) This function fails if we hit one of these!
    // Rewrite it such that we properly traverse "Down" pointers.
    /* Assert( ! (Parser->Tokens->At->Type == CT_InsertedCode || */
    /*            Parser->Tokens->At->Type == CT_MacroLiteral) ); */

    if ( CurrentToken->Type == CTokenType_Space ||
         CurrentToken->Type == CTokenType_Tab )
    {
      Parser->Tokens->End = CurrentToken;
      if (Parser->Tokens->At > CurrentToken)
      {
        Parser->Tokens->At = CurrentToken;
      }
    }
    else
    {
      break;
    }

    --CurrentToken;
  }
}

// @duplicate_EatSpacesTabsAndEscapedNewlines
link_internal u32
EatSpacesTabsAndEscapedNewlines(ansi_stream *Code)
{
  u32 LinesEaten = 0;
  c_token_type Type = PeekToken(Code).Type;
  while ( Type == CTokenType_Space ||
          Type == CTokenType_Tab   ||
          Type == CTokenType_FSlash )
  {
    if ( Type == CTokenType_FSlash )
    {
      if ( PeekToken(Code, 1).Type == CTokenType_Newline )
      {
        ++LinesEaten;
        Advance(Code);
        Advance(Code);
      }
      else
      {
        break;
      }
    }
    else
    {
      Advance(Code);
    }

    Type = PeekToken(Code).Type;
  }
  return LinesEaten;
}

link_internal b32
NextTokenIsSpaceOrTab(parser *Parser)
{
  b32 Result = PeekTokenRaw(Parser).Type == CTokenType_Space ||
               PeekTokenRaw(Parser).Type == CTokenType_Tab;
  return Result;
}

link_internal void
TrimNBSPUntilNewline(parser* Parser)
{
  Assert(Parser->Tokens->At == Parser->Tokens->Start);

  while (NextTokenIsSpaceOrTab(Parser)) { PopTokenRaw(Parser); }
  OptionalTokenRaw(Parser, CTokenType_Newline);
  Parser->Tokens->Start = Parser->Tokens->At;
}





// TODO(Jesse): Probably move at least some of these to counted_string.h ..?

/*******************************            **********************************/
/*******************************  Literals  **********************************/
/*******************************            **********************************/


link_internal counted_string
PopHex(ansi_stream* SourceFileStream)
{
  counted_string Result = {
    .Start = SourceFileStream->At,
    .Count = 0,
  };

  while (Remaining(SourceFileStream))
  {
    if (IsHex(*SourceFileStream->At))
    {
      ++SourceFileStream->At;
    }
    else
    {
      break;
    }
  }

  Result.Count = (umm)SourceFileStream->At - (umm)Result.Start;
  return Result;
}

link_internal counted_string
PopNumeric(ansi_stream* SourceFileStream)
{
  counted_string Result = {
    .Start = SourceFileStream->At,
    .Count = 0,
  };

  while (Remaining(SourceFileStream))
  {
    if (IsNumeric(*SourceFileStream->At))
    {
      ++SourceFileStream->At;
    }
    else
    {
      break;
    }
  }

  Result.Count = (umm)SourceFileStream->At - (umm)Result.Start;
  return Result;
}

link_internal counted_string
PopIdentifier(ansi_stream* SourceFileStream)
{
  counted_string Result = {
    .Start = SourceFileStream->At,
    .Count = 0,
  };

  while (Remaining(SourceFileStream))
  {
    c_token T = PeekToken(SourceFileStream);
    if (T.Type == CTokenType_Unknown)
    {
      ++SourceFileStream->At;
    }
    else
    {
      break;
    }
  }

  Result.Count = (umm)SourceFileStream->At - (umm)Result.Start;
  return Result;
}

link_internal r64
ToFractional(counted_string S)
{
  r64 Result = 0;
  if (S.Count) { Result = (r64)ToU64(S) * Exp(10.0, -SafeTruncateToS32(S.Count)); }
  Assert(Result < 1.0);
  return Result;
}

link_internal u64
BinaryStringToU64(counted_string Bin)
{
  u64 Result = 0;
  for (u64 CharIndex = 0;
      CharIndex < Bin.Count;
      ++CharIndex)
  {
    u64 Digit = (u64)(Bin.Start[CharIndex] - '0');
    Assert(Digit < 2);
    Result |= Digit << ((Bin.Count - CharIndex - 1L));
  }

  return Result;
}

link_internal u64
HexToU64(char C)
{
  u64 Result = 0;

  if (C >= '0' && C <= '9')
  {
    Result = ToU64(C);
  }
  else if (C >= 'a' && C <= 'f')
  {
    Result = (u64)(10 + C - 'a');
  }
  else if (C >= 'A' && C <= 'F')
  {
    Result = (u64)(10 + C - 'A');
  }
  else
  {
    InvalidCodePath();
  }

  Assert(Result < 16);
  return Result;
}

link_internal u64
HexStringToU64(counted_string S)
{
  u64 Result = 0;
  for (u64 CharIndex = 0;
      CharIndex < S.Count;
      ++CharIndex)
  {
    u64 Nibble = HexToU64(S.Start[CharIndex]);
    Result |= Nibble << ((S.Count - CharIndex - 1L) * 4);
  }

  return Result;
}

link_internal c_token
ParseExponentAndSuffixes(ansi_stream *Code, r64 OriginalValue)
{
  c_token Result = {};

  r64 FinalValue = OriginalValue;

  if ( Remaining(Code) &&
       (*Code->At == 'e' || *Code->At == 'E') )
  {
    Advance(Code);

    s32 Exponent = 1;

    char ExpSign = *Code->At;
    if (ExpSign == '-')
    {
      Advance(Code);
      Exponent = -1;
    }
    else if (ExpSign == '+')
    {
      Advance(Code);
      Exponent = 1;
    }

    Exponent = Exponent * SafeTruncateToS32(ToU64(PopNumeric(Code)));

    FinalValue = Exp(FinalValue, Exponent);
  }

  if (Remaining(Code))
  {
    char Suffix = *Code->At;
    switch (Suffix)
    {
      case 'f':
      case 'F':
      {
        Result.Type = CTokenType_FloatLiteral;
        Advance(Code);
      } break;

      case 'l':
      case 'L':
      {
        // Apparently `double` and `long double` are the same storage size (8 bytes), at least in MSVC:
        // https://docs.microsoft.com/en-us/cpp/c-language/storage-of-basic-types?view=vs-2019
        Result.Type = CTokenType_DoubleLiteral;
        Advance(Code);
      } break;

      default:
      {
        Result.Type = CTokenType_DoubleLiteral;
      } break;
    }
  }

  Result.FloatValue = FinalValue;
  return Result;
}

link_internal void
ParseIntegerSuffixes(ansi_stream *Code)
{
  b32 Done = False;
  while (!Done)
  {
    char Suffix = *Code->At;
    switch (Suffix)
    {
      // For now, we just eat suffixes
      // TODO(Jesse id: 278): Disallow invalid suffixes lul/LUL .. LUU .. ULLLL etc..
      // Maybe use a state machine / transition table
      case 'u':
      case 'U':
      case 'l':
      case 'L':
      {
        Advance(Code);
      } break;

      default:
      {
        Done = True;
      } break;
    }
  }
}

link_internal c_token
ParseNumericToken(ansi_stream *Code)
{
  c_token Result = {};
  Result.Type = CTokenType_IntLiteral;

  const char *Start = Code->At;

  b32 Negative = False;

  if (*Code->At == '-')
  {
    Advance(Code);
    Negative = True;
    Result.Flags |= CTFlags_Signed;
  }

  if (*Code->At == '.')
  {
    // This effectively skips parsing the integral portion of the float (if we typed .3f)
  }
  else
  {
    Assert(IsNumeric(*Code->At));
  }

  counted_string IntegralString = { .Start = Code->At, .Count = 0 };
  while (Remaining(Code) && IsNumeric(*Code->At)) { Advance(Code); }
  IntegralString.Count = (umm)(Code->At - IntegralString.Start);

  u64 IntegralPortion = ToU64(IntegralString);

  Result.as_u64 = IntegralPortion;

  if ( IntegralPortion == 0 &&
       (*Code->At == 'x'|| *Code->At == 'X') )
  {
    Advance(Code);
    Result.UnsignedValue = HexStringToU64(PopHex(Code));
    ParseIntegerSuffixes(Code);
  }
  else if (IntegralPortion == 0 && *Code->At == 'b')
  {
    Advance(Code);
    Result.UnsignedValue = BinaryStringToU64(PopNumeric(Code));
  }
  else if ( *Code->At == '.' )
  {
    // Float literal
    Advance(Code);
    r64 Fractional = ToFractional(PopNumeric(Code));
    Result = ParseExponentAndSuffixes(Code, (r64)IntegralPortion + Fractional);
  }
  else if ( *Code->At == 'e' || *Code->At == 'E'  )
  {
    // Float literal
    Result = ParseExponentAndSuffixes(Code, (r64)IntegralPortion);
  }
  else
  {
    //
    // Int literal
    //

    ParseIntegerSuffixes(Code);
  }

  if (Negative)
  {
    switch (Result.Type)
    {
      case CTokenType_IntLiteral:
      {
        Result.as_s64 *= -1;
      } break;

      case CTokenType_DoubleLiteral:
      case CTokenType_FloatLiteral:
      {
        Result.as_r64 *= -1;
      } break;

      InvalidDefaultCase;
    }
  }

  Result.Value.Start = Start;
  Result.Value.Count = (umm)(Code->At - Start);
  return Result;
}

link_internal counted_string
ParseIntegerConstant(parser* Parser)
{
  c_token T = PeekToken(Parser);

  const char* Start = T.Value.Start;
  u32 Count = 0;

  if (T.Type == CTokenType_Minus)
  {
    ++Count;
    RequireToken(Parser, CTokenType_Minus);
  }

  c_token NumberT = RequireToken(Parser, CTokenType_Identifier);
  if (IsNumeric(NumberT.Value))
  {
    Count += NumberT.Value.Count;
  }
  else
  {
    ParseError(Parser, CSz("Expected integer constant expression."));
  }

  counted_string Result = {
    .Count = Count,
    .Start = Start,
  };

  return Result;
}



/*******************************           **********************************/
/*******************************  Helpers  **********************************/
/*******************************           **********************************/


link_internal cs
ToString(parser *Parser, memory_arena *Memory)
{
  Rewind(Parser->Tokens);
  string_from_parser Builder = StartStringFromParser(Parser);

  Parser->Tokens->At = Parser->Tokens->End;

  cs Result = FinalizeStringFromParser(&Builder);
  return Result;
}

link_internal void
EraseToken(c_token *Token)
{
  Token->Erased = True;
}

link_internal void
EraseBetweenExcluding(parser *Parser, c_token *FirstToErase, c_token *OnePastLastToErase)
{
  b32 E0 = (FirstToErase > OnePastLastToErase);
  b32 E1 = (FirstToErase < Parser->Tokens->Start);
  b32 E2 = (FirstToErase >= Parser->Tokens->End);
  b32 E3 = (OnePastLastToErase <= Parser->Tokens->Start);
  b32 E4 = (OnePastLastToErase > Parser->Tokens->End);

  if (E0 || E1 || E2 || E3 || E4)
  {
    ParseError(Parser, CSz("Invalid token span passed to EraseBetweenExcluding"));
  }
  else
  {
    c_token *At = FirstToErase;
    while (At < OnePastLastToErase)
    {
      EraseToken(At++);
    }

  }

  return;
}

link_internal void
TrimFirstToken(parser* Parser, c_token_type TokenType)
{
  Assert(Parser->Tokens->At == Parser->Tokens->Start);
  RequireToken(Parser, TokenType);
  Parser->Tokens->Start = Parser->Tokens->At;
}

// TODO(Jesse): WTF?  I would assume this would just look at the last token and
// strip it.  Is this being used to trim whitespace as well or something?
link_internal void
TrimLastToken(parser* Parser, c_token_type TokenType)
{
  c_token* CurrentToken = Parser->Tokens->End-1;

  while (CurrentToken >= Parser->Tokens->Start)
  {
    if (CurrentToken->Type == TokenType)
    {
      Parser->Tokens->End = CurrentToken;
      break;
    }

    --CurrentToken;
  }
}

inline b32
TokenValidFor(c_token_cursor *Tokens, c_token *T)
{
  b32 Result = T >= Tokens->Start && T <= Tokens->End;
  return Result;
}

link_internal c_token
FirstNonNBSPToken(parser *Parser)
{
  peek_result At = PeekTokenRawCursor(Parser);
  while (IsValid(&At) && IsNBSP(At.At))
  {
    At = PeekTokenRawCursor(&At, 1);
  }
  c_token Result = {};
  if (IsValid(&At)) { Result = *At.At; }
  return Result;
}

link_internal u32
CountTokensBeforeNext(parser *Parser, c_token_type T1, c_token_type T2)
{
  Assert(T1 != T2);
  u32 Result = 0;

  c_token *ResetToStart = Parser->Tokens->At;

  for (;;)
  {
    c_token AtT = PopTokenRaw(Parser);
    if (AtT.Type == T1)
    {
      ++Result;
    }

    if (AtT.Type == T2)
    {
      break;
    }
  }

  Parser->Tokens->At = ResetToStart;

  return Result;
}

link_internal c_token_cursor *
DuplicateCTokenCursor(c_token_cursor *Tokens, memory_arena *Memory)
{
  Assert(Tokens->At == Tokens->Start);

  parser Parser_ = MakeParser(Tokens);
  parser *Parser = &Parser_;


  umm TokenCount = (umm)(Tokens->End - Tokens->Start);
  u32 LineNumber = 0;
  counted_string Filename = CSz("TODO(Jesse): add filename here?");
  c_token_cursor *Result = AllocateTokenCursor(Memory, Filename, TokenCount, TokenCursorSource_Unknown, LineNumber, {});

  c_token *At = Result->Start;
  u32 AtIndex = 0;
  while (c_token *T = PopTokenRawPointer(Parser))
  {
    // NOTE(Jesse): This routine only supports copying buffers with no down pointers
    Assert(HasValidDownPointer(T) == False);

    Assert(T->Type != CTokenType_CommentMultiLine || T->Type != CTokenType_CommentSingleLine || !T->Erased);
    At[AtIndex++] = *T;
  }

  Assert(AtIndex == TokenCount);

  Rewind(Tokens);

  return Result;
}

link_internal parser *
DuplicateCTokenCursor2(c_token_cursor *Tokens, memory_arena *Memory)
{
  Assert(Tokens->At == Tokens->Start);

  parser *Result = Allocate(parser, Memory, 1);
  Result->Tokens = DuplicateCTokenCursor(Tokens, Memory);
  return Result;
}

// NOTE(Jesse): This function is pretty sketch .. it modifies the parser you
// pass in which kinda goes against convention in this codebase.
link_internal parser *
DuplicateParserTokens(parser *Parser, memory_arena *Memory)
{
  Assert(Parser->Tokens->At == Parser->Tokens->Start);
  Parser->Tokens = DuplicateCTokenCursor(Parser->Tokens, Memory);
  return Parser;
}

link_internal b32
TokenCursorsMatch(c_token_cursor *C1, c_token_cursor *C2)
{
  umm C1Elements = TotalElements(C1);
  umm C2Elements = TotalElements(C2);

  b32 Result = C1Elements == C2Elements;

  if (Result)
  {
    for(u32 TokenIndex = 0;
        TokenIndex < C1Elements;
        ++TokenIndex)
    {
      if (C1->Start[TokenIndex] != C2->Start[TokenIndex])
      {
        Result = False;
        break;
      }
    }
  }

  return Result;
}

b32
TryTransmuteKeywordToken(c_token *T, c_token *LastTokenPushed)
{
  c_token_type StartType = T->Type;

  Assert(T->Type == CT_PreprocessorPaste_InvalidToken ||
         T->Type == CTokenType_Unknown);


  // TODO(Jesse): A large portion of this can be generated.  It should also
  // probably do a comptime string hash and switch off that instead of doing
  // so many serial comparisons.  It _also_ shouldn't even do serial
  // comparisons but I benched doing it both ways
  //
  // ie `if {} if {}` vs `if {} else if {} else if {}`
  //
  // and there was no measurable difference in runtime.  I decided to keep them
  // serial in case there's a subtle behavior difference between the two that
  // the tests don't pick up on.  I'd rather hit and debug that when this moves
  // to being generated if it doesn't make the program perceptibly faster.
  //

  if ( StringsMatch(T->Value, CSz("if")) )
  {
    T->Type = CTokenType_If;
  }
  else if ( StringsMatch(T->Value, CSz("else")) )
  {
    T->Type = CTokenType_Else;
  }
  else if ( StringsMatch(T->Value, CSz("break")) )
  {
    T->Type = CTokenType_Break;
  }
  else if ( StringsMatch(T->Value, CSz("switch")) )
  {
    T->Type = CTokenType_Switch;
  }
  else if ( StringsMatch(T->Value, CSz("case")) )
  {
    T->Type = CTokenType_Case;
  }
  else if ( StringsMatch(T->Value, CSz("delete")) )
  {
    T->Type = CT_Keyword_Delete;
  }
  else if ( StringsMatch(T->Value, CSz("default")) )
  {
    T->Type = CTokenType_Default;
  }
  else if ( StringsMatch(T->Value, CSz("for")) )
  {
    T->Type = CTokenType_For;
  }
  else if ( StringsMatch(T->Value, CSz("while")) )
  {
    T->Type = CTokenType_While;
  }
  else if ( StringsMatch(T->Value, CSz("continue")) )
  {
    T->Type = CTokenType_Continue;
  }
  else if ( StringsMatch(T->Value, CSz("return")) )
  {
    T->Type = CTokenType_Return;
  }
  else if ( StringsMatch(T->Value, CSz("thread_local")) )
  {
    T->Type = CTokenType_ThreadLocal;
  }
  else if ( StringsMatch(T->Value, CSz("const")) )
  {
    T->Type = CTokenType_Const;
  }
  else if ( StringsMatch(T->Value, CSz("static")) )
  {
    T->Type = CTokenType_Static;
  }
  else if ( StringsMatch(T->Value, CSz("__volatile__")) )
  {
    T->Type = CTokenType_Volatile;
  }
  else if ( StringsMatch(T->Value, CSz("volatile")) )
  {
    T->Type = CTokenType_Volatile;
  }
  else if ( StringsMatch(T->Value, CSz("void")) )
  {
    T->Type = CTokenType_Void;
  }
  else if ( StringsMatch(T->Value, CSz("long")) )
  {
    T->Type = CTokenType_Long;
  }
  else if ( StringsMatch(T->Value, CSz("float")) )
  {
    T->Type = CTokenType_Float;
  }
  else if ( StringsMatch(T->Value, CSz("char")) )
  {
    T->Type = CTokenType_Char;
  }
  else if ( StringsMatch(T->Value, CSz("double")) )
  {
    T->Type = CTokenType_Double;
  }
  else if ( StringsMatch(T->Value, CSz("short")) )
  {
    T->Type = CTokenType_Short;
  }
  else if ( StringsMatch(T->Value, CSz("int")) )
  {
    T->Type = CTokenType_Int;
  }
  else if ( StringsMatch(T->Value, CSz("bool")) )
  {
    T->Type = CTokenType_Bool;
  }
  else if ( StringsMatch(T->Value, CSz("auto")) )
  {
    T->Type = CTokenType_Auto;
  }
  else if ( StringsMatch(T->Value, CSz("signed")) )
  {
    T->Type = CTokenType_Signed;
  }
  else if ( StringsMatch(T->Value, CSz("unsigned")) )
  {
    T->Type = CTokenType_Unsigned;
  }
  else if ( StringsMatch(T->Value, CSz("public")) )
  {
    T->Type = CT_Keyword_Public;
  }
  else if ( StringsMatch(T->Value, CSz("private")) )
  {
    T->Type = CT_Keyword_Private;
  }
  else if ( StringsMatch(T->Value, CSz("protected")) )
  {
    T->Type = CT_Keyword_Protected;
  }
  else if ( StringsMatch(T->Value, CSz("__Noreturn__")) )
  {
    T->Type = CT_Keyword_Noreturn;
  }
  else if ( StringsMatch(T->Value, CSz("__noreturn__")) )
  {
    T->Type = CT_Keyword_Noreturn;
  }
  else if ( StringsMatch(T->Value, CSz("noreturn")) )
  {
    T->Type = CT_Keyword_Noreturn;
  }
  else if ( StringsMatch(T->Value, CSz("_Noreturn")) )
  {
    T->Type = CT_Keyword_Noreturn;
  }
  else if ( StringsMatch(T->Value, CSz("override")) )
  {
    T->Type = CT_Keyword_Override;
  }
  else if ( StringsMatch(T->Value, CSz("virtual")) )
  {
    T->Type = CT_Keyword_Virtual;
  }
  else if ( StringsMatch(T->Value, CSz("noexcept")) )
  {
    T->Type = CT_Keyword_Noexcept;
  }
  else if ( StringsMatch(T->Value, CSz("explicit")) )
  {
    T->Type = CT_Keyword_Explicit;
  }
  else if ( StringsMatch(T->Value, CSz("constexpr")) )
  {
    T->Type = CT_Keyword_Constexpr;
  }
  else if ( StringsMatch(T->Value, CSz("namespace")) )
  {
    T->Type = CT_Keyword_Namespace;
  }
  else if ( StringsMatch(T->Value, CSz("class")) )
  {
    T->Type = CT_Keyword_Class;
  }
  else if ( StringsMatch(T->Value, CSz("struct")) )
  {
    T->Type = CTokenType_Struct;
  }
  else if ( StringsMatch(T->Value, CSz("typedef")) )
  {
    T->Type = CTokenType_Typedef;
  }
  else if ( StringsMatch(T->Value, CSz("__asm__")) )
  {
    T->Type = CTokenType_Asm;
  }
  else if ( StringsMatch(T->Value, CSz("asm")) )
  {
    T->Type = CTokenType_Asm;
  }
  else if ( StringsMatch(T->Value, CSz("poof")) )
  {
    T->Type = CTokenType_Poof;
  }
  else if ( StringsMatch(T->Value, CSz("union")) )
  {
    T->Type = CTokenType_Union;
  }
  else if ( StringsMatch(T->Value, CSz("using")) )
  {
    T->Type = CTokenType_Using;
  }
  else if ( StringsMatch(T->Value, CSz("enum")) )
  {
    T->Type = CTokenType_Enum;
  }
  else if ( StringsMatch(T->Value, CSz("goto")) )
  {
    T->Type = CTokenType_Goto;
  }
  else if ( StringsMatch(T->Value, CSz("template")) )
  {
    T->Type = CTokenType_TemplateKeyword;
  }
  else if ( StringsMatch(T->Value, CSz("__inline__")) )
  {
    T->Type = CTokenType_Inline;
  }
  else if ( StringsMatch(T->Value, CSz("__inline")) )
  {
    T->Type = CTokenType_Inline;
  }
  else if ( StringsMatch(T->Value, CSz("inline")) )
  {
    T->Type = CTokenType_Inline;
  }
  else if ( StringsMatch(T->Value, CSz("operator")) )
  {
    T->Type = CTokenType_OperatorKeyword;
  }
  else if ( StringsMatch(T->Value, CSz("static_assert")) )
  {
    T->Type = CT_StaticAssert;
  }
  else if ( StringsMatch(T->Value, CSz("_Pragma")) )
  {
    T->Type = CT_KeywordPragma;
  }
  else if ( StringsMatch(T->Value, CSz("__pragma")) )
  {
    T->Type = CT_KeywordPragma;
  }
  else if ( StringsMatch(T->Value, CSz("extern")) )
  {
    T->Type = CTokenType_Extern;
  }
  else if ( StringsMatch(T->Value, CSz("__attribute__")) )
  {
    T->Type = CT_KeywordAttribute;
  }
  else if ( StringsMatch(T->Value, CSz("__has_include")) )
  {
    T->Type = CT_PreprocessorHasInclude;
  }
  else if ( StringsMatch(T->Value, CSz("__has_include_next")) )
  {
    T->Type = CT_PreprocessorHasIncludeNext;
  }
  else if ( StringsMatch(T->Value, CSz("__VA_ARGS__")) )
  {
    T->Type = CT_Preprocessor_VA_ARGS_;
  }
  else if (LastTokenPushed && LastTokenPushed->Type == CT_ScopeResolutionOperator)
  {
    T->QualifierName = LastTokenPushed->QualifierName;
  }

  b32 Result = T->Type != StartType;
  return Result;
}

// TODO(Jesse): Get rid of parse_context here.  It's literally a boolean
struct parse_context;

link_internal c_token_cursor *
TokenizeAnsiStream(ansi_stream Code, memory_arena* Memory, b32 IgnoreQuotes, parse_context *Ctx, token_cursor_source Source)
{
  TIMED_FUNCTION();

  u32 LineNumber = 1;

  umm ByteCount = TotalElements(&Code);
  umm TokensToAllocate = ByteCount;

  // Allocate a huge buffer that gets truncated to the necessary size at the end of the tokenization
  //
  // TODO(Jesse)(hardening): This could now (easily?) expand as we go, but at
  // the moment I don't see a reason for doing that work.  Maybe when I get
  // around to hardening.
  c_token_cursor *Tokens = AllocateTokenCursor(Memory, Code.Filename, TokensToAllocate, Source, LineNumber, {0, 0});

  if (!Tokens->Start)
  {
    Error("Allocating Token Buffer");
    return {};
  }

  if (Code.Start)
  {
    b32 ParsingSingleLineComment = False;
    b32 ParsingMultiLineComment = False;

    c_token *LastTokenPushed = 0;
    c_token *CommentToken = 0;

    TIMED_BLOCK("Lexer");
    while(Remaining(&Code))
    {
      c_token FirstT = PeekToken(&Code);
      c_token SecondT = {};
      if (Remaining(&Code, 1)) SecondT = PeekToken(&Code, 1);

      c_token PushT = {};

      PushT.Type = FirstT.Type;
      PushT.Value = CS(Code.At, 1);

      switch (FirstT.Type)
      {
        case CTokenType_FSlash:
        {
          switch (SecondT.Type)
          {
            case CTokenType_FSlash:
            {
              ParsingSingleLineComment = True;
              PushT.Type = CTokenType_CommentSingleLine;
              PushT.Value = CS(Code.At, 2);
              Advance(&Code);
              Advance(&Code);
            } break;

            case CTokenType_Star:
            {
              ParsingMultiLineComment = True;
              PushT.Type = CTokenType_CommentMultiLine;
              PushT.Value = CS(Code.At, 2);
              Advance(&Code);
              Advance(&Code);
            } break;

            case CTokenType_Equals:
            {
              PushT.Type = CTokenType_DivEquals;
              PushT.Value = CS(Code.At, 2);
              Advance(&Code);
              Advance(&Code);
            } break;

            default:
            {
              Advance(&Code);
            } break;
          }
        } break;

        case CTokenType_LT:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_LessEqual;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_LT)
          {
            PushT.Type = CTokenType_LeftShift;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        } break;

        case CTokenType_GT:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_GreaterEqual;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_GT)
          {
            PushT.Type = CTokenType_RightShift;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        } break;

        case CTokenType_Equals:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_AreEqual;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        } break;

        case CTokenType_Dot:
        {
          if (SecondT.Type == CTokenType_Dot &&
              PeekToken(&Code, 2).Type == CTokenType_Dot)
          {
            PushT.Type = CTokenType_Ellipsis;
            PushT.Value = CS(Code.At, 3);
            Advance(&Code);
            Advance(&Code);
            Advance(&Code);
          }
          else if (Remaining(&Code) > 1 && IsNumeric(*(Code.At+1)))
          {
            PushT = ParseNumericToken(&Code);
          }
          else
          {
            Advance(&Code);
          }

        } break;

        case CTokenType_Bang:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_NotEqual;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        } break;

        case CTokenType_Hat:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_XorEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        }break;

        case CTokenType_Pipe:
        {
          if (SecondT.Type == CTokenType_Pipe)
          {
            PushT.Type = CTokenType_LogicalOr;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_OrEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        }break;

        case CTokenType_Ampersand:
        {
          if (SecondT.Type == CTokenType_Ampersand)
          {
            PushT.Type = CTokenType_LogicalAnd;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_AndEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        }break;

        case CTokenType_Percent:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_ModEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        }break;

        case CTokenType_Minus:
        {
          if (SecondT.Type == CTokenType_Minus)
          {
            PushT.Type = CTokenType_Decrement;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_MinusEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_GT)
          {
            PushT.Type = CTokenType_Arrow;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
            Advance(&Code);
          }
          else if (Remaining(&Code) > 1 && IsNumeric(*(Code.At+1)))
          {
            PushT = ParseNumericToken(&Code);
          }
          else
          {
            Advance(&Code);
          }

        }break;

        case CTokenType_Plus:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_PlusEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_Plus)
          {
            PushT.Type = CTokenType_Increment;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          Advance(&Code);
        }break;

        case CTokenType_Star:
        {
          if (SecondT.Type == CTokenType_Equals)
          {
            PushT.Type = CTokenType_TimesEquals;
            PushT.Value = CS(Code.At, 2);
            Advance(&Code);
          }
          else if (SecondT.Type == CTokenType_FSlash)
          {
            ParsingMultiLineComment = False;
            Advance(&Code);
          }
          else
          {
            PushT.Type = CTokenType_Star;
            PushT.Value = CS(Code.At, 1);
          }
          Advance(&Code);
        } break;

        case CTokenType_SingleQuote:
        {
          if (IgnoreQuotes || ParsingSingleLineComment || ParsingMultiLineComment)
          {
            Advance(&Code);
          }
          else
          {
            PushT.Type = CTokenType_CharLiteral;
            PushT.Value = PopQuotedCharLiteral(&Code, True);

            if(PushT.Value.Count > 2)
            {
              Assert(PushT.Value.Start[0] == '\'');
              Assert(PushT.Value.Start[PushT.Value.Count-1] == '\'');

              for ( u32 CharIndex = 1;
                    CharIndex < PushT.Value.Count-1;
                    ++CharIndex )
              {
                PushT.UnsignedValue += (umm)PushT.Value.Start[CharIndex];
              }
            }
            else
            {
              Error("Quoted char literal with length %u .. ???", (u32)PushT.Value.Count);
            }
          }
        } break;

        case CTokenType_DoubleQuote:
        {
          if (IgnoreQuotes || ParsingSingleLineComment || ParsingMultiLineComment)
          {
            Advance(&Code);
          }
          else
          {
            PushT.Type = CTokenType_StringLiteral;
            PushT.Value = PopQuotedString(&Code, True);
          }
        } break;

        case CTokenType_BSlash:
        {
          if (SecondT.Type == CTokenType_CarrigeReturn)
          {
            ++PushT.Value.Count;
            Advance(&Code);
            Assert(PeekToken(&Code, 1).Type == CTokenType_Newline);
          }

          if (PeekToken(&Code, 1).Type == CTokenType_Newline)
          {
            PushT.Type = CTokenType_EscapedNewline;
            ++PushT.Value.Count;
            Advance(&Code);
          }
          Advance(&Code);
        } break;

        case CTokenType_CarrigeReturn:
        {
          Advance(&Code);
          if (PeekToken(&Code).Type == CTokenType_Newline)
          {
            PushT.Type = CTokenType_Newline;
            PushT.Value.Count = 2;
            ParsingSingleLineComment = False;
            Advance(&Code);
          }
        } break;

        case CTokenType_Newline:
        {
          ParsingSingleLineComment = False;
          Advance(&Code);
        } break;

        case CTokenType_Colon:
        {
          if (SecondT.Type == CTokenType_Colon)
          {
            Advance(&Code);
            Advance(&Code);
            PushT.Type = CT_ScopeResolutionOperator;
            PushT.Value.Count = 2;

            if (LastTokenPushed && LastTokenPushed->Type == CTokenType_Identifier)
            {
              LastTokenPushed->Type = CT_NameQualifier;
              PushT.QualifierName = LastTokenPushed;
            }
          }
          else
          {
            Advance(&Code);
          }

        } break;

        case CTokenType_Hash:
        {
          if (SecondT.Type == CTokenType_Hash)
          {
            Advance(&Code);
            Advance(&Code);
            PushT.Type = CT_PreprocessorPaste;
            PushT.Value.Count = 2;
          }
          else
          {
            if (Ctx)
            {
              const char* HashCharacter = Code.At;
              Advance(&Code);
              const char* FirstAfterHash = Code.At;

              LineNumber += EatSpacesTabsAndEscapedNewlines(&Code);

              counted_string TempValue = PopIdentifier(&Code);

              if ( StringsMatch(TempValue, CSz("define")) )
              {
                PushT.Type = CT_PreprocessorDefine;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("undef")) )
              {
                PushT.Type = CT_PreprocessorUndef;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("if")) )
              {
                PushT.Type = CT_PreprocessorIf;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("else")) )
              {
                PushT.Type = CT_PreprocessorElse;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("elif")) )
              {
                PushT.Type = CT_PreprocessorElif;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("endif")) )
              {
                PushT.Type = CT_PreprocessorEndif;;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("ifndef")) )
              {
                PushT.Type = CT_PreprocessorIfNotDefined;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("ifdef")) )
              {
                PushT.Type = CT_PreprocessorIfDefined;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("include")) )
              {
                PushT.Type = CT_PreprocessorInclude;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("include_next")) )
              {
                PushT.Type = CT_PreprocessorIncludeNext;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("error")) )
              {
                PushT.Type = CT_PreprocessorError;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("warning")) )
              {
                PushT.Type = CT_PreprocessorWarning;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else if ( StringsMatch(TempValue, CSz("pragma")) )
              {
                PushT.Type = CT_PreprocessorPragma;
                PushT.Value.Count = (umm)(TempValue.Start + TempValue.Count - HashCharacter);
              }
              else
              {
                // The token is just a regular hash .. roll back our parsing to the start.
                Code.At = FirstAfterHash;
              }
            }
            else
            {
              Advance(&Code);
            }
          }

        } break;

        case CTokenType_Unknown:
        {
          if (IsNumeric(*Code.At))
          {
            PushT = ParseNumericToken(&Code);
          }
          else
          {
            PushT.Value = PopIdentifier(&Code);

            // This looks kinda dumb but this code has to look like this to
            // pacify the asserts in TryTransmuteKeywordToken().  Not sure if
            // that's actually a good reason for making this code very verbose,
            // but I'm leaving it like this for now.
            //
            // For reference the less-verbose version I had in mind is:
            //
            // PushT.Type = CTokenType_Identifier;
            // TryTransmuteKeywordToken(&PushT, LastTokenPushed);
            //
            // But this triggers the assert in TryTransmuteKeywordToken, which I
            // don't really want to get rid of.

            if (Ctx)
            {
              if (TryTransmuteKeywordToken && TryTransmuteKeywordToken(&PushT, LastTokenPushed))
              {
              }
              else
              {
                PushT.Type = CTokenType_Identifier;
              }
            }
            else
            {
              PushT.Type = CTokenType_Identifier;
            }
          }

        } break;

        default: { Advance(&Code); } break;
      }

      Assert(PushT.Type);

      PushT.Filename = Code.Filename;
      PushT.LineNumber = LineNumber;

      if (ParsingSingleLineComment || ParsingMultiLineComment)
      {

        // Set CommentToken on the first loop through when we start parsing a comment
        if (!CommentToken)
        {
          PushT.Erased = True;
          CommentToken = Push(PushT, Tokens);
          LastTokenPushed = CommentToken;
        }

        if ( (PushT.Type == CTokenType_EscapedNewline) ||
             (ParsingMultiLineComment && PushT.Type == CTokenType_Newline) )
        {
          LastTokenPushed = Push(PushT, Tokens);

          Assert(CommentToken->Erased);
          CommentToken->Value.Count = (umm)((umm)Code.At - (umm)CommentToken->Value.Start - (umm)PushT.Value.Count);
          CommentToken->LineNumber = LineNumber;
          CommentToken = Push(*CommentToken, Tokens);
          Assert(CommentToken->Erased);

          CommentToken->LineNumber = LineNumber+1;
          CommentToken->Value.Start = Code.At;
        }
      }
      else if ( CommentToken &&
                !(ParsingSingleLineComment || ParsingMultiLineComment) ) // Finished parsing a comment
      {
        umm Count = (umm)(Code.At - CommentToken->Value.Start);

        // We finished parsing a comment on this token
        if (PushT.Type == CTokenType_Newline)
        {
          if (Count >= PushT.Value.Count) { Count -= PushT.Value.Count; } // Exclude the \n from single line comments (could also be \r\n)
          LastTokenPushed = Push(PushT, Tokens);
        }

        CommentToken->Value.Count = Count;
        Assert(CommentToken->Erased);
        CommentToken = 0;
      }
      else if ( PushT.Type == CT_PreprocessorInclude ||
                PushT.Type == CT_PreprocessorIncludeNext )
      {
        LineNumber += EatSpacesTabsAndEscapedNewlines(&Code);
        if (*Code.At == '"')
        {
          PushT.Flags |= CTFlags_RelativeInclude;
          PushT.IncludePath = PopQuotedString(&Code, True);
        }
        else if (*Code.At == '<')
        {
          PushT.IncludePath = EatBetweenExcluding(&Code, '<', '>');
        }
        else
        {
          // NOTE(Jesse): This path throws an error during ResolveInclude()
        }

        // TODO(Jesse, correctness): This seems very sus..  Why would we need to eat whitespace?
        LineNumber += EatSpacesTabsAndEscapedNewlines(&Code);
        PushT.Value.Count = (umm)(Code.At - PushT.Value.Start);

        LastTokenPushed = Push(PushT, Tokens);

        c_token InsertedCodePlaceholder = {};
        InsertedCodePlaceholder.Type = CT_InsertedCode;
        InsertedCodePlaceholder.Filename = Code.Filename;
        InsertedCodePlaceholder.LineNumber = LineNumber;

        LastTokenPushed = Push(InsertedCodePlaceholder, Tokens);
      }
      else if (IsNBSP(&PushT) && LastTokenPushed && PushT.Type == LastTokenPushed->Type)
      {
        Assert( (LastTokenPushed->Value.Start+LastTokenPushed->Value.Count) == PushT.Value.Start);
        LastTokenPushed->Value.Count += 1;
      }
      else
      {
        LastTokenPushed = Push(PushT, Tokens);
      }

      // NOTE(Jesse): This has to come last.
      if ( IsNewline(PushT.Type) ) { ++LineNumber; }

      continue;
    }

    END_BLOCK();
  }
  else
  {
    Warn("Input to TokenizeAnsiStreamInput was of length 0");
  }

  ResizeCursor(Tokens, AtElements(Tokens), Memory);
  Rewind(Tokens);

  return Tokens;
}

link_internal c_token_cursor *
CTokenCursorForAnsiStream(parse_context *Ctx, ansi_stream SourceFileStream, token_cursor_source Source, memory_arena *Memory)
{
  c_token_cursor *Result = TokenizeAnsiStream(SourceFileStream, Memory, False, Ctx, Source);
  return Result;
}

link_internal parser *
ParserForAnsiStream(parse_context *Ctx, ansi_stream SourceFileStream, token_cursor_source Source, memory_arena *Memory)
{
  TIMED_FUNCTION();

  parser *Result = Allocate(parser, Memory, 1);

  if (Result)
  {
    if (SourceFileStream.Start)
    {
      Result->Tokens = CTokenCursorForAnsiStream(Ctx, SourceFileStream, Source, Memory);
    }
    else
    {
      Result->ErrorCode = ParseErrorCode_InputStreamNull;
    }
  }
  else
  {
    Error("Allocating parser during ParserForAnsiStream");
  }

  return Result;
}

link_internal parser *
ParserForFile(parse_context *Ctx, counted_string Filename, token_cursor_source Source, memory_arena *Memory)
{
  ansi_stream SourceFileStream = AnsiStreamFromFile(Filename, Memory);
  parser *Result = ParserForAnsiStream(Ctx, SourceFileStream, Source, Memory);
  return Result;
}

