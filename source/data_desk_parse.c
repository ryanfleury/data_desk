/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Data Desk

Author  : Ryan Fleury
Updated : 5 December 2019
License : MIT, at end of file.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct ParseError ParseError;
struct ParseError
{
    char *string;
    char *file;
    int line;
};

typedef struct ParseContextSymbolTableKey ParseContextSymbolTableKey;
struct ParseContextSymbolTableKey
{
    char *key;
    int key_length;
    int namespace_index;
};

typedef struct ParseContextSymbolTableValue ParseContextSymbolTableValue;
struct ParseContextSymbolTableValue
{
    DataDeskNode *root;
};

enum        // TODO(mal): Move to data_desk.h if user-facing
{
    NAMESPACE_INDEX_GLOBAL = 0,
};

typedef struct NamespaceNode NamespaceNode;
struct NamespaceNode
{
    NamespaceNode *next;
    char *name;
    int index;
};

typedef struct ParseContext ParseContext;
struct ParseContext
{
    MemoryArena arena;
    int error_stack_size;
    int error_stack_max;
    ParseError *error_stack;
    DataDeskNode *tag_stack_head;
    DataDeskNode *tag_stack_tail;
    unsigned int symbol_table_max;
    unsigned int symbol_table_count;
    ParseContextSymbolTableKey *symbol_table_keys;
    ParseContextSymbolTableValue *symbol_table_values;
    
    struct
    {
        NamespaceNode global_namespace;
        NamespaceNode *namespace_head;  // NOTE(mal): Always points to global_namespace
        NamespaceNode *namespace_tail;
        NamespaceNode *current_namespace;
        int namespace_count;
    };
};

static void
ParseContextInitNamespaces(ParseContext *context)
{
    context->namespace_count = 1;
    context->current_namespace = context->namespace_head = context->namespace_tail = &context->global_namespace;
}

static void
ParseContextCleanUp(ParseContext *context)
{
    MemoryArenaClear(&context->arena);
}

static unsigned int global_crc32_table[] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

static unsigned int
ParseContextSymbolTableHash(ParseContextSymbolTableKey symbol_key)
{
    unsigned int crc = symbol_key.namespace_index;
    for(int i = 0; symbol_key.key[i] && i < symbol_key.key_length; ++i)
    {
        crc = (crc << 8) ^ global_crc32_table[((crc >> 24) ^ symbol_key.key[i]) & 255];
    }
    return crc;
}

