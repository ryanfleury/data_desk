// Sample code to print out the data from MD_ParseWholeFile
#include "md.h"
#include "md.c"

#define INDENT_SPACES 4
static void PrintNode(MD_Node* node, FILE* file, int indent_count) {
    char* indent_str = _MD_PushArray(_MD_GetCtx(), char, indent_count*INDENT_SPACES+1);
    for (int i = 0; i < indent_count*INDENT_SPACES; i++) {
        indent_str[i] = ' ';
    }
    indent_str[indent_count*INDENT_SPACES] = '\0';
    
    fprintf(file, "%sNode{\n", indent_str);
    fprintf(file, "%s    Kind: %.*s,\n", indent_str, MD_StringExpand(MD_StringFromNodeKind(node->kind)));
    
    int flags_bits = sizeof(node->flags)*8;
    char binary_flags[sizeof(node->flags)*8+1];
    binary_flags[flags_bits] = '\0';
    int flag_index = 0;
    MD_u32 flags = node->flags;
    for (int i = 0; i < flags_bits; i++) {
        binary_flags[i] = (flags&1) ? '1' : '0';
        flag_index++;
        flags >>= 1;
    }
    
    fprintf(file, "%s    Flags: %s,\n", indent_str, binary_flags);
    fprintf(file, "%s    Flag Names: ", indent_str, binary_flags);
    MD_String8List flags_list = MD_StringListFromNodeFlags(node->flags);
    MD_String8 flag_names = MD_JoinStringListWithSeparator(flags_list, MD_S8CString(", "));
    fprintf(file, "%.*s\n", MD_StringExpand(flag_names));
    
    if(node->string.size > 0) fprintf(file, "%s    String: %.*s,\n", indent_str, MD_StringExpand(node->string));
    if(node->whole_string.size > 0) fprintf(file, "%s    Whole String: %.*s,\n", indent_str, MD_StringExpand(node->whole_string));
    if (node->first_tag->kind != MD_NodeKind_Nil) {
        fprintf(file, "%s    Tags: ", indent_str);
        for (MD_EachNode(tag, node->first_tag)) {
            fprintf(file, "@%.*s, ", MD_StringExpand(tag->string));
        }
        fprintf(file, "\n");
    }
    
    for(MD_EachNode(child, node->first_child)) {
        PrintNode(child, file, indent_count+1);
    }
    fprintf(file, "%s}\n", indent_str);
}

int main(int argument_count, char **arguments)
{
    // NOTE(pmh): Parse all the files passed in via command line.
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i]));
        MD_PushSibling(&first, &last, MD_NilNode(), root);
    }
    
    for(MD_EachNode(root, first))
    {
        MD_String8 filename = MD_TrimExtension(MD_TrimFolder(root->filename));
        MD_String8List str_list = {0};
        MD_PushStringToList(&str_list, MD_S8CString("parsed_"));
        MD_PushStringToList(&str_list, filename);
        MD_PushStringToList(&str_list, MD_S8CString(".txt"));
        char* out_file = (char*)MD_PushStringCopy(MD_JoinStringList(str_list)).str;
        printf("Parse Input -> Output: %s -> %s\n", root->filename.str, out_file);
        
        FILE* file = fopen(out_file, "w");
        for(MD_EachNode(node, root->first_child))
        {
            PrintNode(node, file, 0);
            fprintf(file, "\n");
        }
        fclose(file);
    }
    
    return 0;
}