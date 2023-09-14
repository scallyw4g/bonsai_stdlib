
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
link_internal void PrintTraySimple(c_token *T, b32 Force = False, u32 Depth = 0);

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

inline void
WriteTo(char_cursor *Dest, counted_string S)
{
  if (Dest) { CopyToDest(Dest, S); } else { DebugChars(CSz("%S"), S); }
}

inline void
PrintToken(c_token *Token, char_cursor *Dest = 0)
{
  if (Token)
  {
    // NOTE(Jesse): Annoyingly, printing out the line tray for multi-line comments
    // requires that we allow printing tokens with Value.Count == 0
    /* Assert(Token->Value.Start && Token->Value.Count); */

    counted_string Color = {};

    switch (Token->Type)
    {
      case CT_PreprocessorInclude:
      case CT_PreprocessorIncludeNext:
      case CT_PreprocessorIf:
      case CT_PreprocessorElse:
      case CT_PreprocessorElif:
      case CT_PreprocessorEndif:
      case CT_PreprocessorIfDefined:
      case CT_PreprocessorIfNotDefined:
      case CT_PreprocessorDefine:
      case CT_PreprocessorUndef:
      case CT_PreprocessorPragma:
      case CT_PreprocessorError:
      case CT_PreprocessorWarning:
      case CT_Preprocessor_VA_ARGS_:
      {
        if (Token->Erased)
        {
          Color = TerminalColors.BrightYellow;
        }
        else
        {
          Color = TerminalColors.Yellow;
        }
      } break;

      case CT_MacroLiteral:
      {
        if (Token->Erased)
        {
          Color = TerminalColors.Purple;
        }
        else
        {
          Color = TerminalColors.Blue;
        }
      } break;

      default:
      {
        if (Token->Erased)
        {
          Color = TerminalColors.Grey;
        }
      } break;
    }


    if (Color.Count) { WriteTo(Dest, Color); }

    /* if (Token->Type == CTokenType_Newline) { WriteTo(Dest, CSz("\\n")); } */

    WriteTo(Dest, Token->Value);

    if (Color.Count) { WriteTo(Dest, TerminalColors.White); }
  }
}

inline void
PrintToken(c_token Token, char_cursor *Dest = 0)
{
  PrintToken(&Token, Dest);
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

link_internal b32
IsValidForCursor(c_token_cursor *Tokens, c_token *T)
{
  b32 Result = T < Tokens->End && T >= Tokens->Start;
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