static DataDeskNode *
ParseContextLookUpSymbol(ParseContext *context, ParseContextSymbolTableKey symbol_key)
{
    DataDeskNode *symbol_value = 0;
    if(context->symbol_table_max)
    {
        unsigned int key_hash = ParseContextSymbolTableHash(symbol_key) % context->symbol_table_max;
        unsigned int original_hash = key_hash;
        
        for(;;)
        {
            if(context->symbol_table_keys[key_hash].key)
            {
                if(StringMatchCaseSensitiveN(context->symbol_table_keys[key_hash].key, symbol_key.key, symbol_key.key_length))
                {
                    symbol_value = context->symbol_table_values[key_hash].root;
                    break;
                }
                else
                {
                    if(++key_hash >= context->symbol_table_max)
                    {
                        key_hash = 0;
                    }
                    if(key_hash == original_hash)
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    
    if(!symbol_value && symbol_key.namespace_index != NAMESPACE_INDEX_GLOBAL)
    {
        symbol_key.namespace_index = NAMESPACE_INDEX_GLOBAL;
        symbol_value = ParseContextLookUpSymbol(context, symbol_key);
    }
    
    return symbol_value;
}

enum
{
    PARSE_CONTEXT_ADD_SYMBOL_MEMORY_FAILURE,
    PARSE_CONTEXT_ADD_SYMBOL_ALREADY_DEFINED,
    PARSE_CONTEXT_ADD_SYMBOL_SUCCESS,
};

static int
ParseContextAddSymbol(ParseContext *context, char *key, int key_length, DataDeskNode *root)
{
    int result = PARSE_CONTEXT_ADD_SYMBOL_MEMORY_FAILURE;
    
    // NOTE(rjf): Reallocate symbol table if necessary (if the count we have is 75%+ of
    // the allocated size of the symbol table).
    if(context->symbol_table_count >= (context->symbol_table_max * 3) / 4)
    {
        // NOTE(rjf): If we're resizing, increase size by a factor of 1.5x (exponentially),
        // and fall back on 1024 (this case happens if this is the first add ever).
        unsigned int new_symbol_table_max = (context->symbol_table_max * 3) / 2;
        if(new_symbol_table_max == 0)
        {
            new_symbol_table_max = 1024;
        }
        
        ParseContextSymbolTableKey *new_symbol_table_keys = calloc(sizeof(ParseContextSymbolTableKey), new_symbol_table_max);
        ParseContextSymbolTableValue *new_symbol_table_values = calloc(sizeof(ParseContextSymbolTableValue), new_symbol_table_max);
        
        // NOTE(rjf): Re-hash all current keys to the new table memory.
        // TODO(rjf): Can this be done more intelligently?
        for(unsigned int i = 0; i < context->symbol_table_max; ++i)
        {
            if(context->symbol_table_keys[i].key)
            {
                ParseContextSymbolTableKey old_symbol_key = context->symbol_table_keys[i];
                DataDeskNode *old_root = context->symbol_table_values[i].root;
                
                unsigned int new_hash = ParseContextSymbolTableHash(old_symbol_key) % new_symbol_table_max;
                unsigned int original_new_hash = new_hash;
                
                for(;;)
                {
                    if(new_symbol_table_keys[new_hash].key)
                    {
                        if(StringMatchCaseSensitive(new_symbol_table_keys[new_hash].key, old_symbol_key.key))
                        {
                            new_symbol_table_values[new_hash].root = old_root;
                            break;
                        }
                        else
                        {
                            if(++new_hash >= new_symbol_table_max)
                            {
                                new_hash = 0;
                            }
                            
                            if(new_hash == original_new_hash)
                            {
                                // NOTE(rjf): This case should really never happen, when we don't find
                                // an open slot in the *resized* hash table.
                                Assert("Resized hash table doesn't have space...?" == 0);
                                break;
                            }
                        }
                    }
                    else
                    {
                        new_symbol_table_keys[new_hash] = old_symbol_key;
                        new_symbol_table_values[new_hash].root = old_root;
                        break;
                    }
                }
            }
        }
        
        free(context->symbol_table_keys);
        free(context->symbol_table_values);
        context->symbol_table_keys   = new_symbol_table_keys;
        context->symbol_table_values = new_symbol_table_values;
        context->symbol_table_max    = new_symbol_table_max;
    }
    
    ParseContextSymbolTableKey new_symbol_key = {0};
    new_symbol_key.key = key;
    new_symbol_key.key_length = key_length;
    new_symbol_key.namespace_index = context->current_namespace->index;
    
    unsigned int key_hash = ParseContextSymbolTableHash(new_symbol_key) % context->symbol_table_max;
    unsigned int original_hash = key_hash;
    
    for(;;)
    {
        if(context->symbol_table_keys[key_hash].key)
        {
            if(StringMatchCaseSensitiveN(context->symbol_table_keys[key_hash].key, key, key_length))
            {
                if(context->symbol_table_values[key_hash].root)
                {
                    result = PARSE_CONTEXT_ADD_SYMBOL_ALREADY_DEFINED;
                }
                else
                {
                    result = PARSE_CONTEXT_ADD_SYMBOL_SUCCESS;
                    context->symbol_table_keys[key_hash].key = key;
                    context->symbol_table_keys[key_hash].key_length = key_length;
                    context->symbol_table_values[key_hash].root = root;
                    ++context->symbol_table_count;
                }
                break;
            }
            else
            {
                if(++key_hash >= context->symbol_table_max)
                {
                    key_hash = 0;
                }
                if(key_hash == original_hash)
                {
                    break;
                }
            }
        }
        else
        {
            context->symbol_table_keys[key_hash].key = key;
            context->symbol_table_keys[key_hash].key_length = key_length;
            context->symbol_table_values[key_hash].root = root;
            result = PARSE_CONTEXT_ADD_SYMBOL_SUCCESS;
            ++context->symbol_table_count;
            break;
        }
    }
    
    return result;
}

static void *
ParseContextAllocateMemory(ParseContext *context, unsigned int size)
{
    return MemoryArenaAllocate(&context->arena, size);
}

static void
BumpEndLine(DataDeskNode *node, int line)
{
    if(node && node->end_line < line)
    {
        node->end_line = line;
    }
}

static DataDeskNode *
ParseContextAllocateNode(ParseContext *context, Tokenizer *tokenizer, Token token,
                         DataDeskNodeType type)
{
    DataDeskNode *node = ParseContextAllocateMemory(context, sizeof(DataDeskNode));
    MemorySet(node, 0, sizeof(*node));
    node->type = type;
    node->file = tokenizer->filename;
    node->line = tokenizer->line;
    // node->end_line = node->line;
    node->string = token.string;
    node->string_length = token.string_length;
    
    if(context->namespace_count == 0){
        ParseContextInitNamespaces(context);
    }
    
    node->namespace_index = context->current_namespace->index;
    
    if(type == DataDeskNodeType_Identifier ||
       type == DataDeskNodeType_StructDeclaration ||
       type == DataDeskNodeType_UnionDeclaration ||
       type == DataDeskNodeType_EnumDeclaration ||
       type == DataDeskNodeType_FlagsDeclaration ||
       type == DataDeskNodeType_ConstantDefinition ||
       type == DataDeskNodeType_ProcedureHeader ||
       type == DataDeskNodeType_Declaration)
    {
        node->namespace_string = context->current_namespace->name;
    }
    return node;
}

static DataDeskNode *
InsertChild(DataDeskNode *node, DataDeskNode *parent)
{
    BumpEndLine(parent, node ? node->line : 0);
    
    DataDeskNode *tail = node;
    for(DataDeskNode *child = node; child; child = child->next)
    {
        child->parent = parent;
        tail = child;
    }
    
    if(parent->children_list_tail == 0)
    {
        parent->children_list_head = parent->children_list_tail = node;
    }
    else
    {
        parent->children_list_tail->next = node;
        node->prev = parent->children_list_tail;
        parent->children_list_tail = tail;
    }
    
    return node;
}

static void
ParseContextPushTag(ParseContext *context, DataDeskNode *tag)
{
    if(context->tag_stack_tail == 0)
    {
        context->tag_stack_head = context->tag_stack_tail = tag;
    }
    else
    {
        context->tag_stack_tail->next = tag;
        tag->prev = context->tag_stack_tail;
        context->tag_stack_tail = tag;
        tag->next = 0;
    }
}

static DataDeskNode *
ParseContextPopAllTags(ParseContext *context)
{
    DataDeskNode *tags_head = context->tag_stack_head;
    context->tag_stack_head = context->tag_stack_tail = 0;
    return tags_head;
}

static char *
ParseContextAllocateStringCopyLowercaseWithUnderscores(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        int last_character_was_lowercase = 0;
        int bytes_needed = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] >= 'a' && string[i] <= 'z')
            {
                last_character_was_lowercase = 1;
            }
            else if(string[i] >= 'A' && string[i] <= 'Z' &&
                    last_character_was_lowercase)
            {
                ++bytes_needed;
            }
            ++bytes_needed;
        }
        
        ++bytes_needed;
        
        new_string = ParseContextAllocateMemory(context, bytes_needed);
        int new_string_write_pos = 0;
        
        last_character_was_lowercase = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] >= 'a' && string[i] <= 'z')
            {
                last_character_was_lowercase = 1;
            }
            if(string[i] >= 'A' && string[i] <= 'Z' &&
               last_character_was_lowercase)
            {
                new_string[new_string_write_pos++] = '_';
                last_character_was_lowercase = 0;
            }
            
            new_string[new_string_write_pos++] = CharToLower(string[i]);
        }
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyUppercaseWithUnderscores(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        new_string = ParseContextAllocateStringCopyLowercaseWithUnderscores(context, string);
        for(int i = 0; new_string[i]; ++i)
        {
            new_string[i] = CharToUpper(new_string[i]);
        }
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyWithSpaces(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        int need_uppercase = 1;
        new_string = ParseContextAllocateStringCopyLowercaseWithUnderscores(context, string);
        for(int i = 0; new_string[i]; ++i)
        {
            if(CharIsAlpha(new_string[i]))
            {
                if(need_uppercase)
                {
                    new_string[i] = CharToUpper(new_string[i]);
                    need_uppercase = 0;
                }
                else
                {
                    new_string[i] = new_string[i];
                }
            }
            else
            {
                if(new_string[i] == '_')
                {
                    new_string[i] = ' ';
                    need_uppercase = 1;
                }
            }
        }
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyLowerCamelCase(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        int bytes_needed = 0;
        int found_alpha = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(!found_alpha || string[i] != '_')
            {
                ++bytes_needed;
            }
            
            if(CharIsAlpha(string[i]))
            {
                found_alpha = 1;
            }
        }
        
        ++bytes_needed;
        
        new_string = ParseContextAllocateMemory(context, bytes_needed);
        int new_string_write_pos = 0;
        found_alpha = 0;
        int need_capital = 0;
        
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] == '_' && found_alpha)
            {
                need_capital = 1;
            }
            else
            {
                new_string[new_string_write_pos++] = need_capital ? CharToUpper(string[i]) : string[i];
                
                if(CharIsAlpha(string[i]))
                {
                    found_alpha = 1;
                }
                
                need_capital = 0;
            }
        }
    }
    return new_string;
}

