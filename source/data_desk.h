/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Data Desk

Author  : Ryan Fleury
Updated : 11 May 2020
License : MIT, at end of file.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef DATA_DESK_H_INCLUDED_
#define DATA_DESK_H_INCLUDED_

#define DATA_DESK_VERSION_MAJOR 1
#define DATA_DESK_VERSION_MINOR 1
#define DATA_DESK_VERSION_PATCH 0
#define DATA_DESK_STRINGIFY_(a) #a
#define DATA_DESK_STRINGIFY(a) DATA_DESK_STRINGIFY_(a)
#define DATA_DESK_VERSION_STRING DATA_DESK_STRINGIFY(DATA_DESK_VERSION_MAJOR) "." DATA_DESK_STRINGIFY(DATA_DESK_VERSION_MINOR) "." DATA_DESK_STRINGIFY(DATA_DESK_VERSION_PATCH)

// NOTE(rjf): Enables definition of old DATA_DESK_NODE_TYPE_* and similar names. This will eventually
// go away in favor of the new DataDeskNodeType_* names.
#ifndef DATA_DESK_USE_OLD_NAMES
#define DATA_DESK_USE_OLD_NAMES 1
#endif

// NOTE(rjf): C Runtime Library
#include <stdio.h>
#include <stdarg.h>

#if defined(_MSC_VER)
#define DATA_DESK_EXPORT __declspec(dllexport)
#if _MSC_VER < 1900
#define MSVC2013_C 1
#endif
#else
#define DATA_DESK_EXPORT
#endif

#if !defined(MSVC2013_C)
#define MSVC2013_C 0
#endif

#if defined(__cplusplus)
#define DATA_DESK_EXTERN_C extern "C"
#else
#define DATA_DESK_EXTERN_C
#endif

#define DATA_DESK_FUNC DATA_DESK_EXPORT DATA_DESK_EXTERN_C
#define DATA_DESK_PROC DATA_DESK_FUNC
#if MSVC2013_C
#define DATA_DESK_HEADER_PROC static
#else
#define DATA_DESK_HEADER_PROC static inline
#endif

// NOTE(rjf): Documentation macros that are evaluated by external tool.
#define DataDeskDoc(...)



/*
| /////////////////////////////////////////////////////////////////
|  Callback Structs/Functions
| /////////////////////////////////////////////////////////////////
|
| These are structures and function typedefs that correspond to
| the callbacks that a custom layer can receive. A custom layer
| doesn't need to define every possible callback. The symbol
| names that Data Desk expects for each callback are listed
| next to the corresponding function typedef.
*/

typedef struct DataDeskNode DataDeskNode;

DataDeskDoc("Defines the expected format for the callback that Data Desk calls when it first starts up. Loaded from custom layers as the symbol \"DataDeskCustomInitCallback\".")
typedef void DataDeskInitCallback(void);

DataDeskDoc("Defines the expected format for the callback that Data Desk calls for each top-level structure that it parses. Loaded from custom layers as the symbol \"DataDeskCustomParseCallback\".")
typedef void DataDeskParseCallback(DataDeskDoc("The root node of the abstract-syntax-tree that was parsed.")
                                   DataDeskNode *root,
                                   
                                   DataDeskDoc("The name of the file from which the tree was parsed.")
                                   char *filename);

DataDeskDoc("Defines the expected format for the callback that Data Desk calls when it shuts down. Loaded from custom layers as the symbol \"DataDeskCleanUpCallback\".")
typedef void DataDeskCleanUpCallback(void);





/*
| /////////////////////////////////////////////////////////////////
|  Abstract Syntax Tree Representation
| /////////////////////////////////////////////////////////////////
|
| The following code outlines the general structure for the
| abstract syntax trees that Data Desk generates. Each sub-struct
| contained inside of the DataDeskNode struct is only safe to
| access if the "type" variable in the struct is set to the
| corresponding type value. All of the type constants are defined
| in the following list.
*/

DataDeskDoc("Used to identify which kind of grammatical object a node represents, stored in the @StructMember(DataDeskNode, type) member of the @Struct(DataDeskNode) structure. Check this member against this enum before assuming that a node is a particular language construct.")
typedef enum DataDeskNodeType
{
    DataDeskDoc("Invalid node type, produced by either invalid code or a bug in the parser.")
        DataDeskNodeType_Invalid,
    
    DataDeskDoc("An alias for DataDeskNodeType_Invalid.")
        DataDeskNodeType_Null = DataDeskNodeType_Invalid,
    
    DataDeskDoc("An identifier node that might reference another abstract-syntax-tree, or just being used as a block of alphanumeric text (like when referencing something in the host language). You can check the @NodeMember(reference) member in nodes of this type to see if Data Desk parsed an abstract-syntax-tree that this node is referring to.")
        DataDeskNodeType_Identifier,
    
    DataDeskDoc("An identifier node that represents a numeric constant in the code. The textual contents representing the intended numeric value are stored in the node's @NodeMember(string) member.")
        DataDeskNodeType_NumericConstant,
    
    DataDeskDoc("An identifier node that represents a string constant in the code. The textual contents representing the intended string value are stored in the node's @NodeMember(string) member.")
        DataDeskNodeType_StringConstant,
    
    DataDeskDoc("An identifier node that represents a character constant in the code. The textual contents representing the intended character value are stored in the node's @NodeMember(string) member.")
        DataDeskNodeType_CharConstant,
    
    DataDeskDoc("A node that represents an operation that is performed on a single child node. The type of operation is stored in the @NodeMember(sub_type) member. The child node being operated on is pointed at by either the @NodeMember(operand) member, or the @NodeMember(children_list_head) member.")
        DataDeskNodeType_UnaryOperator,
    
    DataDeskDoc("A node that represents an operation that is performed on two children nodes. The type of operation is stored in the @NodeMember(sub_type) member. The children nodes being operated on are pointed at by either the @NodeMember(left) and @NodeMember(right) members, or the @NodeMember(children_list_head) and @NodeMember(children_list_tail) members.")
        DataDeskNodeType_BinaryOperator,
    
    DataDeskDoc("A node that represents a product-type that includes a set of declarations. The list of declarations is pointed at by the @NodeMember(children_list_head) member.")
        DataDeskNodeType_StructDeclaration,
    
    DataDeskDoc("A node that represents a sum-type that includes a set of declarations. The list of declarations is pointed at by the @NodeMember(children_list_head) member.")
        DataDeskNodeType_UnionDeclaration,
    
    DataDeskDoc("A node that represents a sum-type constant that includes a set of constants. The list of constants is pointed at by the @NodeMember(children_list_head) member.")
        DataDeskNodeType_EnumDeclaration,
    
    DataDeskDoc("A node that represents a product-type constant that includes a set of constants. The list of constants is pointed at by the @NodeMember(children_list_head) member.")
        DataDeskNodeType_FlagsDeclaration,
    
    DataDeskDoc("A node that binds some name (referenced by nodes of type @NodeType(Identifier)) to some expression. The expression is pointed at by the @NodeMember(children_list_head) member.")
        DataDeskNodeType_ConstantDefinition,
    
    DataDeskDoc("A node that represents the header of some procedure. The return type can be accessed through the @NodeMember(procedure_header.return_type) member. The list of parameters can be accessed through the @NodeMember(procedure_header.first_parameter) member.")
        DataDeskNodeType_ProcedureHeader,
    
    DataDeskDoc("A node that represents the declaration of some variable. This variable includes a name (referenced by @NodeType(Identifier) nodes) which is stored in the @NodeMember(string) member, a type expression that is pointed at by the @NodeMember(declaration.type) member, and an optional initialization.")
        DataDeskNodeType_Declaration,
    
    DataDeskDoc("A node that forms a type expression. The type of type-decorator that this node is, or how it alters the type it is helping to define, is stored in the @NodeMember(sub_type) member. This node stores arrays and pointers.")
        DataDeskNodeType_TypeDecorator,
    
    DataDeskDoc("A node that specifies a piece of metadata on a node. This node can take a set of parameters (pointed at by the @NodeMember(children_list_head) member) and can be used to document information that cannot be expressed through other node types.")
        DataDeskNodeType_Tag,
    
#if DATA_DESK_USE_OLD_NAMES
    DATA_DESK_NODE_TYPE_invalid = DataDeskNodeType_Invalid,
    DATA_DESK_NODE_TYPE_identifier,
    DATA_DESK_NODE_TYPE_numeric_constant,
    DATA_DESK_NODE_TYPE_string_constant,
    DATA_DESK_NODE_TYPE_char_constant,
    DATA_DESK_NODE_TYPE_unary_operator,
    DATA_DESK_NODE_TYPE_binary_operator,
    DATA_DESK_NODE_TYPE_struct_declaration,
    DATA_DESK_NODE_TYPE_union_declaration,
    DATA_DESK_NODE_TYPE_enum_declaration,
    DATA_DESK_NODE_TYPE_flags_declaration,
    DATA_DESK_NODE_TYPE_constant_definition,
    DATA_DESK_NODE_TYPE_procedure_header,
    DATA_DESK_NODE_TYPE_declaration,
    DATA_DESK_NODE_TYPE_type_decorator,
    DATA_DESK_NODE_TYPE_tag,
#endif
}
DataDeskNodeType;

