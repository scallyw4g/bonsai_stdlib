
#define InvalidDefaultWhileParsing(P, ErrorMessage) \
    default: { ParseError(P, ErrorMessage, PeekTokenPointer(P)); } break;

#define InvalidDefaultError(P, ErrorMessage, Token) \
    default: { ParseError(P, ErrorMessage, Token); } break;


struct c_token_cursor;
struct macro_def;
struct macro_expansion
{
  c_token_cursor *Expansion;
  macro_def *Def;
};


enum c_token_type
{
  CTokenType_Unknown = 0,

  CT_ControlChar_Start_of_Heading          = 1,
  CT_ControlChar_Start_of_Text             = 2,
  CT_ControlChar_End_of_Text               = 3,
  CT_ControlChar_End_of_Transmission       = 4,
  CT_ControlChar_Enquiry                   = 5,
  CT_ControlChar_Acknowledgement           = 6,
  CT_ControlChar_Bell                      = 7,
  CT_ControlChar_Backspace                 = 8,
  /* CT_ControlChar_Horizontal_Tab            = 9, */
  /* CT_ControlChar_Line_Feed                 = 10, */
  CT_ControlChar_Vertical_Tab              = 11,
  CT_ControlChar_Form_Feed                 = 12,
  /* CT_ControlChar_Carriage_Return           = 13, */
  CT_ControlChar_Shift_Out                 = 14,
  CT_ControlChar_Shift_In                  = 15,
  CT_ControlChar_Data_Link_Escape          = 16,
  CT_ControlChar_Device_Control_1          = 17,
  CT_ControlChar_Device_Control_2          = 18,
  CT_ControlChar_Device_Control_3          = 19,
  CT_ControlChar_Device_Control_4          = 20,
  CT_ControlChar_Negative_Acknowledgement  = 21,
  CT_ControlChar_Synchronous_Idle          = 22,
  CT_ControlChar_End_of_Transmission_Block = 23,
  CT_ControlChar_Cancel                    = 24,
  CT_ControlChar_End_of_Medium             = 25,
  CT_ControlChar_Substitute                = 26,
  CT_ControlChar_Escape                    = 27,
  CT_ControlChar_File_Separator            = 28,
  CT_ControlChar_Group_Separator           = 29,
  CT_ControlChar_Record_Separator          = 30,
  CT_ControlChar_Unit_Separator            = 31,
  CT_ControlChar_Delete                    = 127,

  CTokenType_OpenBracket   = '[',
  CTokenType_CloseBracket  = ']',
  CTokenType_OpenBrace     = '{',
  CTokenType_CloseBrace    = '}',
  CTokenType_OpenParen     = '(',
  CTokenType_CloseParen    = ')',
  CTokenType_Dot           = '.',
  CTokenType_Comma         = ',',
  CTokenType_Semicolon     = ';',
  CTokenType_Colon         = ':',
  CTokenType_Hash          = '#',
  CTokenType_At            = '@',
  CTokenType_Dollar        = '$',
  CTokenType_Space         = ' ',
  CTokenType_Tab           = '\t',
  CTokenType_Star          = '*',
  CTokenType_Ampersand     = '&', // TODO(Jesse id: 238, tags: immediate, cleanup): Change name to BitwiseAnd
  CTokenType_SingleQuote   = '\'',
  CTokenType_DoubleQuote   = '"',
  CTokenType_Equals        = '=',
  CTokenType_LT            = '<',
  CTokenType_GT            = '>',
  CTokenType_Plus          = '+',
  CTokenType_Minus         = '-',
  CTokenType_Percent       = '%',
  CTokenType_Bang          = '!',
  CTokenType_Hat           = '^',
  CTokenType_Question      = '?',
  CTokenType_FSlash        = '/',
  CTokenType_BSlash        = '\\',
  CTokenType_Tilde         = '~',
  CTokenType_Backtick      = '`',
  CTokenType_Pipe          = '|', // TODO(Jesse id: 239, tags: immediate, cleanup): Change name to BitwiseOr
  CTokenType_Newline       = '\n',
  CTokenType_CarrigeReturn = '\r',
  CTokenType_EOF           = EOF,


  CTokenType_CommentSingleLine     = 256, // Making the assumption we'll never parse anything that's not ascii, so start the non-ascii tokens at 256
  CTokenType_CommentMultiLine,

  CTokenType_EscapedNewline,

  CTokenType_Identifier,

  CTokenType_StringLiteral,
  CTokenType_CharLiteral,
  CTokenType_IntLiteral,
  CTokenType_DoubleLiteral,
  CTokenType_FloatLiteral,

  CTokenType_Poof,

  // C11 keyword
  CT_Keyword_Noreturn,

  CT_Keyword_Override,
  CT_Keyword_Virtual,
  CT_Keyword_Noexcept,
  CT_Keyword_Explicit,
  CT_Keyword_Constexpr,
  CT_Keyword_Namespace,
  CT_Keyword_Class,
  CTokenType_Struct,
  CTokenType_Enum,
  CTokenType_Union,
  CTokenType_Typedef,

  CTokenType_Using,

  CTokenType_ThreadLocal,
  CTokenType_Const,
  CTokenType_Static,
  CTokenType_Volatile,
  CTokenType_Long,
  CTokenType_Unsigned,
  CTokenType_Signed,

  CTokenType_Bool,
  CTokenType_Auto,
  CTokenType_Void,
  CTokenType_Double,
  CTokenType_Float,
  CTokenType_Char,
  CTokenType_Int,
  CTokenType_Short,

  CTokenType_Inline,
  CTokenType_TemplateKeyword,
  CTokenType_OperatorKeyword,