static char *
ParseContextAllocateStringCopyUpperCamelCase(ParseContext *context, char *string)
{
    char *new_string = 0;
    if(string)
    {
        new_string = ParseContextAllocateStringCopyLowerCamelCase(context, string);
        for(int i = 0; new_string[i]; ++i)
        {
            if(CharIsAlpha(new_string[i]))
            {
                new_string[i] = CharToUpper(new_string[i]);
                break;
            }
        }
    }
    return new_string;
}

static void
ParseContextPushError(ParseContext *context, Tokenizer *tokenizer, char *msg, ...)
{
    if(!context->error_stack)
    {
        context->error_stack_max = 16;
        context->error_stack = ParseContextAllocateMemory(context,
                                                          sizeof(ParseError) * context->error_stack_max);
    }
    
    if(context->error_stack_size < context->error_stack_max)
    {
        va_list args;
        va_start(args, msg);
        unsigned int msg_bytes = vsnprintf(0, 0, msg, args)+1;
        va_end(args);
        va_start(args, msg);
        char *stored_msg = ParseContextAllocateMemory(context, msg_bytes);
        Assert(stored_msg != 0);
        vsnprintf(stored_msg, msg_bytes, msg, args);
        va_end(args);
        
        ParseError error =
        {
            stored_msg,
            tokenizer->filename,
            tokenizer->line,
        };
        context->error_stack[context->error_stack_size++] = error;
    }
}

