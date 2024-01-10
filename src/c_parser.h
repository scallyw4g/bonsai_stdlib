
#define InvalidDefaultWhileParsing(P, ErrorMessage) \
    default: { ParseError(P, ErrorMessage, PeekTokenPointer(P)); } break;

#define InvalidDefaultError(P, ErrorMessage, Token) \
    default: { ParseError(P, ErrorMessage, Token); } break;



enum parse_warn_code
{
  ParseWarnCode_None,

  ParseWarnCode_MacroRedefined,

  ParseWarnCode_Generic,
};

enum parse_error_code
{
  ParseErrorCode_None,

  // C parsing errors
  ParseErrorCode_ExpectedSemicolonOrEquals,
  ParseErrorCode_StreamEndedUnexpectedly,
  ParseErrorCode_RequireTokenFailed,
  ParseErrorCode_InvalidTokenGenerated,
  ParseErrorCode_MalformedType,


  // Poof errors
  ParseErrorCode_PoofUserlandError,
  ParseErrorCode_PoofTypeError,
  ParseErrorCode_DUnionParse,
  ParseErrorCode_UndefinedDatatype,
  ParseErrorCode_InvalidDirective,
  ParseErrorCode_InvalidOperator,
  ParseErrorCode_InvalidMetaTransformOp,
  ParseErrorCode_InvalidArgument,
  ParseErrorCode_InvalidArgumentType,
  ParseErrorCode_InvalidArgumentCount,
  ParseErrorCode_InvalidName,
  ParseErrorCode_InvalidTag,
  ParseErrorCode_InvalidFunction,
  ParseErrorCode_SyntaxError,
  ParseErrorCode_NotImplemented, // NOTE(Jesse): This means the compiler should support this case, but doesn't


  // General errors
  ParseErrorCode_InputStreamNull,


  // Well, shit.
  ParseErrorCode_InternalCompilerError,
  ParseErrorCode_StackOverflow, // Poof stack overflow.  Usually from calling a function recursively


  // We hit an error, but didn't classify it.
  ParseErrorCode_Unknown,
};
poof(generate_string_table(parse_error_code))
#include <generated/generate_string_table_parse_error_code.h>


struct parser
{
  parse_warn_code WarnCode;
  parse_error_code ErrorCode;
  c_token_cursor *Tokens;
};

link_internal b32
AreEqual(parser P1, parser P2)
{
  NotImplemented;
  return False;
}

link_internal void
DeepCopy( parser *Src, parser *Dest)
{
  NotImplemented;
}

poof(generate_cursor(parser))
#include <generated/generate_cursor_parser.h>

poof(generate_stream(parser))
#include <generated/generate_stream_parser.h>

link_internal u64
Hash(parser *Parser)
{
  u64 Result = Hash(&Parser->Tokens->Filename);
  return Result;
}

link_internal parser
MakeParser(c_token_cursor *Tokens)
{
  parser Result = {
    .Tokens = Tokens,
  };
  return Result;
}




struct peek_result
{
  c_token_cursor *Tokens;

  c_token *At;
  b32 DoNotDescend;
};

struct string_from_parser
{
  parser *Parser;
  c_token *StartToken;
};




inline void
Invalidate(peek_result *Peek)
{
  Peek->At = 0;
}

inline void
Invalidate(c_token_cursor *Tokens)
{
  Tokens->At = Tokens->End;
}

inline b32
IsValid(peek_result *Peek)
{
  b32 Result = Peek->At != 0;
  return Result;
}


b32 AreEqual(c_token T1, c_token T2)
{
  b32 Result = (T1.Type == T2.Type);

  if (Result && (T1.Type == CTokenType_Newline ||
                 T1.Type == CTokenType_EscapedNewline ) )
  {
    // NOTE(Jesse): On Windows newline chars can be length 2 (\r\n) so we don't
    // check that the Value strings match for that case
    Assert(T2.Type == CTokenType_Newline || T2.Type == CTokenType_EscapedNewline );
  }
  else
  {
    if (Result && (T1.Value.Count > 1 || T2.Value.Count > 1) )
    {
      Result &= StringsMatch(T1.Value, T2.Value);
    }
  }
  return Result;
}

b32 AreEqual(c_token *T1, c_token T2)
{
  b32 Result = False;
  if (T1)
  {
    Result = AreEqual(*T1, T2);
  }
  return Result;
}

b32 AreEqual(c_token T1, c_token *T2)
{
  b32 Result = False;
  if (T2)
  {
    Result = AreEqual(T1, *T2);
  }
  return Result;
}

b32 AreEqual(c_token *T1, c_token *T2)
{
  b32 Result = False;
  if (T1 && T2)
  {
    Result = AreEqual(*T1, *T2);
  }
  return Result;
}

b32
operator==(c_token T1, c_token T2)
{
  b32 Result = AreEqual(T1, T2);
  return Result;
}

b32
operator!=(c_token T1, c_token T2)
{
  b32 Result = !(T1==T2);
  return Result;
}

inline c_token
CToken(counted_string Value)
{
  c_token Result = {
    .Type = CTokenType_Identifier,
    .Value = Value
  };
  return Result;
}

inline c_token
CToken(c_token_type Type, counted_string Value = CSz(""))
{
  c_token Result = {};

  Result.Type = Type;
  Result.Value = Value;

  return Result;
}

inline void
CTokenCursor(c_token_cursor *Result, c_token *Buffer, umm Count, counted_string Filename, token_cursor_source Source, c_token_cursor_up Up)
{
  Result->Filename = Filename;
  Result->Source = Source;
  Result->Start = Buffer;
  Result->At = Result->Start;
  Result->End = Result->Start + Count;
  Result->Up = Up;
}

// TODO(Jesse)(safety, memory): Remove this; these are not allowed to be
// stack-allocated so we might as well not have a function that returns one..
link_internal c_token_cursor
CTokenCursor(c_token *Start, c_token *End, counted_string Filename, token_cursor_source Source, c_token_cursor_up Up)
{
  c_token_cursor Result = {
    .Source = Source,
    .Start = Start,
    .At = Start,
    .End = End,
    .OwnedByThread = INVALID_THREAD_LOCAL_THREAD_INDEX,
    .Up = Up,
    .Filename = Filename,
  };
  return Result;
}

