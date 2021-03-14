////////////////////////////////
//~ Basic Unicode string types.

@struct MD_String8: {
 str: *MD_u8,
 size: MD_u64,
};

@struct MD_String16: {
 str: *MD_u16,
 size: MD_u64,
};

@struct MD_String32: {
 str: *MD_u32,
 size: MD_u64,
};

@struct MD_String8Node: {
 next: MD_String8Node,
 string: MD_String8,
};

@struct MD_String8List: {
 node_count: MD_u64,
 total_size: MD_u64,
 first: *MD_String8Node,
 last: *MD_String8Node,
};

@prefix(MD_StringMatchFlag)
@base_type(MD_u32)
@flags MD_StringMatchFlags: {
 CaseInsensitive,
 RightSideSloppy,
 FindLast,
 SlashInsensitive,
};

@struct MD_UnicodeConsume: {
 codepoint: MD_u32,
 advance: MD_u32,
};

@enum MD_WordStyle: {
 UpperCamelCase,
 LowerCamelCase,
 UpperCase,
 LowerCase,
};

////////////////////////////////
//~ Node types that are used to build all ASTs.

@enum MD_NodeKind: {
    Nil,
    File,
    Label,
    UnnamedSet,
    Tag,
    MAX,
};

@prefix(MD_NodeFlag)
@base_type(MD_u32)
@flags MD_NodeFlags: {
 ParenLeft,
 ParenRight,
 BracketLeft,
 BracketRight,
 BraceLeft,
 BraceRight,
 
 BeforeSemicolon,
 BeforeComma,
 
 AfterSemicolon,
 AfterComma,
 
 Numeric,
 Identifier,
 StringLiteral,
 CharLiteral,
};

@prefix(MD_NodeMatchFlag)
@base_type(MD_u32)
@flags MD_NodeMatchFlags: {
 MD_NodeMatchFlag_Tags,
 MD_NodeMatchFlag_TagArguments,
};

@struct MD_Node: {
 next: *MD_Node,
 prev: *MD_Node,
 parent: *MD_Node,
 first_child: *MD_Node,
 last_child: *MD_Node,
 
 // Tag list.
 first_tag: *MD_Node,
 last_tag: *MD_Node,
 
 // Node info.
 kind: MD_NodeKind,
 flags: MD_NodeFlags,
 string: MD_String8,
 whole_string: MD_String8,
 string_hash: MD_u64,
 
 // Source code location information.
 filename: MD_String8,
 file_contents: *MD_u8,
 at: *MD_u8,
};

////////////////////////////////
//~ Code Location Info.

@struct MD_CodeLoc: {
    filename: MD_String8,
    line: MD_u32,
    column: MD_u32,
};

////////////////////////////////
//~ Warning Levels

@enum MD_MessageKind: {
    Error,
    Warning,
}

////////////////////////////////
//~ String-To-Node table

@enum MD_NodeTableCollisionRule: {
    Chain,
    Overwrite,
}

@struct MD_NodeTableSlot: {
    next: *MD_NodeTableSlot,
    hash: MD_u64,
    node: *MD_Node,
};

@struct MD_NodeTable: {
    table_size: MD_u64,
    table: **MD_NodeTableSlot,
};

////////////////////////////////
//~ Tokens

@enum MD_TokenKind: {
 Nil,
 
 RegularMin,
 
 // A group of characters that begins with an underscore or alphabetic character,
 // and consists of numbers, alphabetic characters, or underscores after that.
 Identifier,
 
 // A group of characters beginning with a numeric character or a '-', and then
 // consisting of only numbers, alphabetic characters, or '.'s after that.
 NumericLiteral,
 
 // A group of arbitrary characters, grouped together by a " character, OR by a
 // """ symbol at the beginning and end of the group. String literals beginning with
 // " are to only be specified on a single line, but """ strings can exist across
 // many lines.
 StringLiteral,
 
 // A group of arbitrary characters, grouped together by a ' character at the
 // beginning, and a ' character at the end.
 CharLiteral,
 
 // A group of symbolic characters. The symbolic characters are:
 // ~!@#$%^&*()-+=[{]}:;<>,./?|\
 //
 // Groups of multiple characters are only allowed in specific circumstances. Most of these
 // are only 1 character long, but some groups are allowed:
 //
 // "<<", ">>", "<=", ">=", "+=", "-=", "*=", "/=", "::", ":=", "==", "&=", "|=", "->"
 Symbol,
 
 RegularMax,
 
 Comment,
 
 WhitespaceMin,
 Whitespace,
 Newline,
 WhitespaceMax,
 
 MAX,
};

@struct MD_Token: {
 kind: MD_TokenKind,
 string: MD_String8,
 outer_string: MD_String8,
};

@prefix(MD_TokenGroup)
@base_type(MD_u32)
@flags MD_TokenGroups: {
 Comment,
 Whitespace,
 Regular,
};

////////////////////////////////
//~ Parsing State

@struct MD_Error: {
    next: *MD_Error;
    string: MD_String8;
    filename: MD_String8;
    node: *MD_Node;
};