static DataDeskNode *
ParseExpression(ParseContext *context, Tokenizer *tokenizer);

static DataDeskNode *ParseCode(ParseContext *context, Tokenizer *tokenizer);

static void
ParseTagList(ParseContext *context, Tokenizer *tokenizer)
{
    for(;;)
    {
        Token tag = {0};
        DataDeskNode *tag_node = 0;
        if(RequireTokenType(tokenizer, Token_Tag, &tag))
        {
            tag_node = ParseContextAllocateNode(context, tokenizer, tag, DataDeskNodeType_Tag);
            
            if(RequireToken(tokenizer, "(", 0))
            {
                for(;;)
                {
                    DataDeskNode *parameter = ParseExpression(context, tokenizer);
                    if(parameter)
                    {
                        InsertChild(parameter, tag_node);
                    }
                    
                    if(RequireToken(tokenizer, ")", 0))
                    {
                        break;
                    }
                    if(!RequireToken(tokenizer, ",", 0))
                    {
                        ParseContextPushError(context, tokenizer, "Expected ','.");
                    }
                }
                
                tag_node->tag.first_tag_parameter = tag_node->children_list_head;
            }
            
            ParseContextPushTag(context, tag_node);
        }
        else
        {
            break;
        }
    }
}

static int
BinaryOperatorPrecedence(int type)
{
    int precedence = 0;
    
    static int precedence_table[DataDeskBinaryOperatorType_Max] =
    {
        0, // Invalid
        1, // Add
        1, // Subtract
        2, // Multiply
        2, // Divide
        2, // Modulus
        3, // Bitshift Left
        3, // Bitshift Right
        4, // Bitwise And
        4, // Bitwise Or
        5, // Boolean And
        5, // Boolean Or
    };
    
    if(type >= 0 && type < DataDeskBinaryOperatorType_Max)
    {
        precedence = precedence_table[type];
    }
    
    return precedence;
}

static int
UnaryOperatorPrecedence(int type)
{
    int precedence = 0;
    
    static int precedence_table[DataDeskUnaryOperatorType_Max] =
    {
        0, // Invalid
        6, // Negative
        6, // Not
        6, // Bitwise negate
    };
    
    if(type >= 0 && type < DataDeskUnaryOperatorType_Max)
    {
        precedence = precedence_table[type];
    }
    
    return precedence;
}

