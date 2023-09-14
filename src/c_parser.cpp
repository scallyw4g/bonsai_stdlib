
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