// NOTE(rjf): Used to store sub-type of main node type (like the specific kind of binary
// operator for a DataDeskNodeType_BinaryOperator node).
typedef unsigned int DataDeskNodeSubType;

// NOTE(rjf): The unary operator precedence table in UnaryOperatorPrecedence
// must update to match this when this changes, and also the DataDeskGetUnaryOperatorString
// procedure in this file.
DataDeskDoc("Defines the sub-types of unary operators.")
typedef enum DataDeskUnaryOperatorType
{
    DataDeskDoc("Invalid operator type, produced by either invalid code or a bug in the parser.")
        DataDeskUnaryOperatorType_Invalid,
    DataDeskDoc("An alias for DataDeskUnaryOperatorType_Invalid.")
        DataDeskUnaryOperatorType_Null = DataDeskUnaryOperatorType_Invalid,
    DataDeskDoc("Encodes an arithmetic negation operator.")
        DataDeskUnaryOperatorType_Negative,
    DataDeskDoc("Encodes a boolean-wise NOT operator.")
        DataDeskUnaryOperatorType_Not,
    DataDeskDoc("Encodes a bitwise negation operator.")
        DataDeskUnaryOperatorType_BitwiseNegate,
    DataDeskUnaryOperatorType_Max,
    
#if DATA_DESK_USE_OLD_NAMES
    DATA_DESK_UNARY_OPERATOR_TYPE_invalid = DataDeskUnaryOperatorType_Invalid,
    DATA_DESK_UNARY_OPERATOR_TYPE_negative,
    DATA_DESK_UNARY_OPERATOR_TYPE_not,
    DATA_DESK_UNARY_OPERATOR_TYPE_bitwise_negate,
    DATA_DESK_UNARY_OPERATOR_TYPE_MAX,
#endif
}
DataDeskUnaryOperatorType;

// NOTE(rjf): The binary operator precedence table in BinaryOperatorPrecedence
// must update to match this when this changes, and also the DataDeskGetBinaryOperatorString
// procedure in this file.
DataDeskDoc("Defines the sub-types of binary operators.")
typedef enum DataDeskBinaryOperatorType
{
    DataDeskDoc("Invalid operator type, produced by either invalid code or a bug in the parser.")
        DataDeskBinaryOperatorType_Invalid,
    
    DataDeskDoc("An alias for DataDeskBinaryOperatorType_Invalid.")
        DataDeskBinaryOperatorType_Null = DataDeskBinaryOperatorType_Invalid,
    
    DataDeskDoc("Encodes an addition operator, which represents the sum of the two children.")
        DataDeskBinaryOperatorType_Add,
    
    DataDeskDoc("Encodes a subtraction operator, which represents the difference of the two children.")
        DataDeskBinaryOperatorType_Subtract,
    
    DataDeskDoc("Encodes a multiplication operator, which represents the product of the two children.")
        DataDeskBinaryOperatorType_Multiply,
    
    DataDeskDoc("Encodes a division operator, which represents the division of the left child by the right child.")
        DataDeskBinaryOperatorType_Divide,
    
    DataDeskDoc("Encodes a modulus operator, which represents the remainder of a division of the left child by the right child.")
        DataDeskBinaryOperatorType_Modulus,
    
    DataDeskDoc("Encodes a left bitshift operator.")
        DataDeskBinaryOperatorType_BitshiftLeft,
    
    DataDeskDoc("Encodes a right bitshift operator.")
        DataDeskBinaryOperatorType_BitshiftRight,
    
    DataDeskDoc("Encodes a bitwise AND operator.")
        DataDeskBinaryOperatorType_BitwiseAnd,
    
    DataDeskDoc("Encodes a bitwise OR operator.")
        DataDeskBinaryOperatorType_BitwiseOr,
    
    DataDeskDoc("Encodes a boolean AND operator.")
        DataDeskBinaryOperatorType_BooleanAnd,
    
    DataDeskDoc("Encodes a boolean OR operator.")
        DataDeskBinaryOperatorType_BooleanOr,
    
    DataDeskBinaryOperatorType_Max,
    
#if DATA_DESK_USE_OLD_NAMES
    DATA_DESK_BINARY_OPERATOR_TYPE_invalid = DataDeskBinaryOperatorType_Invalid,
    DATA_DESK_BINARY_OPERATOR_TYPE_add,
    DATA_DESK_BINARY_OPERATOR_TYPE_subtract,
    DATA_DESK_BINARY_OPERATOR_TYPE_multiply,
    DATA_DESK_BINARY_OPERATOR_TYPE_divide,
    DATA_DESK_BINARY_OPERATOR_TYPE_modulus,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_left,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitshift_right,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_and,
    DATA_DESK_BINARY_OPERATOR_TYPE_bitwise_or,
    DATA_DESK_BINARY_OPERATOR_TYPE_boolean_and,
    DATA_DESK_BINARY_OPERATOR_TYPE_boolean_or,
    DATA_DESK_BINARY_OPERATOR_TYPE_MAX,
#endif
}
DataDeskBinaryOperatorType;

DataDeskDoc("Defines the sub-types of type decorators.")
typedef enum DataDeskTypeDecoratorType
{
    DataDeskDoc("Encodes a pointer type-decorator. The rest of the type information will be encoded in the first child of a node with this type.")
        DataDeskTypeDecoratorType_Pointer,
    
    DataDeskDoc("Encodes an array type-decorator. The rest of the type information will be encoded in the first child of a node with this type. The expression that describes the array's size will be stored in the second child of a node with this type.")
        DataDeskTypeDecoratorType_Array,
}
DataDeskTypeDecoratorType;

DataDeskDoc("Defines the set of word styles that Data Desk uses for name transforms.")
typedef enum DataDeskWordStyle
{
    DataDeskDoc("Stylizes a word with the first letter capitalized, with the rest lowercase.")
        DataDeskWordStyle_Word,
    DataDeskWordStyle_CamelCase = DataDeskWordStyle_Word,
    
    DataDeskDoc("Stylizes a word with all letters capitalized.")
        DataDeskWordStyle_WORD,
    DataDeskWordStyle_AllCaps = DataDeskWordStyle_WORD,
    DataDeskWordStyle_UpperCase = DataDeskWordStyle_WORD,
    
    DataDeskDoc("Stylizes a word with all letters lowercase.")
        DataDeskWordStyle_word,
    DataDeskWordStyle_SnakeCase = DataDeskWordStyle_word,
    DataDeskWordStyle_LowerCase = DataDeskWordStyle_word,
    
    DataDeskWordStyle_Max,
}
DataDeskWordStyle;

DataDeskDoc("Defines the set of word separators that Data Desk uses for name transforms.")
typedef enum DataDeskWordSeparator
{
    DataDeskDoc("No separator at all. Used in styles like CamelCase.")
        DataDeskWordSeparator_None,
    DataDeskDoc("Underscore as a separator. Used in styles like snake_case.")
        DataDeskWordSeparator_Underscore,
    DataDeskDoc("Space as a separator. Used for human-readable string transforms.")
        DataDeskWordSeparator_Space,
    DataDeskWordSeparator_Max,
}
DataDeskWordSeparator;

DataDeskDoc("Defines the format for a single node inside of a Data Desk abstract-syntax-tree.")
struct DataDeskNode
{
    //~ NOTE(rjf): AST Relationship Data
    