static DataDeskNode *
ParseUnaryExpression(ParseContext *context, Tokenizer *tokenizer)
{
    DataDeskNode *expression = 0;
    
    Token token = PeekToken(tokenizer);
    
    if(TokenMatch(token, "("))
    {
        NextToken(tokenizer);
        expression = ParseExpression(context, tokenizer);
        if(!RequireToken(tokenizer, ")", 0))
        {
            ParseContextPushError(context, tokenizer, "Missing ')'.");
        }
    }
    else if(TokenMatch(token, "-"))
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token, DataDeskNodeType_UnaryOperator);
        expression->sub_type = DataDeskUnaryOperatorType_Negative;
        DataDeskNode *subexpr = ParseExpression(context, tokenizer);
        InsertChild(subexpr, expression);
    }
    else if(TokenMatch(token, "!"))
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token, DataDeskNodeType_UnaryOperator);
        expression->sub_type = DataDeskUnaryOperatorType_Not;
        DataDeskNode *subexpr = ParseExpression(context, tokenizer);
        InsertChild(subexpr, expression);
    }
    else if(TokenMatch(token, "~"))
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token, DataDeskNodeType_UnaryOperator);
        expression->sub_type = DataDeskUnaryOperatorType_BitwiseNegate;
        DataDeskNode *subexpr = ParseExpression(context, tokenizer);
        InsertChild(subexpr, expression);
    }
    else if(token.type == Token_NumericConstant)
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token,
                                              DataDeskNodeType_NumericConstant);
    }
    else if(token.type == Token_AlphanumericBlock)
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token,
                                              DataDeskNodeType_Identifier);
    }
    else if(token.type == Token_StringConstant)
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token,
                                              DataDeskNodeType_StringConstant);
    }
    else if(token.type == Token_CharConstant)
    {
        NextToken(tokenizer);
        expression = ParseContextAllocateNode(context, tokenizer, token,
                                              DataDeskNodeType_CharConstant);
    }
    
    return expression;
}

static DataDeskNode *
ParseExpression_(ParseContext *context, Tokenizer *tokenizer, int precedence_in)
{
    DataDeskNode *expression = ParseUnaryExpression(context, tokenizer);
    
    if(!expression)
    {
        goto end_parse;
    }
    
    Token token = PeekToken(tokenizer);
    DataDeskBinaryOperatorType operator_type = GetBinaryOperatorTypeFromToken(token);
    
    if(operator_type != DataDeskBinaryOperatorType_Invalid)
    {
        for(int precedence = BinaryOperatorPrecedence(operator_type); precedence >= precedence_in;
            --precedence)
        {
            for(;;)
            {
                token = PeekToken(tokenizer);
                operator_type = GetBinaryOperatorTypeFromToken(token);
                int operator_precedence = BinaryOperatorPrecedence(operator_type);
                
                if(operator_precedence != precedence)
                {
                    break;
                }
                
                if(operator_type == DataDeskBinaryOperatorType_Invalid)
                {
                    break;
                }
                
                NextToken(tokenizer);
                
                DataDeskNode *right = ParseExpression_(context, tokenizer, precedence+1);
                if(!right)
                {
                    ParseContextPushError(context, tokenizer, "Expected right-hand-side of binary expression.");
                    goto end_parse;
                }
                
                DataDeskNode *left = expression;
                
                expression = ParseContextAllocateNode(context, tokenizer, token,
                                                      DataDeskNodeType_BinaryOperator);
                expression->sub_type = operator_type;
                InsertChild(left, expression);
                InsertChild(right, expression);
            }
        }
    }
    
    end_parse:;
    return expression;
}

static DataDeskNode *
ParseExpression(ParseContext *context, Tokenizer *tokenizer)
{
    return ParseExpression_(context, tokenizer, 1);
}

static DataDeskNode *ParseDeclarationBody     (ParseContext *context, Tokenizer *tokenizer, Token name);
static DataDeskNode *ParseStructBody          (ParseContext *context, Tokenizer *tokenizer, Token name);
static DataDeskNode *ParseUnionBody           (ParseContext *context, Tokenizer *tokenizer, Token name);
static DataDeskNode *ParseEnumBody            (ParseContext *context, Tokenizer *tokenizer, Token name);
static DataDeskNode *ParseFlagsBody           (ParseContext *context, Tokenizer *tokenizer, Token name);
static DataDeskNode *ParseProcedureHeaderBody (ParseContext *context, Tokenizer *tokenizer, Token name);

