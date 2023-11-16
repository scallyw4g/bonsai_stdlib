
poof(buffer(c_token))
#include <generated/buffer_c_token.h>

poof(buffer(c_token_buffer))
#include <generated/buffer_c_token_buffer.h>

poof(generate_stream(c_token_buffer))
#include <generated/generate_stream_c_token_buffer.h>


link_internal b32
IsValidForCursor(c_token_cursor *Tokens, c_token *T)
{
  b32 Result = T < Tokens->End && T >= Tokens->Start;
  return Result;
}

/*****************************                ********************************/
/*****************************  Error Output  ********************************/
/*****************************                ********************************/


link_internal void
PrintTraySimple(c_token *T, b32 Force = False, u32 Depth = 0)
{
  if (T)
  {
    Assert(T->LineNumber < 100000);
    if (Force || T->Type == CTokenType_Newline || T->Type == CTokenType_EscapedNewline)
    {
      DebugChars("%*s", Depth*4, "");
      DebugChars("%*u |", 6, T->LineNumber);
    }
  }
  else
  {
    DebugChars("%*s", Depth*4, "");
    DebugChars("%*c |", 6, ' ');
  }
}

inline void
WriteTo(char_cursor *Dest, counted_string S)
{
  if (Dest) { CopyToDest(Dest, S); } else { DebugChars("%S", S); }
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


link_internal void
DumpSingle(c_token_cursor *Cursor, c_token *At)
{
  u32 TokenCount = (u32)TotalElements(Cursor);

  for ( u32 TokenIndex = 0;
        TokenIndex  < TokenCount;
        ++TokenIndex)
  {
    c_token *T = Cursor->Start+TokenIndex;
    if (T == At)
    {
      DebugChars("%S>%S", TerminalColors.Green, TerminalColors.White);
    }

    // NOTE(Jesse): Handle cursors that haven't been completely filled
    if (T->Type)
    {
      PrintToken(T);
    }
    else
    {
      break;
    }
  }

  return;
}

#if 0
link_internal void
DumpChain(parser* Parser, u32 LinesToDump = u32_MAX)
{
  c_token_cursor *Cursor = Parser->Tokens;
  c_token *At = Cursor->At;

  while(Cursor->Up) Cursor = Cursor->Up;

  DebugLine("-------------------- Dumping Cursor -- At Line (%d) ----------------------------", At->LineNumber);

  u32 Index = 0;
  /* while (Cursor) */
  {
    DumpSingle(Cursor, At);
    DebugLine("\n%S^%S Link(%d) %S^%S",
        TerminalColors.Blue, TerminalColors.White,
        Index,
        TerminalColors.Blue, TerminalColors.White);
    /* Cursor = Cursor->Next; */
    ++Index;
  }
}
#endif

link_internal void
PrintTokenVerbose(c_token_cursor *Tokens, c_token *T, c_token *AbsAt, u32 Depth)
{
  counted_string AtMarker = CSz(" ");
  if (AbsAt == T )
  {
    AtMarker = CSz(GREEN_TERMINAL ">" WHITE_TERMINAL);
  }
  else if (Tokens->At == T)
  {
    AtMarker = CSz(">");
  }

  counted_string ErasedString = T->Erased ? CSz("e") : CSz(" ");
  DebugChars("%*s(%S) %d %S %S %S ", Depth*2, "",  Basename(T->Filename), T->LineNumber, ErasedString, AtMarker, ToString(T->Type));
  PrintToken(*T);

  if (T->Type != CTokenType_Newline && T->Type != CTokenType_EscapedNewline)
  {
    DebugChars("\n");
  }
}

void
DumpCursorSlice(c_token_cursor* Tokens, umm PrevTokens, umm NextTokens)
{
  DebugLine("\n >> Dumping Cursor %S", Tokens->Filename);

  PrintTraySimple(Tokens->Start);

  c_token *StartT = Tokens->At - Min(PrevTokens, AtElements(Tokens));
  Assert(IsValidForCursor(Tokens, StartT));

  c_token *EndT = Tokens->At + Min(NextTokens, Remaining(Tokens));
  Assert(IsValidForCursor(Tokens, EndT) || EndT == Tokens->End);


  c_token *At = StartT;
  while (At < EndT)
  {
    PrintToken(At);
    PrintTraySimple(At);
    /* PrintTokenVerbose(Tokens, T, AbsoluteAt, Depth); */

    ++At;
  }

  DebugLine("\n");
}

void
DumpCursorSimple(c_token_cursor* Tokens, c_token *AbsoluteAt = 0, u32 Depth = 0)
{
  umm TotalTokens = TotalElements(Tokens);
  if (TotalTokens)
  {
    b32 Force = True;
    DebugChars("\n");

    PrintTraySimple(0, Force, Depth);
    DebugLine(">> Dumping Cursor >> %S", Tokens->Filename);

    PrintTraySimple(Tokens->Start, Force, Depth);

    c_token *LastToken = 0;
    for (umm TIndex = 0; TIndex < TotalTokens; ++TIndex)
    {
      c_token *T = Tokens->Start + TIndex;

#if 1
      PrintToken(T);
      if (TIndex+1 < TotalTokens) { PrintTraySimple(T, False, Depth); }
      else { LastToken = T; }
#else
      PrintTokenVerbose(Tokens, T, AbsoluteAt, Depth);
#endif

      switch (T->Type)
      {
        case CT_MacroLiteral:
        case CT_InsertedCode:
        {
          if (T->Down)
          {
            Assert(T->Down->Up.Tokens == Tokens);
            DumpCursorSimple(T->Down, AbsoluteAt, Depth+1);
            PrintTraySimple(T, Force, Depth);
          }
        } break;

        default: {} break;;
      }
    }

    if (AbsoluteAt == Tokens->End)
    {
      c_token UnknownMarkerToken = {};
      UnknownMarkerToken.Filename = Tokens->Start->Filename;
      PrintTokenVerbose(Tokens, &UnknownMarkerToken, &UnknownMarkerToken, Depth);
    }

    if (LastToken == 0 || LastToken->Type != CTokenType_Newline)
    {
      DebugChars("\n");
    }

    PrintTraySimple(0, Force, Depth);
    DebugLine(">> done");
  }
}

#if 0
link_internal void
DumpEntireParser(parser* Parser, u32 LinesToDump = u32_MAX, b32 Verbose = False)
{
  DebugLine("%S---%S", TerminalColors.Purple, TerminalColors.White);

  c_token *WasAt = PeekTokenRawPointer(Parser);

  u32 StartingLineNumber = WasAt->LineNumber;
  parse_error_code PrevError = Parser->ErrorCode;
  Parser->ErrorCode = ParseErrorCode_None;

  FullRewind(Parser);

  char TrayBuffer[32];
  counted_string TempString = CSz(TrayBuffer);
  char_cursor TrayCursor = CharCursor(TempString);


  /* b32 DoTray = true;//!Verbose; */
  b32 DoTray = !Verbose;
  if (DoTray)
  {
    PrintTray(&TrayCursor, PeekTokenRawPointer(Parser), 5);
    DebugChars(CS(&TrayCursor));
    TrayCursor.At = TrayCursor.Start;
  }

  while(c_token *T = PeekTokenRawPointer(Parser))
  {
    if (T == WasAt)
    {
      DebugChars("%S>%S", TerminalColors.Green, TerminalColors.White);
    }

    if (Verbose)
    {
      counted_string ErasedString = T->Erased ? CSz("(e)") : CSz("   ");
      DebugChars("(%d) %S %S", T->LineNumber, ErasedString, ToString(T->Type));
    }

    if ( Verbose && !IsNewline(T) )
    {
      DebugChars("(");
    }

    PrintToken(T);

    if ( Verbose && !IsNewline(T) )
    {
      DebugChars(")");
    }

    if (IsNewline(T))
    {
      if (LinesToDump-- == 0) break;

      if (DoTray)
      {
        PrintTray(&TrayCursor, PeekTokenRawPointer(Parser), 5);
        DebugChars(CS(&TrayCursor));
        TrayCursor.At = TrayCursor.Start;
      }
    }
    else
    {
      if (Verbose)
      {
        DebugChars("\n");
      }
    }

    // TODO(Jesse): require token should accept pointers
    RequireTokenRaw(Parser, *T);
  }

  if (WasAt)
  {
    RewindTo(Parser, WasAt);
    if (PeekTokenRawPointer(Parser) != WasAt)
    {
      AdvanceTo(Parser, WasAt);
    }
    Assert(PeekTokenRawPointer(Parser) == WasAt);
  }

  DebugLine("\n%S---%S", TerminalColors.Purple, TerminalColors.White);
  Parser->ErrorCode = PrevError;

  Assert(PeekTokenRawPointer(Parser)->LineNumber == StartingLineNumber);
}
#endif

#if 0
link_internal void
TruncateAtNextLineEnd(c_token_cursor *Tokens, u32 Count)
{
  while (Remaining(Tokens))
  {
    if(PopTokenRaw(Tokens).Type == CTokenType_Newline)
    {
      if (Count == 0)
      {
        break;
      }
      --Count;
    }
  }

  Tokens->End = Tokens->At;
  Tokens->Next = 0;
}

link_internal void
TruncateAtPreviousLineStart(parser* Parser, u32 Count )
{
  while (Parser->Tokens->At > Parser->Tokens->Start)
  {
    if (Parser->Tokens->At->Type == CTokenType_Newline)
    {
      if (Count == 0)
      {
        ++Parser->Tokens->At;
        break;
      }
      --Count;
    }
    --Parser->Tokens->At;
  }

  Parser->Tokens->Start = Parser->Tokens->At;
  Parser->Prev = 0;
}
#endif