    DataDeskDoc("Points at the next node in the graph, when in a list.")
        DataDeskNode *next;
    DataDeskDoc("Points at the previous node in the graph, when in a list.")
        DataDeskNode *prev;
    DataDeskDoc("Points at the parent node in the graph. This is 0 when the associated code is defined globally.")
        DataDeskNode *parent;
    DataDeskDoc("Determines the node type.")
        DataDeskNodeType type;
    DataDeskDoc("Determines the node sub-type, which form categories within node types.")
        DataDeskNodeSubType sub_type;
    
    //~ NOTE(rjf): String
    DataDeskDoc("Stores the length of the node's string, if applicable.")
        union
    {
        int string_length;
        int name_length;
    };
    
    DataDeskDoc("Stores a string for a given node, if one is appropriate.")
        union
    {
        char *string;
        char *name;
    };
    
    //~ NOTE(rjf): String Transforms
    
    // NOTE(rjf): DataDeskWordStyle_Max*DataDeskWordStyle_Max => First-word rule multiplied
    // by second+ word rule
    //
    // DataDeskWordSeparator_Max => All possible separators
    
    DataDeskDoc("Stores a table of transformed node strings. These can be accessed conveniently through the @Procedure(DataDeskGetTransformedString) procedure.")
        char *name_transforms[DataDeskWordStyle_Max][DataDeskWordStyle_Max][DataDeskWordSeparator_Max];
    
    //~ NOTE(rjf): File/Line Source Information
    DataDeskDoc("Stores a null-terminated string containing the filename from which this node was parsed.")
        char *file;
    DataDeskDoc("Stores an integer representing the line number from which this node was parsed.")
        int line;
    DataDeskDoc("Stores an integer representing the last line number from which this node was parsed.")
        int end_line;
    
    //~ NOTE(rjf): Tag List
    union
    {
        DataDeskNode *tag_list_head;
        DataDeskNode *tag_list;
        DataDeskNode *first_tag;
    };
    
    //~ NOTE(rjf): Children and node-specific children aliases
    DataDeskDoc("Stores pointers to a list of this node's children. Aliases are intended for specific node types and are just for convenience; the children_list_head and children_list_tail members are the generic, general-purpose names.")
        union
    {
        struct
        {
            DataDeskNode *children_list_head;
            DataDeskNode *children_list_tail;
        };
        
        struct
        {
            DataDeskNode *left;
            DataDeskNode *right;
        };
        
        struct
        {
            DataDeskNode *operand;
        };
        
        struct
        {
            DataDeskNode *reference;
        };
        
        struct
        {
            DataDeskNode *expression;
        };
    };
    
    //~ NOTE(rjf): Node-specific data shortcuts
    DataDeskDoc("Stores node-specific pointers to important node children for certain node types.")
        union
    {
        
        struct
        {
            DataDeskNode *return_type;
            DataDeskNode *first_parameter;
        }
        procedure_header;
        
        struct
        {
            DataDeskNode *type;
            DataDeskNode *initialization;
        }
        declaration;
        
        struct
        {
            DataDeskNode *first_tag_parameter;
        }
        tag;
        
    };
    
    // TODO(mal): Write DataDeskDoc
    int namespace_index;
    char *namespace_string;
    
    // TODO(mal): Write DataDeskDoc
    struct
    {
        DataDeskNode *namespace_alias_prev;
        DataDeskNode *namespace_alias_next;
    };
};


/*
| /////////////////////////////////////////////////////////////////
|  Introspection Helper Functions
| /////////////////////////////////////////////////////////////////
|
| The following function prototypes are convenience utility
| functions that can be helpful when introspecting upon abstract
| syntax trees.
*/

//~ NOTE(rjf): Helpers
DataDeskDoc("Returns 1 if @Parameter(0) is an alphabetic character.")
DATA_DESK_HEADER_PROC int DataDeskCharIsAlpha(int c);

DataDeskDoc("Returns 1 if @Parameter(0) is a numeric character.")
DATA_DESK_HEADER_PROC int DataDeskCharIsDigit(int c);

DataDeskDoc("Returns 1 if @Parameter(0) is a whitespace character.")
DATA_DESK_HEADER_PROC int DataDeskCharIsSpace(int c);


//~ NOTE(rjf): Name Introspection
DataDeskDoc("Returns 1 if @Parameter(0) contains the substring @Parameter(1).")
DATA_DESK_HEADER_PROC int DataDeskStringHasSubString(char *string, char *substring);
DataDeskDoc("Returns a text-transformed name of the passed node matching the passed word style and separator.")
DATA_DESK_HEADER_PROC char *DataDeskGetTransformedString(DataDeskDoc("The node to retrieve the string for.")
                                                         DataDeskNode *root,
                                                         
                                                         DataDeskDoc("The capitalization style applied to each word of the string.")
                                                         DataDeskWordStyle style,
                                                         
                                                         DataDeskDoc("The separator inserted between words in the string.")
                                                         DataDeskWordSeparator separator);

DataDeskDoc("Returns a text-transformed name of the passed node matching the passed word style and separator, with a different style for the first word.")
DATA_DESK_HEADER_PROC char *
DataDeskGetTransformedStringDifferentFirstWord(
                                               DataDeskDoc("The node to retrieve the string for.")
                                               DataDeskNode *root,
                                               
                                               DataDeskDoc("The capitalization style applied to only the first word of the string.")
                                               DataDeskWordStyle first_word_style,
                                               
                                               DataDeskDoc("The capitalization style applied to all but the first word of the string.")
                                               DataDeskWordStyle style,
                                               
                                               DataDeskDoc("The separator inserted between words in the string.")
                                               DataDeskWordSeparator separator);


//~ NOTE(rjf): Tag Introspection
DataDeskDoc("Searches through the tags of the passed node to find one that matches the passed string, and returns a pointer to it, or 0 if it was not found on the passed node.")
DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskGetNodeTag(DataDeskDoc("The node to check.")
                   DataDeskNode *root,
                   DataDeskDoc("A string containing the name associated with a tag, excluding the '@' character.")
                   char *tag);

DataDeskDoc("Given a tag node, finds the nth tag parameter (where n is @Parameter(1)) and returns a pointer to it.")
DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskGetTagParameter(DataDeskDoc("The tag node to find the parameter in.")
                        DataDeskNode *tag,
                        DataDeskDoc("The index of the parameter to find, with 0 being the first parameter.")
                        int parameter_number);

DataDeskDoc("Returns 1 if the passed node has a tag that matches the passed string, or 0 otherwise.")
DATA_DESK_HEADER_PROC int
DataDeskNodeHasTag(DataDeskDoc("The node to find the tag in.")
                   DataDeskNode *root,
                   DataDeskDoc("A string containing the name associated with a tag, excluding the '@' character.")
                   char *tag);

DataDeskDoc("Searches for a tag on @Parameter(0) that matches @Parameter(1), retrieves the tag parameter indexed at @Parameter(2), and returns a pointer to the tag argument in @Parameter(3). Returns nonzero if the tag and parameter were found, and 0 otherwise.")
DATA_DESK_HEADER_PROC int
DataDeskRequireTagParameter(DataDeskDoc("The node to find the tag parameters in.")
                            DataDeskNode *root,
                            
                            DataDeskDoc("A string containing the name associated with a tag, excluding the '@' character.")
                            char *tag,
                            
                            DataDeskDoc("The index of the parameter to retrieve.")
                            int index,
                            
                            DataDeskDoc("A pointer to a pointer that will store the result, if one is found.")
                            DataDeskNode **result);

DataDeskDoc("Searches for a tag on @Parameter(0) that matches @Parameter(1), retrieves the tag parameters until @Parameter(2) tag parameters are found, and returns a pointer to the tag arguments in @Parameter(3). Returns nonzero if the tag and parameters were found, and 0 otherwise.")
DATA_DESK_HEADER_PROC int
DataDeskRequireTagParameters(DataDeskDoc("The node to find the tag parameters in.")
                             DataDeskNode *root,
                             
                             DataDeskDoc("A string containing the name associated with a tag, excluding the '@' character.")
                             char *tag,
                             
                             DataDeskDoc("The number of parameters to extract from the tag.")
                             int count,
                             
                             DataDeskDoc("A pointer to a set of DataDeskNode *s that must be large enough to store the desired number of parameters.")
                             DataDeskNode **array);



//~ NOTE(rjf): Type Introspection