static DataDeskNode *
ParseType(ParseContext *context, Tokenizer *tokenizer)
{
    DataDeskNode *type = 0;
    
    Token token = {0};
    if(RequireToken(tokenizer, "*", &token))
    {
        type = ParseContextAllocateNode(context, tokenizer, token,
                                        DataDeskNodeType_TypeDecorator);
        type->sub_type = DataDeskTypeDecoratorType_Pointer;
        
        DataDeskNode *operand = ParseType(context, tokenizer);
        InsertChild(operand, type);
    }
    else if(RequireToken(tokenizer, "[", &token))
    {
        type = ParseContextAllocateNode(context, tokenizer, token,
                                        DataDeskNodeType_TypeDecorator);
        type->sub_type = DataDeskTypeDecoratorType_Array;
        
        DataDeskNode *array_expr = ParseExpression(context, tokenizer);
        
        if(!array_expr)
        {
            goto end_parse;
        }
        
        if(!RequireToken(tokenizer, "]", 0))
        {
            ParseContextPushError(context, tokenizer, "Missing ']'.");
        }
        
        DataDeskNode *operand = ParseType(context, tokenizer);
        
        InsertChild(operand, type);
        InsertChild(array_expr, type);
    }
    else if(RequireToken(tokenizer, "struct", &token))
    {
        type = ParseStructBody(context, tokenizer, (Token){0});
    }
    else if(RequireToken(tokenizer, "union", &token))
    {
        type = ParseUnionBody(context, tokenizer, (Token){0});
    }
    else if(RequireTokenType(tokenizer, Token_AlphanumericBlock, &token))
    {
        type = ParseContextAllocateNode(context, tokenizer, token,
                                        DataDeskNodeType_Identifier);
    }
    
    end_parse:;
    return type;
}

static DataDeskNode *
ParseCode(ParseContext *context, Tokenizer *tokenizer)
{
    DataDeskNode *head = 0;
    DataDeskNode *tail = 0;
    
    Token token = {0};
    
    context->current_namespace = &context->global_namespace; // NOTE(mal): Revert back to global_namespace for each new file
    
    do
    {
        if(RequireToken(tokenizer, "namespace", 0))
        {
            Token namespace_token = {0};
            if(RequireTokenType(tokenizer, Token_AlphanumericBlock, &namespace_token))
            {
                NamespaceNode *target_namespace = 0;
                
                if(context->namespace_count == 0){
                    ParseContextInitNamespaces(context);
                }
                
                for(NamespaceNode *node = context->namespace_head->next; node; node = node->next){
                    if(StringMatchCaseSensitiveN(node->name, namespace_token.string, namespace_token.string_length) &&
                       node->name[namespace_token.string_length] == 0)
                    {
                        target_namespace = node;
                        break;
                    }
                }
                
                if(!target_namespace)
                {
                    target_namespace = ParseContextAllocateMemory(context, sizeof(NamespaceNode));
                    
                    context->namespace_tail->next = target_namespace;
                    context->namespace_tail = target_namespace;
                    
                    target_namespace->name = ParseContextAllocateMemory(context, namespace_token.string_length+1);
                    MemoryCopy(target_namespace->name, namespace_token.string, namespace_token.string_length);
                    target_namespace->name[namespace_token.string_length] = 0;
                    
                    target_namespace->index = context->namespace_count++;
                }
                
                context->current_namespace = target_namespace;
            }
            else
            {
#if 0
                ParseContextPushError(context, tokenizer, "Expected namespace identifier.");
#else
                // NOTE(mal): Selects global namespace if no namespaces is specified
                context->current_namespace = &context->global_namespace;
#endif
            }
            
            if(!RequireToken(tokenizer, ";", 0))
            {
                ParseContextPushError(context, tokenizer, "Expected ';'.");
            }
        }
        
        ParseTagList(context, tokenizer);
        DataDeskNode *tag_list = ParseContextPopAllTags(context);
        
        token = PeekToken(tokenizer);
        if(token.type == Token_Invalid)
        {
            break;
        }
        
        DataDeskNode *new_node = 0;
        
        // NOTE(rjf): Try parsing things that start with a name (declarations, structs, etc.).
        Token name = {0};
        if(RequireTokenType(tokenizer, Token_AlphanumericBlock, &name))
        {
            Tokenizer reset_tokenizer = *tokenizer;
            
            // NOTE(rjf): Constant/immutable things (structs/functions/etc.).
            if(RequireToken(tokenizer, "::", 0))
            {
                
                // NOTE(rjf): Struct.
                if(RequireToken(tokenizer, "struct", 0))
                {
                    new_node = ParseStructBody(context, tokenizer, name);
                }
                
                // NOTE(rjf): Union.
                else if(RequireToken(tokenizer, "union", 0))
                {
                    new_node = ParseUnionBody(context, tokenizer, name);
                }
                
                // NOTE(rjf): Enum.
                else if(RequireToken(tokenizer, "enum", 0))
                {
                    new_node = ParseEnumBody(context, tokenizer, name);
                }
                
                // NOTE(rjf): Flags.
                else if(RequireToken(tokenizer, "flags", 0))
                {
                    new_node = ParseFlagsBody(context, tokenizer, name);
                }
                
                // NOTE(rjf): Procedure header.
                else if(RequireToken(tokenizer, "proc", 0))
                {
                    new_node = ParseProcedureHeaderBody(context, tokenizer, name);
                }
                
                // NOTE(rjf): Some constant expression.
                else
                {
                    new_node = ParseContextAllocateNode(context, tokenizer, name,
                                                        DataDeskNodeType_ConstantDefinition);
                    InsertChild(ParseExpression(context, tokenizer), new_node);
                }
                
                RequireToken(tokenizer, ";", 0);
                
                if(new_node)
                {
                    if(ParseContextAddSymbol(context, new_node->string, new_node->string_length, new_node) == PARSE_CONTEXT_ADD_SYMBOL_ALREADY_DEFINED)
                    {
                        ParseContextPushError(context, tokenizer, "\"%.*s\" has already been defined.", new_node->string_length, new_node->string);
                    }
                }
            }
            
            // NOTE(rjf): Global declaration.
            else if(RequireToken(tokenizer, ":", 0))
            {
                new_node = ParseDeclarationBody(context, tokenizer, name);
                
                if(new_node != 0)
                {
                    RequireToken(tokenizer, ";", 0);
                    new_node->first_tag = tag_list;
                    if(ParseContextAddSymbol(context, new_node->string, new_node->string_length, new_node) == PARSE_CONTEXT_ADD_SYMBOL_ALREADY_DEFINED)
                    {
                        ParseContextPushError(context, tokenizer, "\"%.*s\" has already been defined.", new_node->string_length, new_node->string);
                    }
                }
                else
                {
                    break;
                }
            }
            
            else
            {
                *tokenizer = reset_tokenizer;
            }
            
        }
        
        // NOTE(rjf): Expression.
        if(new_node == 0)
        {
            new_node = ParseExpression(context, tokenizer);
            if(new_node != 0)
            {
                if(!RequireToken(tokenizer, ";", 0))
                {
                    ParseContextPushError(context, tokenizer, "Expected ';'.");
                }
            }
        }
        
        // NOTE(rjf): If a node was parsed, link it up, and attach tags to it.
        if(new_node != 0)
        {
            new_node->first_tag = tag_list;
            if(tail == 0)
            {
                head = tail = new_node;
                new_node->next = new_node->prev = 0;
            }
            else
            {
                tail->next = new_node;
                new_node->prev = tail;
                tail = new_node;
            }
        }
        else
        {
            break;
        }
        
        if(context->error_stack_size)
        {
            break;
        }
    }
    while(token.type != Token_Invalid);
    
    return head;
}