  CT_Keyword_Public,
  CT_Keyword_Private,
  CT_Keyword_Protected,

  CT_KeywordPragma,
  CT_KeywordAttribute,

  CT_StaticAssert,

  CTokenType_Extern,
  CTokenType_Asm,
  CTokenType_Goto,

  CTokenType_Ellipsis,

  CTokenType_If,
  CTokenType_Else,
  CTokenType_Break,
  CTokenType_Switch,
  CTokenType_Case,
  CT_Keyword_Delete,
  CTokenType_Default,
  CTokenType_For,
  CTokenType_While,
  CTokenType_Continue,
  CTokenType_Return,

  CTokenType_LeftShift,
  CTokenType_RightShift,

  CTokenType_LessEqual,
  CTokenType_GreaterEqual,
  CTokenType_AreEqual,
  CTokenType_NotEqual,

  CTokenType_PlusEquals,
  CTokenType_MinusEquals,
  CTokenType_TimesEquals,
  CTokenType_DivEquals,
  CTokenType_ModEquals,
  CTokenType_AndEquals,
  CTokenType_OrEquals,
  CTokenType_XorEquals,

  CTokenType_Increment,
  CTokenType_Decrement,
  CTokenType_LogicalAnd,
  CTokenType_LogicalOr,

  CTokenType_Arrow,

  CT_ScopeResolutionOperator,
  CT_NameQualifier,

  CT_MacroLiteral,
  CT_MacroLiteral_SelfRefExpansion,

  CT_PreprocessorHasInclude,
  CT_PreprocessorHasIncludeNext,
  CT_PreprocessorPaste,
  CT_PreprocessorPaste_InvalidToken,

  CT_PreprocessorInclude,
  CT_PreprocessorIncludeNext,

  CT_PreprocessorIf,
  CT_PreprocessorElse,
  CT_PreprocessorElif,
  CT_PreprocessorEndif,
  CT_PreprocessorIfDefined,
  CT_PreprocessorIfNotDefined,
  CT_PreprocessorDefine,
  CT_PreprocessorUndef,
  CT_PreprocessorPragma,
  CT_PreprocessorError,
  CT_PreprocessorWarning,
  CT_Preprocessor_VA_ARGS_,
  CT_Preprocessor_Nuked,

  CT_InsertedCode,     // This is how macros and includes get inserted

  CT_PoofInsertedCode, // This hijacks an existing token so poof can insert arbitrary stuff into source files
  CT_PoofModifiedToken,
};
poof(generate_string_table(c_token_type))
#include <generated/generate_string_table_c_token_type.h>

enum c_token_flags
{
  CTFlags_None = 0,
  CTFlags_RelativeInclude = 1 << 0,
};


enum token_cursor_source
{
  TokenCursorSource_Unknown,

  TokenCursorSource_RootFile,
  TokenCursorSource_Include,
  TokenCursorSource_MacroExpansion,
  TokenCursorSource_MetaprogrammingExpansion,
  TokenCursorSource_PoofSymbolIteration,
  TokenCursorSource_PasteOperator,
  TokenCursorSource_CommandLineOption,
  TokenCursorSource_BodyText,

  // NOTE(Jesse): These should be created temporarily and discarded.  This type
  // should never be seen in "user" code.. it's just used in parsing.
  TokenCursorSource_IntermediateRepresentaton,

  TokenCursorSource_Count,
};
poof(generate_string_table(token_cursor_source));
#include <generated/generate_string_table_token_cursor_source.h>


struct c_token
{
  c_token_type Type;
  counted_string Value;

  counted_string Filename;
  u32 LineNumber;
  b32 Erased; // TODO(Jesse): Pack this into Flags

  u8 Flags;

  union
  {
     /* s64         SignedValue; */ // TODO(Jesse id: 272): Fold `-` sign into this value at tokenization time?
     u64            UnsignedValue;
     r64            FloatValue;
     c_token        *QualifierName;

     // NOTE(Jesse): I ordered the 'macro_expansion' struct such that the
     // pointer to the expanded macro will be at the same place as the `Down`
     // poninter.  This is sketchy as fuck, but it'll work, and this the
     // assertions at @janky-macro-expansion-struct-ordering should catch the
     // bug if we reorder the pointers.
     c_token_cursor  *Down;
     macro_expansion Macro;

     counted_string IncludePath; // TODO(Jesse): We probably care that this (and Macro) increase struct size by 8.  Heap allocate to fix?
  };

  // TODO(Jesse)(correctness): The preprocessor doesn't support this for some reason..
  operator bool()
  {
    b32 Result = (b32)((u64)Type | Value.Count);
    return Result;
  }

};



// TODO(Jesse): Add a way to append additional members to generated datatypes
// then reenable this.
/* poof(generate_cursor(c_token)) */
#include <generated/generate_cursor_c_token.h>

poof(buffer(c_token))
#include <generated/buffer_c_token.h>

poof(buffer(c_token_buffer))
#include <generated/buffer_c_token_buffer.h>

poof(generate_stream(c_token_buffer))
#include <generated/generate_stream_c_token_buffer.h>


enum parse_warn_code
{
  ParseWarnCode_None,

  ParseWarnCode_MacroRedefined,
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
  ParseErrorCode_InvalidOperator,
  ParseErrorCode_InvalidMetaTransformOp,
  ParseErrorCode_InvalidArgument,
  ParseErrorCode_InvalidArgumentType,
  ParseErrorCode_InvalidArgumentCount,
  ParseErrorCode_InvalidName,
  ParseErrorCode_InvalidFunction,
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





struct peek_result
{
  c_token_cursor *Tokens;

  c_token *At;
  b32 DoNotDescend;
};

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