// NOTE(rjf): Takes a string representing a type in the form "*[]name" (with any
// number of *'s and []'s, in any order). Returns if the AST matches the form
// specified by the string.
DataDeskDoc("Determines if the type-evaluation of @Parameter(0) matches the form specified by the passed @Parameter(1) parameter, which is to be of similar form to the Data Desk grammar (e.g. \"*[]type\"), but without array size expressions. Returns nonzero if the types seem to match, and 0 otherwise.")
DATA_DESK_HEADER_PROC int
DataDeskMatchType(DataDeskDoc("The node to compare with the passed type string.")
                  DataDeskNode *root,
                  DataDeskDoc("The string representing some type expression.")
                  char *type);

DataDeskDoc("Evaluates the type of @Parameter(0). Returns \".\" if the type is not a pointer type, and \"->\" otherwise.")
DATA_DESK_HEADER_PROC char *
DataDeskGetTypeAccessString(DataDeskDoc("The node to evaluate the type for.") DataDeskNode *node);

DataDeskDoc("Finds the type-expression of @Parameter(0).")
DATA_DESK_HEADER_PROC DataDeskNode *DataDeskEvaluateType(DataDeskDoc("The node to evaluate the type for.") DataDeskNode *root);
DataDeskDoc("Finds the base type of the type-expression of @Parameter(0), which discounts any type decorators.")
DATA_DESK_HEADER_PROC DataDeskNode *DataDeskEvaluateBaseType(DataDeskDoc("The node to evaluate the base-type for.") DataDeskNode *root);

DataDeskDoc("Returns nonzero if @Parameter(0) is an array type, and 0 otherwise. If it is found that @Parameter(0) is an array type, the base type of the array is returned to @Parameter(1), and the size-expression of the array is returned to @Parameter(2).")
DATA_DESK_HEADER_PROC int
DataDeskIsArrayType(DataDeskDoc("The node to evaluate the type for.")
                    DataDeskNode *root,
                    DataDeskDoc("A pointer to a pointer which will store the base-type of the type expression, if the type is found to be an array type.")
                    DataDeskNode **base_type,
                    DataDeskDoc("A pointer to a pointer which will store the array size expression, if the type is found to be an array type.")
                    DataDeskNode **array_size_expression);

DataDeskDoc("Returns nonzero if @Parameter(0) is a pointer type, and 0 otherwise. If it is found that @Parameter(0) is a pointer type, the base type of the array is returned to @Parameter(1).")
DATA_DESK_HEADER_PROC int
DataDeskIsPointerType(DataDeskDoc("The node to evaluate the type for.")
                      DataDeskNode *root,
                      DataDeskDoc("A pointer to a pointer which will store the base-type of the type expression, if the type is found to be a pointer type.")
                      DataDeskNode **base_type);


//~ NOTE(rjf): Interpretation
DataDeskDoc("Tries to interpret and evaluate the tree pointed to by @Parameter(0) as an integer.")
DATA_DESK_HEADER_PROC int
DataDeskInterpretNumericExpressionAsInteger(DataDeskDoc("The root node of the expression to evaluate.")
                                            DataDeskNode *root);


//~ NOTE(rjf): Output
DataDeskDoc("Returns the string that encodes the passed binary operator type.")
DATA_DESK_HEADER_PROC char *
DataDeskGetBinaryOperatorString(DataDeskDoc("The binary operator type to retrieve the string for.")
                                DataDeskBinaryOperatorType type);

DataDeskDoc("Returns the string that encodes the passed unary operator type.")
DATA_DESK_HEADER_PROC char *
DataDeskGetUnaryOperatorString(DataDeskDoc("The unary operator type to retrieve the string for.")
                               DataDeskUnaryOperatorType type);

DataDeskDoc("Prints out an error message that is attached to @Parameter(0).")
DATA_DESK_HEADER_PROC void
DataDeskError(DataDeskDoc("The node that the error is associated with.")
              DataDeskNode *node,
              DataDeskDoc("A format string, used to determine the extra arguments passed, and generate the final error string (printf-style).")
              char *format,
              ...);

DataDeskDoc("Prints out a warning message that is attached to @Parameter(0).")
DATA_DESK_HEADER_PROC void DataDeskWarning(DataDeskDoc("The node that the warning is associated with.")
                                           DataDeskNode *node,
                                           DataDeskDoc("A format string, used to determine the extra arguments passed, and generate the final warning string (printf-style).")
                                           char *format, ...);


//~ NOTE(rjf): CRT Generation Functions
DataDeskDoc("Prints out the graph pointed to by @Parameter(1) to the file-stream pointed to by @Parameter(0).")
DATA_DESK_HEADER_PROC void DataDeskFWriteGraphAsC(DataDeskDoc("The FILE * to write the output to.")
                                                  FILE *file,
                                                  DataDeskDoc("The root of the abstract-syntax-tree to write to the output stream.")
                                                  DataDeskNode *root);

DataDeskDoc("Prints out the @Parameter(1) string to the file-stream pointed to by @Parameter(0), but trying to do so as a string with words separated by spaces.")
DATA_DESK_HEADER_PROC void DataDeskFWriteStringWithSpaces(DataDeskDoc("The FILE * to write the output to.")
                                                          FILE *file,
                                                          DataDeskDoc("The string to write.")
                                                          char *string);

DataDeskDoc("Prints out the @Parameter(1) string to the file-stream pointed to by @Parameter(0), but trying to do so as a string with lowercase words separated by underscores (snake_case).")
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsLowercaseWithUnderscores(DataDeskDoc("The FILE * to write the output to.")
                                                                          FILE *file,
                                                                          DataDeskDoc("The string to write.")
                                                                          char *string);

DataDeskDoc("Prints out the @Parameter(1) string to the file-stream pointed to by @Parameter(0), but trying to do so as a string with uppercase words separated by underscores (UPPER_SNAKE_CASE).")
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsUppercaseWithUnderscores(DataDeskDoc("The FILE * to write the output to.")
                                                                          FILE *file,
                                                                          DataDeskDoc("The string to write.")
                                                                          char *string);

DataDeskDoc("Prints out the @Parameter(1) string to the file-stream pointed to by @Parameter(0), but trying to do so as a string with uppercase words separated by nothing (UpperCamelCase).")
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsUpperCamelCase(DataDeskDoc("The FILE * to write the output to.")
                                                                FILE *file,
                                                                DataDeskDoc("The string to write.")
                                                                char *string);

DataDeskDoc("Prints out the @Parameter(1) string to the file-stream pointed to by @Parameter(0), but trying to do so as a string with uppercase words separated by nothing, with a lowercase first word (lowerCamelCase).")
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsLowerCamelCase(DataDeskDoc("The FILE * to write the output to.")
                                                                FILE *file,
                                                                DataDeskDoc("The string to write.")
                                                                char *string);

DATA_DESK_HEADER_PROC void DataDeskFWriteStringWithSpacesN(FILE *file, char *string, int string_length);
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsLowercaseWithUnderscoresN(FILE *file, char *string, int string_length);
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsUppercaseWithUnderscoresN(FILE *file, char *string, int string_length);
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsUpperCamelCaseN(FILE *file, char *string, int string_length);
DATA_DESK_HEADER_PROC void DataDeskFWriteStringAsLowerCamelCaseN(FILE *file, char *string, int string_length);





/*
| /////////////////////////////////////////////////////////////////
|  Helper Function Implementation Code
| /////////////////////////////////////////////////////////////////
*/

DATA_DESK_HEADER_PROC int
DataDeskCharIsAlpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

DATA_DESK_HEADER_PROC int
DataDeskCharIsDigit(int c)
{
    return (c >= '0' && c <= '9');
}

DATA_DESK_HEADER_PROC int
DataDeskCharIsLowercaseAlpha(int c)
{
    return (c >= 'a' && c <= 'z');
}

DATA_DESK_HEADER_PROC int
DataDeskCharIsUppercaseAlpha(int c)
{
    return (c >= 'A' && c <= 'Z');
}

DATA_DESK_HEADER_PROC int
DataDeskCharIsSpace(int c)
{
    return (c <= 32 && c);
}

DATA_DESK_HEADER_PROC int
DataDeskCharToLower(int c)
{
    return (DataDeskCharIsUppercaseAlpha(c) ? c + 32 : c);
}

DATA_DESK_HEADER_PROC int
DataDeskCharToUpper(int c)
{
    return (DataDeskCharIsLowercaseAlpha(c) ? c - 32 : c);
}