static DataDeskNode *
ParseDeclarationBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_Declaration);
    DataDeskNode *type = ParseType(context, tokenizer);
    root->declaration.type = InsertChild(type, root);
    if(RequireToken(tokenizer, "=", 0))
    {
        DataDeskNode *initialization = 0;
        initialization = ParseExpression(context, tokenizer);
        
        if(!initialization)
        {
            goto end_parse;
        }
        
        root->declaration.initialization = InsertChild(initialization, root);
    }
    
    end_parse:;
    return root;
}

static DataDeskNode *
ParseDeclarationList(ParseContext *context, Tokenizer *tokenizer)
{
    DataDeskNode *head = 0;
    DataDeskNode *tail = 0;
    
    for(;;)
    {
        if(TokenMatch(PeekToken(tokenizer), "}") || TokenMatch(PeekToken(tokenizer), ")"))
        {
            break;
        }
        
        ParseTagList(context, tokenizer);
        DataDeskNode *tag_list = ParseContextPopAllTags(context);
        
        Token name = {0};
        if(RequireTokenType(tokenizer, Token_AlphanumericBlock, &name) &&
           RequireToken(tokenizer, ":", 0))
        {
            DataDeskNode *declaration = ParseDeclarationBody(context, tokenizer, name);
            declaration->first_tag = tag_list;
            
            if(tail == 0)
            {
                head = tail = declaration;
            }
            else
            {
                tail->next = declaration;
                declaration->prev = tail;
                tail = tail->next;
            }
            
            if(!(TokenMatch(PeekToken(tokenizer), "}") || TokenMatch(PeekToken(tokenizer), ")")) &&
               !RequireToken(tokenizer, ";", 0) && !RequireToken(tokenizer, ",", 0))
            {
                ParseContextPushError(context, tokenizer, "Expected ';' or ',' after declaration.");
                break;
            }
        }
        else
        {
            ParseContextPushError(context, tokenizer, "Expected declaration (<name> : <type>).");
            break;
        }
    }
    
    return head;
}