@struct MD_ParseCtx: {
    first_root: *MD_Node,
    last_root: *MD_Node,
    first_error: *MD_Error,
    last-error: *MD_Error,
    at: *MD_u8,
    filename: MD_String8,
    file_contents: MD_String8,
};

@struct MD_ParseResult: {
    node: *MD_Node;
    first_error: *MD_Error;
    bytes_parse: MD_u64;
};

////////////////////////////////
//~ Expression and Type-Expression parser helper types.

@enum MD_ExprKind: {
 // VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
 // it is kept in-sync with the MD_ExprPrecFromExprKind and the following
 // functions:
 //
 // MD_BinaryExprKindFromNode
 // MD_PreUnaryExprKindFromNode
 // MD_PostUnaryExprKindFromNode
 
 Nil,
 
 // NOTE(rjf): Atom
 Atom,
 
 // NOTE(rjf): Access
 Dot,
 Arrow,
 Call,
 Subscript,
 Dereference,
 Reference,
 
 // NOTE(rjf): Arithmetic
 Add,
 Subtract,
 Multiply,
 Divide,
 Mod,
 
 // NOTE(rjf): Comparison
 IsEqual,
 IsNotEqual,
 LessThan,
 GreaterThan,
 LessThanEqualTo,
 GreaterThanEqualTo,
 
 // NOTE(rjf): Bools
 BoolAnd,
 BoolOr,
 BoolNot,
 
 // NOTE(rjf): Bitwise
 BitAnd,
 BitOr,
 BitNot,
 BitXor,
 LeftShift,
 RightShift,
 
 // NOTE(rjf): Unary numeric
 Negative,
 
 // NOTE(rjf): Type
 Pointer,
 Array,
 
 MAX,
}

@typedef(MD_i32) MD_ExprPrec;

@struct MD_Expr: {
 node: *MD_Node,
 kind: MD_ExprKind,
 parent: *MD_Expr,
 sub: [2]*MD_Expr,
};

////////////////////////////////
//~ Command line parsing helper types.

@struct MD_CommandLine: {
 arguments: *MD_String8;
 argument_count: MD_u32;
};

////////////////////////////////
//~ File system access types.

@prefix(MD_FileFlag)
@base_type(MD_u32)
@flags MD_FileFlags: {
 Directory,
};


@struct MD_FileInfo: {
 flags: MD_FileFlags;
 filename: MD_String8;
 file_size: MD_u64;
};

@struct MD_FileIter: {
 state: MD_u64,
};

////////////////////////////////
//~ Basic Utilities

@macro MD_Assert: {
 c,
};

@macro MD_ArrayCount: {
 a,
};

////////////////////////////////
//~ Characters

@func MD_CharIsAlpha: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsAlphaUpper: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsAlphaLower: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsDigit: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsSymbol: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsSpace: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharToUpper: {
 c: MD_u8,
 return: MD_u8,
};

@func MD_CharToLower: {
 c: MD_u8,
 return: MD_u8,
};

@func MD_CorrectSlash: {
 c: MD_u8,
 return: MD_u8,
};

////////////////////////////////
//~ Strings

@func MD_S8: {
 str: *MD_u8,
 size: MD_u64,
 return: MD_String8,
};

@macro MD_S8CString: {
 s,
};

@macro MD_S8Lit: {
 s,
};

@func MD_S8Range: {
 str: *MD_u8,
 opl: *MD_u8,
 return: MD_String8,
};

@func MD_StringSubstring: {
 str: MD_String8,
 min: MD_u64,
 max: MD_u64
 return: MD_String8,
};

@func MD_StringSkip: {
 str: MD_String8,
 min: MD_u64,
 return: MD_String8,
};

@func MD_StringChop: {
 str: MD_String8,
 nmax: MD_u64,
 return: MD_String8,
};

@func MD_StringPrefix: {
 str: MD_String8,
 size: MD_u64,
 return: MD_String8,
};

@func MD_StringSuffix: {
 str: MD_String8,
 size: MD_u64,
 return: MD_String8,
};

@func MD_StringMatch: {
 a: MD_String8,
 b: MD_String8,
 flags: MD_StringMatchFlags,
 return: MD_b32,
};

@func MD_FindSubstring: {
 str: MD_String8,
 substring: MD_String8,
 start_pos: MD_u64,
 flags: MD_StringMatchFlags,
 return: MD_u64,
};

@func MD_FindLastSubstring: {
 str: MD_String8,
 substring: MD_String8,
 flags: MD_StringMatchFlags,
 return: MD_u64,
};

@func MD_TrimExtension: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_TrimFolder: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_ExtensionFromPath: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_FolderFromPath: {
 string: MD_String8,
 return: MD_String8,
};


@func MD_String8     MD_PushStringCopy: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_String8     MD_PushStringFV: {
 fmt: *char,
 args: va_list,
 return: MD_String8,
};

@func MD_String8     MD_PushStringF: {
 fmt: *char,
 "...",
 return: MD_String8,
};

@macro MD_StringExpand: { s, }