DATA_DESK_HEADER_PROC int
DataDeskStringMatch(char *a, char *b)
{
    int matches = 0;
    if(a && b)
    {
        matches = 1;
        for(int i = 0;; i += 1)
        {
            if(a[i] != b[i])
            {
                matches = 0;
                break;
            }
            if(a[i] == 0 || b[i] == 0)
            {
                break;
            }
        }
    }
    return matches;
}

DATA_DESK_HEADER_PROC int
DataDeskStringHasAlphanumericBlock(char *string, char *substring)
{
    int matches = 0;
    
    if(string && substring)
    {
        for(int i = 0; string[i]; ++i)
        {
            if(string[i] == substring[0])
            {
                matches = 1;
                
                int string_i = i+1;
                int substring_i = 1;
                for(;;)
                {
                    if(!substring[substring_i])
                    {
                        if(substring_i > 0 &&
                           (DataDeskCharIsAlpha(substring[substring_i-1]) ||
                            DataDeskCharIsDigit(substring[substring_i-1]) ||
                            substring[substring_i-1] == '_'))
                        {
                            if(DataDeskCharIsAlpha(string[string_i]) ||
                               DataDeskCharIsDigit(string[string_i]) ||
                               string[string_i] == '_')
                            {
                                matches = 0;
                            }
                        }
                        break;
                    }
                    
                    if(string[string_i] != substring[substring_i])
                    {
                        matches = 0;
                        break;
                    }
                    
                    ++string_i;
                    ++substring_i;
                }
            }
            
            if(matches)
            {
                break;
            }
        }
    }
    
    return matches;
}

DATA_DESK_HEADER_PROC int
DataDeskStringHasSubString(char *string, char *substring)
{
    return DataDeskStringHasAlphanumericBlock(string, substring);
}

DATA_DESK_HEADER_PROC char *
DataDeskGetTransformedString(DataDeskNode *root, DataDeskWordStyle style, DataDeskWordSeparator separator)
{
    return DataDeskGetTransformedStringDifferentFirstWord(root, style, style, separator);
}

DATA_DESK_HEADER_PROC char *
DataDeskGetTransformedStringDifferentFirstWord(DataDeskNode *root,
                                               DataDeskWordStyle first_word_style,
                                               DataDeskWordStyle style, DataDeskWordSeparator separator)
{
    char *result = 0;
    if(root)
    {
        result = root->name_transforms[style][first_word_style][separator];
    }
    return result;
}

DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskGetNodeTag(DataDeskNode *root, char *tag)
{
    DataDeskNode *found_tag_node = 0;
    for(DataDeskNode *tag_node = root->first_tag;
        tag_node; tag_node = tag_node->next)
    {
        if(DataDeskStringMatch(tag_node->string + 1, tag))
        {
            found_tag_node = tag_node;
            break;
        }
    }
    return found_tag_node;
}

DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskGetTagParameter(DataDeskNode *tag, int parameter_number)
{
    DataDeskNode *result = 0;
    if(tag && tag->type == DataDeskNodeType_Tag)
    {
        int i = 0;
        for(DataDeskNode *parameter = tag->children_list_head;
            parameter; parameter = parameter->next)
        {
            if(i == parameter_number)
            {
                result = parameter;
                break;
            }
            ++i;
        }
    }
    return result;
}

DATA_DESK_HEADER_PROC int
DataDeskNodeHasTag(DataDeskNode *node, char *tag)
{
    DataDeskNode *tag_node = DataDeskGetNodeTag(node, tag);
    return tag_node != 0;
}

DATA_DESK_HEADER_PROC int
DataDeskRequireTagParameter(DataDeskNode *root, char *tag, int index, DataDeskNode **result)
{
    int found = 0;
    
    DataDeskNode *tag_node = DataDeskGetNodeTag(root, tag);
    if(tag_node)
    {
        DataDeskNode *tag_param = DataDeskGetTagParameter(tag_node, index);
        if(tag_param)
        {
            if(result)
            {
                *result = tag_param;
            }
            found = 1;
        }
    }
    
    return found;
}

DATA_DESK_HEADER_PROC int
DataDeskRequireTagParameters(DataDeskNode *root, char *tag, int count, DataDeskNode **array)
{
    int found = 1;
    for(int i = 0; i < count; ++i)
    {
        found &= DataDeskRequireTagParameter(root, tag, i, array + i);
    }
    return found;
}