static DataDeskNode *
ParseIdentifierList(ParseContext *context, Tokenizer *tokenizer)
{
    DataDeskNode *head = 0;
    DataDeskNode *tail = 0;
    
    for(;;)
    {
        if(TokenMatch(PeekToken(tokenizer), "}") || TokenMatch(PeekToken(tokenizer), ")"))
        {
            break;
        }
        
        ParseTagList(context, tokenizer);
        DataDeskNode *tag_list = ParseContextPopAllTags(context);
        
        Token name = {0};
        if(RequireTokenType(tokenizer, Token_AlphanumericBlock, &name))
        {
            DataDeskNode *identifier = ParseContextAllocateNode(context, tokenizer, name,
                                                                DataDeskNodeType_Identifier);
            identifier->first_tag = tag_list;
            
            if(RequireToken(tokenizer, "=", 0))
            {
                DataDeskNode *initialization = 0;
                initialization = ParseExpression(context, tokenizer);
                
                if(!initialization)
                {
                    break;
                }
                
                InsertChild(initialization, initialization);
            }
            
            if(tail == 0)
            {
                head = tail = identifier;
            }
            else
            {
                tail->next = identifier;
                identifier->prev = tail;
                tail = tail->next;
            }
            
            if(!RequireToken(tokenizer, ";", 0) && !RequireToken(tokenizer, ",", 0))
            {
                ParseContextPushError(context, tokenizer, "Expected ';' or ',' after constant.");
                break;
            }
        }
        else
        {
            ParseContextPushError(context, tokenizer, "Expected identifier.");
            break;
        }
    }
    
    return head;
}

static DataDeskNode *
ParseStructBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_StructDeclaration);
    
    if(!RequireToken(tokenizer, "{", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '{'.");
        goto end_parse;
    }
    
    DataDeskNode *declaration_list =
        ParseDeclarationList(context, tokenizer);
    InsertChild(declaration_list, root);
    
    if(!RequireToken(tokenizer, "}", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '}'.");
        goto end_parse;
    }
    
    BumpEndLine(root, tokenizer->line);
    
    end_parse:;
    return root;
}

static DataDeskNode *
ParseUnionBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_UnionDeclaration);
    
    if(!RequireToken(tokenizer, "{", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '{'.");
        goto end_parse;
    }
    
    InsertChild(ParseDeclarationList(context, tokenizer), root);
    
    if(!RequireToken(tokenizer, "}", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '}'.");
        goto end_parse;
    }
    
    BumpEndLine(root, tokenizer->line);
    
    end_parse:;
    return root;
}

static DataDeskNode *
ParseEnumBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_EnumDeclaration);
    
    if(!RequireToken(tokenizer, "{", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '{'.");
        goto end_parse;
    }
    
    InsertChild(ParseIdentifierList(context, tokenizer), root);
    
    if(!RequireToken(tokenizer, "}", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '}'.");
        goto end_parse;
    }
    
    BumpEndLine(root, tokenizer->line);
    
    end_parse:;
    return root;
}

static DataDeskNode *
ParseFlagsBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_FlagsDeclaration);
    
    if(!RequireToken(tokenizer, "{", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '{'.");
        goto end_parse;
    }
    
    InsertChild(ParseIdentifierList(context, tokenizer), root);
    
    if(!RequireToken(tokenizer, "}", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '}'.");
        goto end_parse;
    }
    
    BumpEndLine(root, tokenizer->line);
    
    end_parse:;
    return root;
}

static DataDeskNode *
ParseProcedureHeaderBody(ParseContext *context, Tokenizer *tokenizer, Token name)
{
    DataDeskNode *root = ParseContextAllocateNode(context, tokenizer, name,
                                                  DataDeskNodeType_ProcedureHeader);
    
    DataDeskNode *declaration_list = 0;
    DataDeskNode *return_type = 0;
    
    if(!RequireToken(tokenizer, "(", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected '('.");
        goto end_parse;
    }
    
    declaration_list = ParseDeclarationList(context, tokenizer);
    
    if(!RequireToken(tokenizer, ")", 0))
    {
        ParseContextPushError(context, tokenizer, "Expected ')'.");
        goto end_parse;
    }
    
    if(RequireToken(tokenizer, "->", 0))
    {
        return_type = ParseType(context, tokenizer);
    }
    
    if(return_type)
    {
        root->procedure_header.return_type = InsertChild(return_type, root);
    }
    
    if(declaration_list)
    {
        root->procedure_header.first_parameter = InsertChild(declaration_list, root);
    }
    
    end_parse:;
    return root;
}

/*
Copyright 2019 Ryan Fleury

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