DATA_DESK_HEADER_PROC int
DataDeskTypeMatchesString(DataDeskNode *type, char *type_string)
{
    int matches = 0;
    
    DataDeskNode *decorator = type;
    DataDeskNode *type_identifier = 0;
    char *type_string_identifier = 0;
    
    if(decorator->type == DataDeskNodeType_Identifier)
    {
        matches = 1;
        type_string_identifier = type_string;
        type_identifier = decorator;
    }
    else if(decorator->type == DataDeskNodeType_TypeDecorator)
    {
        matches = 1;
        for(int i = 0; type_string[i]; i += 1)
        {
            while(type_string[i] <= 32 && type_string[i])
            {
                i += 1;
            }
            
            if(type_string[i] == '*')
            {
                if(decorator->sub_type != DataDeskTypeDecoratorType_Pointer)
                {
                    matches = 0;
                    break;
                }
                else
                {
                    i += 1;
                    decorator = decorator->children_list_head;
                }
            }
            else if(type_string[i] == '[' && type_string[i+1] == ']')
            {
                if(decorator->sub_type != DataDeskTypeDecoratorType_Array)
                {
                    matches = 0;
                    break;
                }
                else
                {
                    i += 1;
                    decorator = decorator->children_list_head;
                }
            }
            else
            {
                if(decorator->type == DataDeskNodeType_Identifier)
                {
                    type_identifier = decorator;
                    type_string_identifier = type_string + i;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    if(matches)
    {
        if(!type_identifier || !type_string_identifier ||
           !DataDeskStringHasSubString(type_identifier->string, type_string_identifier))
        {
            matches = 0;
        }
    }
    
    return matches;
}

DATA_DESK_HEADER_PROC int
DataDeskMatchType(DataDeskNode *root, char *type)
{
    int matches = 0;
    DataDeskNode *type_node = DataDeskEvaluateType(root);
    matches = DataDeskTypeMatchesString(type_node, type);
    return matches;
}

DATA_DESK_HEADER_PROC int
DataDeskCStringToInt(char *string)
{
    int value = 0;
    
    char value_str[64] = {0};
    int value_str_write_pos = 0;
    for(int i = 0; string[i]; ++i)
    {
        if(DataDeskCharIsDigit(string[i]))
        {
            for(int j = i; string[j]; ++j)
            {
                if(DataDeskCharIsDigit(string[j]))
                {
                    value_str[value_str_write_pos++] = string[j];
                }
                else
                {
                    break;
                }
            }
            
            int number_of_digits = value_str_write_pos;
            int multiplier = 1;
            for(int j = 0; j < number_of_digits; ++j)
            {
                value += (value_str[number_of_digits - j - 1] - '0') * multiplier;
                multiplier *= 10;
            }
            
            break;
        }
    }
    
    return value;
}

DATA_DESK_HEADER_PROC int
DataDeskInterpretNumericExpressionAsInteger(DataDeskNode *root)
{
    int result = 0;
    if(root)
    {
        switch(root->type)
        {
            case DataDeskNodeType_NumericConstant:
            {
                result = DataDeskCStringToInt(root->string);
                break;
            }
            case DataDeskNodeType_Identifier:
            {
                result = DataDeskInterpretNumericExpressionAsInteger(root->children_list_head);
                break;
            }
            case DataDeskNodeType_ConstantDefinition:
            {
                result = DataDeskInterpretNumericExpressionAsInteger(root->children_list_head);
                break;
            }
            case DataDeskNodeType_UnaryOperator:
            {
                DataDeskUnaryOperatorType unary_operator_type = root->sub_type;
                int operand = DataDeskInterpretNumericExpressionAsInteger(root->operand);
                switch(unary_operator_type)
                {
                    case DataDeskUnaryOperatorType_Not:
                    {
                        result = !operand;
                        break;
                    }
                    case DataDeskUnaryOperatorType_Negative:
                    {
                        result = -operand;
                        break;
                    }
                    case DataDeskUnaryOperatorType_BitwiseNegate:
                    {
                        result = ~operand;
                        break;
                    }
                    default: break;
                }
                break;
            }
            
            case DataDeskNodeType_BinaryOperator:
            {
                DataDeskBinaryOperatorType binary_operator_type = root->sub_type;
                int left_tree = DataDeskInterpretNumericExpressionAsInteger(root->left);
                int right_tree = DataDeskInterpretNumericExpressionAsInteger(root->right);
                
                switch(binary_operator_type)
                {
                    case DataDeskBinaryOperatorType_Add:            { result = left_tree + right_tree; break; }
                    case DataDeskBinaryOperatorType_Subtract:       { result = left_tree - right_tree; break; }
                    case DataDeskBinaryOperatorType_Multiply:       { result = left_tree * right_tree; break; }
                    case DataDeskBinaryOperatorType_Divide:         { result = (right_tree != 0) ? (left_tree / right_tree) : 0; break; }
                    case DataDeskBinaryOperatorType_Modulus:        { result = (right_tree != 0) ? (left_tree % right_tree) : 0; break; }
                    case DataDeskBinaryOperatorType_BitshiftLeft:   { result = left_tree << right_tree; break; }
                    case DataDeskBinaryOperatorType_BitshiftRight:  { result = left_tree >> right_tree; break; }
                    default: break;
                }
                
                break;
            }
            
            default: break;
        }
    }
    return result;
}

DATA_DESK_HEADER_PROC char *
DataDeskGetBinaryOperatorString(DataDeskBinaryOperatorType type)
{
    char *strings[] =
    {
        "(invalid)",
        "+",
        "-",
        "*",
        "/",
        "%",
        "<<",
        ">>",
        "&",
        "|",
        "&&",
        "||",
    };
    return strings[type];
}

DATA_DESK_HEADER_PROC char *
DataDeskGetUnaryOperatorString(DataDeskUnaryOperatorType type)
{
    char *strings[] =
    {
        "(invalid)",
        "-",
        "!",
        "~",
    };
    return strings[type];
}

DATA_DESK_HEADER_PROC void
DataDeskError(DataDeskNode *node, char *format, ...)
{
    char *filename = "";
    int line = 0;
    
    if(node)
    {
        filename = node->file;
        line = node->line;
    }
    
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR (%s:%i): ", filename, line);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

DATA_DESK_HEADER_PROC void
DataDeskWarning(DataDeskNode *node, char *format, ...)
{
    char *filename = "";
    int line = 0;
    
    if(node)
    {
        filename = node->file;
        line = node->line;
    }
    
    va_list args;
    va_start(args, format);
    fprintf(stderr, "WARNING (%s:%i): ", filename, line);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

DATA_DESK_HEADER_PROC int
DataDeskGetIndirectionCountForType(DataDeskNode *node)
{
    int indirection_count = 0;
    for(DataDeskNode *type = node; type; type = type->children_list_head)
    {
        if(type->type == DataDeskNodeType_TypeDecorator &&
           type->sub_type == DataDeskTypeDecoratorType_Pointer)
        {
            indirection_count += 1;
        }
        else
        {
            break;
        }
    }
    return indirection_count;
}

DATA_DESK_HEADER_PROC char *
DataDeskGetAccessStringForType(DataDeskNode *node)
{
    if(DataDeskGetIndirectionCountForType(node) == 1)
    {
        return "->";
    }
    return ".";
}

DATA_DESK_HEADER_PROC char *
DataDeskGetAccessStringForDeclaration(DataDeskNode *node)
{
    return DataDeskGetAccessStringForType(node->declaration.type);
}

DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskEvaluateType(DataDeskNode *root)
{
    DataDeskNode *type = 0;
    
    for(;root;)
    {
        if(root->type == DataDeskNodeType_TypeDecorator)
        {
            type = root;
            break;
        }
        else if(root->type == DataDeskNodeType_Identifier)
        {
            DataDeskNode *referenced = root->reference;
            if(referenced == 0)
            {
                type = root;
                break;
            }
            else
            {
                if(referenced->type == DataDeskNodeType_Declaration)
                {
                    root = referenced;
                }
                else
                {
                    type = root;
                    break;
                }
            }
        }
        else if(root->type == DataDeskNodeType_Declaration)
        {
            type = root->declaration.type;
            break;
        }
    }
    
    return type;
}

DATA_DESK_HEADER_PROC DataDeskNode *
DataDeskEvaluateBaseType(DataDeskNode *root)
{
    DataDeskNode *base_type = 0;
    DataDeskNode *type = DataDeskEvaluateType(root);
    for(DataDeskNode *type_piece = type; type_piece; type_piece = type_piece->children_list_head)
    {
        if(type_piece->type == DataDeskNodeType_Identifier ||
           type_piece->type == DataDeskNodeType_StructDeclaration ||
           type_piece->type == DataDeskNodeType_EnumDeclaration ||
           type_piece->type == DataDeskNodeType_FlagsDeclaration)
        {
            base_type = type_piece;
            break;
        }
    }
    return base_type;
}

DATA_DESK_HEADER_PROC int
DataDeskIsArrayType(DataDeskNode *root, DataDeskNode **base_type_ptr, DataDeskNode **array_size_expression_ptr)
{
    int result = 0;
    
    DataDeskNode *type = DataDeskEvaluateType(root);
    DataDeskNode *base_type = DataDeskEvaluateBaseType(type);
    DataDeskNode *last_array_type_decorator = 0;
    
    for(DataDeskNode *type_piece = type; type_piece; type_piece = type_piece->children_list_head)
    {
        if(type_piece->type == DataDeskNodeType_TypeDecorator &&
           type_piece->sub_type == DataDeskTypeDecoratorType_Array)
        {
            if(last_array_type_decorator == 0)
            {
                last_array_type_decorator = type_piece;
                result = 1;
            }
            else
            {
                result = 0;
                break;
            }
        }
    }
    
    if(result && last_array_type_decorator)
    {
        DataDeskNode *array_size_expression = last_array_type_decorator->children_list_head ? last_array_type_decorator->children_list_head->next : 0;
        if(array_size_expression_ptr)
        {
            *array_size_expression_ptr = array_size_expression;
        }
        if(base_type_ptr)
        {
            *base_type_ptr = base_type;
        }
    }
    
    return result;
}

DATA_DESK_HEADER_PROC int
DataDeskIsPointerType(DataDeskNode *root, DataDeskNode **base_type_ptr)
{
    int result = 0;
    
    DataDeskNode *type = DataDeskEvaluateType(root);
    DataDeskNode *base_type = DataDeskEvaluateBaseType(type);
    DataDeskNode *last_pointer_type_decorator = 0;
    
    for(DataDeskNode *type_piece = type; type_piece; type_piece = type_piece->children_list_head)
    {
        if(type_piece->type == DataDeskNodeType_TypeDecorator &&
           type_piece->sub_type == DataDeskTypeDecoratorType_Pointer)
        {
            if(last_pointer_type_decorator == 0)
            {
                last_pointer_type_decorator = type_piece;
                result = 1;
            }
            else
            {
                result = 0;
                break;
            }
        }
    }
    
    if(result)
    {
        if(base_type_ptr)
        {
            *base_type_ptr = base_type;
        }
    }
    
    return result;
}

typedef struct DataDeskCPrintContext DataDeskCPrintContext;
struct DataDeskCPrintContext
{
    int nest;
    int indent;
    int indented_this_line;
    int lines_to_print;
};

DATA_DESK_HEADER_PROC void
_DataDeskCPrintPushIndent(DataDeskCPrintContext *context)
{
    context->indent += 1;
}

DATA_DESK_HEADER_PROC void
_DataDeskCPrintPopIndent(DataDeskCPrintContext *context)
{
    context->indent -= 1;
}

DATA_DESK_HEADER_PROC void
_DataDeskFWriteC(FILE *file, DataDeskCPrintContext *context, char *format, ...)
{
    if(context->indented_this_line == 0)
    {
        context->indented_this_line = 1;
        for(int i = 0; i < context->indent; ++i)
        {
            fprintf(file, "    ");
        }
    }
    
    for(int i = 0; format[i]; ++i)
    {
        if(format[i] == '\n')
        {
            context->indented_this_line = 0;
        }
    }
    
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
}

DATA_DESK_HEADER_PROC void
_DataDeskFWriteLines(FILE *file, DataDeskCPrintContext *context)
{
    for(int i = 0; i < context->lines_to_print; i += 1)
    {
        fprintf(file, "\n");
    }
    context->lines_to_print = 0;
}

DATA_DESK_HEADER_PROC void
_DataDeskCBumpLines(DataDeskCPrintContext *context, DataDeskNode *node)
{
    if(node && node->next)
    {
        int lines = (node->next->line - node->end_line);
        if(lines > 1)
        {
            context->lines_to_print += lines - 1;
        }
    }
}

DATA_DESK_HEADER_PROC void
_DataDeskFWriteGraphAsC(FILE *file, DataDeskNode *root, DataDeskCPrintContext *context)
{
    
#define _DDCScope for(int _i_ = (_DataDeskFWriteC(file, context, "{\n"), _DataDeskCPrintPushIndent(context), 0); !_i_; ++_i_, _DataDeskCPrintPopIndent(context), _DataDeskFWriteC(file, context, "}"))
    
    if(root)
    {
        _DataDeskFWriteLines(file, context);
        
        if(root->first_tag && root->first_tag->type == DataDeskNodeType_Tag)
        {
            _DataDeskFWriteC(file, context, "// ");
            for(DataDeskNode *tag = root->first_tag; tag; tag = tag->next)
            {
                _DataDeskFWriteC(file, context, "%.*s", tag->string_length, tag->string);
                
                if(tag->children_list_head)
                {
                    _DataDeskFWriteC(file, context, "(");
                    for(DataDeskNode *tag_arg = tag->children_list_head; tag_arg; tag_arg = tag_arg->next)
                    {
                        _DataDeskFWriteGraphAsC(file, tag_arg, context);
                        context->lines_to_print = 0;
                        if(tag_arg->next)
                        {
                            _DataDeskFWriteC(file, context, ", ");
                        }
                    }
                    _DataDeskFWriteC(file, context, ")");
                }
                if (tag->next)
                {
                    _DataDeskFWriteC(file, context, " ");
                }
            }
            _DataDeskFWriteC(file, context, "\n");
        }
        
        char *namespace_prefix = "";
        char *underscore = "";
        if(root->namespace_string)
        {
            namespace_prefix = root->namespace_string;
            underscore = "_";
        }
        
        switch(root->type)
        {
            case DataDeskNodeType_Identifier:
            {
                _DataDeskFWriteC(file, context, "%s%s", namespace_prefix, underscore);
                // NOTE(mal): Falls through  <-------------------------------- IMPORTANT
            }
            case DataDeskNodeType_NumericConstant:
            case DataDeskNodeType_CharConstant:
            {
                _DataDeskFWriteC(file, context, "%.*s", root->string_length, root->string);
                
                if(root->type == DataDeskNodeType_Identifier &&
                   root->parent &&
                   ((root->parent->type == DataDeskNodeType_TypeDecorator &&
                     (root->parent->sub_type != DataDeskTypeDecoratorType_Array ||
                      root->parent->children_list_head == root)) ||
                    root->parent->type == DataDeskNodeType_Declaration ||
                    (root->parent->type == DataDeskNodeType_ProcedureHeader &&
                     root->parent->children_list_head == root)
                    ))
                {
                    _DataDeskFWriteC(file, context, " ");
                }
                
                break;
            }
            
            case DataDeskNodeType_StringConstant:
            {
                if(root->string && root->string[0] == '"')
                {
                    if(root->string[1] == '"' && root->string[2] == '"')
                    {
                        char *string = root->string;
                        fprintf(file, "\"");
                        for(int i = 3; string[i]; ++i)
                        {
                            if(string[i] == '"' && string[i+1] == '"' && string[i+2] == '"')
                            {
                                break;
                            }
                            else if(string[i] == '\n')
                            {
                                fprintf(file, "\\n\"\n\"");
                            }
                            else if(string[i] == '\r')
                            {
                                fprintf(file, "\\r");
                            }
                            else if(string[i] == '\t')
                            {
                                fprintf(file, "\\t");
                            }
                            else
                            {
                                fprintf(file, "%c", string[i]);
                            }
                        }
                        fprintf(file, "\"");
                    }
                    else
                    {
                        _DataDeskFWriteC(file, context, "%.*s", root->string_length, root->string);
                    }
                }
                else
                {
                    _DataDeskFWriteC(file, context, "\"\"");
                }
                break;
            }
            
            case DataDeskNodeType_UnaryOperator:
            {
                _DataDeskFWriteC(file, context, "(");
                char *unary_operator_string = DataDeskGetUnaryOperatorString(root->sub_type);
                _DataDeskFWriteC(file, context, "%s", unary_operator_string);
                _DataDeskFWriteC(file, context, "(");
                _DataDeskFWriteGraphAsC(file, root->operand, context);
                _DataDeskFWriteC(file, context, ")");
                _DataDeskFWriteC(file, context, ")");
                break;
            }
            
            case DataDeskNodeType_BinaryOperator:
            {
                _DataDeskFWriteC(file, context, "(");
                _DataDeskFWriteGraphAsC(file, root->left, context);
                char *binary_operator_string = DataDeskGetBinaryOperatorString(root->sub_type);
                _DataDeskFWriteC(file, context, "%s", binary_operator_string);
                _DataDeskFWriteGraphAsC(file, root->right, context);
                _DataDeskFWriteC(file, context, ")");
                
                break;
            }
            
            case DataDeskNodeType_StructDeclaration:
            {
                if(!root->parent || root->parent->type != DataDeskNodeType_Declaration)
                {
                    _DataDeskFWriteC(file, context, "typedef struct %s%s%.*s %s%s%.*s;\n",
                                     namespace_prefix, underscore,
                                     root->string_length, root->string,
                                     namespace_prefix, underscore,
                                     root->string_length, root->string);
                }
                
                if(root->string)
                {
                    _DataDeskFWriteC(file, context, "struct %s%s%.*s\n",
                                     namespace_prefix, underscore,
                                     root->string_length, root->string);
                }
                else
                {
                    _DataDeskFWriteC(file, context, "struct\n");
                }
                
                _DDCScope
                {
                    for(DataDeskNode *member = root->children_list_head;
                        member; member = member->next)
                    {
                        _DataDeskFWriteGraphAsC(file, member, context);
                        _DataDeskFWriteC(file, context, ";\n");
                    }
                }
                
                if(!root->parent || root->parent->type != DataDeskNodeType_Declaration)
                {
                    _DataDeskFWriteC(file, context, ";\n");
                }
                
                break;
            }
            
            case DataDeskNodeType_UnionDeclaration:
            {
                if(!root->parent || root->parent->type != DataDeskNodeType_Declaration)
                {
                    _DataDeskFWriteC(file, context, "typedef union %s%s%.*s %s%s%.*s;\n",
                                     namespace_prefix, underscore,
                                     root->string_length, root->string,
                                     namespace_prefix, underscore,
                                     root->string_length, root->string);
                }
                
                if(root->string)
                {
                    _DataDeskFWriteC(file, context, "union %s%s%.*s\n", 
                                     namespace_prefix, underscore,
                                     root->string_length, root->string);
                }
                else
                {
                    _DataDeskFWriteC(file, context, "union\n");
                }
                
                _DDCScope
                {
                    for(DataDeskNode *member = root->children_list_head;
                        member; member = member->next)
                    {
                        _DataDeskFWriteGraphAsC(file, member, context);
                        _DataDeskFWriteC(file, context, ";\n");
                    }
                }
                
                if(!root->parent || root->parent->type != DataDeskNodeType_Declaration)
                {
                    _DataDeskFWriteC(file, context, ";\n");
                }
                
                break;
            }
            
            case DataDeskNodeType_EnumDeclaration:
            {
                _DataDeskFWriteC(file, context, "typedef enum %s%s%.*s\n", 
                                 namespace_prefix, underscore,
                                 root->string_length, root->string);
                _DDCScope
                {
                    for(DataDeskNode *member = root->children_list_head; member; member = member->next)
                    {
                        _DataDeskFWriteGraphAsC(file, member, context);
                        _DataDeskFWriteC(file, context, ",\n");
                    }
                }
                _DataDeskFWriteC(file, context, "\n%s%s%.*s;\n", namespace_prefix, underscore, root->string_length, root->string);
                break;
            }
            
            case DataDeskNodeType_FlagsDeclaration:
            {
                int needed_bits_for_flag_type = 32;
                int current_bit = 0;
                
                _DataDeskFWriteC(file, context, "enum\n");
                _DDCScope
                {
                    for(DataDeskNode *member = root->children_list_head; member; member = member->next)
                    {
                        _DataDeskFWriteC(file, context, "%s%s%.*s = (1<<%i),\n",
                                         namespace_prefix, underscore,
                                         member->string_length, member->string, current_bit);
                        ++current_bit;
                    }
                }
                _DataDeskFWriteC(file, context, ";\n");
                
                if(current_bit >= 31)
                {
                    needed_bits_for_flag_type = 64;
                }
                
                if(root->string)
                {
                    if(needed_bits_for_flag_type == 32)
                    {
                        _DataDeskFWriteC(file, context, "typedef unsigned int %s%s%.*s;\n", 
                                         namespace_prefix, underscore,
                                         root->string_length, root->string);
                    }
                    else if(needed_bits_for_flag_type > 32)
                    {
                        _DataDeskFWriteC(file, context, "typedef unsigned long int %s%s%.*s;\n", 
                                         namespace_prefix, underscore,
                                         root->string_length, root->string);
                    }
                }
                
                break;
            }
            
            case DataDeskNodeType_Declaration:
            {
                _DataDeskFWriteGraphAsC(file, root->children_list_head, context);
                _DataDeskFWriteC(file, context, "%.*s", root->string_length, root->string);
                
                for(DataDeskNode *type_decorator = root->children_list_head;
                    type_decorator && type_decorator->type == DataDeskNodeType_TypeDecorator;
                    type_decorator = type_decorator->children_list_head)
                {
                    if(type_decorator->sub_type == DataDeskTypeDecoratorType_Array)
                    {
                        DataDeskNode *array_size = type_decorator->children_list_head ? type_decorator->children_list_head->next : 0;
                        if(array_size)
                        {
                            if (type_decorator->parent &&
                                type_decorator->parent->type == DataDeskNodeType_TypeDecorator &&
                                type_decorator->parent->sub_type == DataDeskTypeDecoratorType_Pointer)
                            {
                                _DataDeskFWriteC(file, context, ")");
                            }
                            _DataDeskFWriteC(file, context, "[");
                            context->lines_to_print = 0;
                            _DataDeskFWriteGraphAsC(file, array_size, context);
                            _DataDeskFWriteC(file, context, "]");
                        }
                    }
                }
                
                break;
            }
            
            case DataDeskNodeType_TypeDecorator:
            {
                _DataDeskFWriteGraphAsC(file, root->children_list_head, context);
                if(root->sub_type == DataDeskTypeDecoratorType_Pointer)
                {
                    if (root->children_list_head &&
                        root->children_list_head->type == DataDeskNodeType_TypeDecorator &&
                        root->children_list_head->sub_type == DataDeskTypeDecoratorType_Array)
                    {
                        _DataDeskFWriteC(file, context, "(");
                    }
                    _DataDeskFWriteC(file, context, "*");
                }
                break;
            }
            
            case DataDeskNodeType_ConstantDefinition:
            {
                if(root->parent && root->parent->type == DataDeskNodeType_EnumDeclaration)
                {
                    _DataDeskFWriteC(file, context, "%s%s%.*s", namespace_prefix, underscore, root->string_length, root->string);
                    if(root->children_list_head)
                    {
                        _DataDeskFWriteC(file, context, " = ");
                        _DataDeskFWriteGraphAsC(file, root->children_list_head, context);
                    }
                }
                else
                {
                    _DataDeskFWriteC(file, context, "#define %s%s%.*s (", namespace_prefix, underscore, root->string_length, root->string);
                    _DataDeskFWriteGraphAsC(file, root->children_list_head, context);
                    _DataDeskFWriteC(file, context, ")\n");
                }
                break;
            }
            
            case DataDeskNodeType_ProcedureHeader:
            {
                if(root->procedure_header.return_type)
                {
                    _DataDeskFWriteGraphAsC(file, root->procedure_header.return_type, context);
                }
                else
                {
                    _DataDeskFWriteC(file, context, "void");
                }
                _DataDeskFWriteC(file, context, "%.*s(", root->string_length, root->string);
                if(root->procedure_header.first_parameter)
                {
                    for(DataDeskNode *parameter = root->procedure_header.first_parameter;
                        parameter; parameter = parameter->next)
                    {
                        _DataDeskFWriteGraphAsC(file, parameter, context);
                        if(parameter->next)
                        {
                            _DataDeskFWriteC(file, context, ", ");
                        }
                    }
                }
                else
                {
                    _DataDeskFWriteC(file, context, "void");
                }
                _DataDeskFWriteC(file, context, ");\n");
                break;
            }
            
            default: break;
        }
        
        _DataDeskCBumpLines(context, root);
    }
    
#undef _DDCScope
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteGraphAsC(FILE *file, DataDeskNode *root)
{
    DataDeskCPrintContext context = {0};
    _DataDeskFWriteGraphAsC(file, root, &context);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringWithSpaces(FILE *file, char *string)
{
    int string_length = 0;
    for(; string[string_length]; ++string_length);
    DataDeskFWriteStringWithSpacesN(file, string, string_length);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsLowercaseWithUnderscores(FILE *file, char *string)
{
    int string_length = 0;
    for(; string[string_length]; ++string_length);
    DataDeskFWriteStringAsLowercaseWithUnderscoresN(file, string, string_length);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsUppercaseWithUnderscores(FILE *file, char *string)
{
    int string_length = 0;
    for(; string[string_length]; ++string_length);
    DataDeskFWriteStringAsUppercaseWithUnderscoresN(file, string, string_length);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsUpperCamelCase(FILE *file, char *string)
{
    int string_length = 0;
    for(; string[string_length]; ++string_length);
    DataDeskFWriteStringAsUpperCamelCaseN(file, string, string_length);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsLowerCamelCase(FILE *file, char *string)
{
    int string_length = 0;
    for(; string[string_length]; ++string_length);
    DataDeskFWriteStringAsLowerCamelCaseN(file, string, string_length);
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringWithSpacesN(FILE *file, char *string, int string_length)
{
    for(int i = 0; i < string_length && string[i]; ++i)
    {
        if(string[i] != '_')
        {
            fprintf(file, "%c", string[i]);
            if(DataDeskCharIsUppercaseAlpha(string[i+1]))
            {
                fprintf(file, " ");
            }
            else if(string[i+1] == '_')
            {
                fprintf(file, " ");
            }
        }
    }
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsLowercaseWithUnderscoresN(FILE *file, char *string, int string_length)
{
    for(int i = 0; i < string_length && string[i]; ++i)
    {
        fprintf(file, "%c", DataDeskCharToLower(string[i]));
        if(DataDeskCharIsUppercaseAlpha(string[i+1]))
        {
            fprintf(file, "_");
        }
    }
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsUppercaseWithUnderscoresN(FILE *file, char *string, int string_length)
{
    for(int i = 0; i < string_length && string[i]; ++i)
    {
        fprintf(file, "%c", DataDeskCharToUpper(string[i]));
        if(DataDeskCharIsUppercaseAlpha(string[i+1]))
        {
            fprintf(file, "_");
        }
    }
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsUpperCamelCaseN(FILE *file, char *string, int string_length)
{
    int needs_uppercase = 1;
    for(int i = 0; i < string_length && string[i]; ++i)
    {
        if(string[i] != '_')
        {
            if(string[i+1] == '_')
            {
                needs_uppercase = 1;
            }
        }
        
        if(needs_uppercase)
        {
            fprintf(file, "%c", DataDeskCharToUpper(string[i]));
            needs_uppercase = 0;
        }
        else
        {
            fprintf(file, "%c", string[i]);
        }
    }
}

DATA_DESK_HEADER_PROC void
DataDeskFWriteStringAsLowerCamelCaseN(FILE *file, char *string, int string_length)
{
    int needs_uppercase = 0;
    int need_first_lowercase = 1;
    
    for(int i = 0; i < string_length && string[i]; ++i)
    {
        if(string[i] != '_')
        {
            if(string[i+1] == '_')
            {
                needs_uppercase = 1;
            }
        }
        
        if(needs_uppercase)
        {
            fprintf(file, "%c", DataDeskCharToUpper(string[i]));
            needs_uppercase = 0;
        }
        else
        {
            if(need_first_lowercase)
            {
                need_first_lowercase = 0;
                fprintf(file, "%c", DataDeskCharToLower(string[i]));
            }
            else
            {
                fprintf(file, "%c", string[i]);
            }
        }
    }
}

#endif // DATA_DESK_H_INCLUDED_

/*
Copyright 2020 Ryan Fleury

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
