#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <stdio.h>
#include <string.h>

void print_separator(const char *title) {
    printf("\n========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

// Analyze a single function
void analyze_function(LLVMValueRef func) {
    const char *name = LLVMGetValueName(func);
    int is_decl = LLVMIsDeclaration(func);
    
    printf("\n  Function: @%s\n", name);
    printf("    Type: %s\n", is_decl ? "declaration (external)" : "definition");
    
    if (is_decl) {
        return;
    }
    
    // Count basic blocks
    int block_count = 0;
    for (LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(func);
         bb != NULL;
         bb = LLVMGetNextBasicBlock(bb)) {
        block_count++;
    }
    printf("    Basic blocks: %d\n", block_count);
    
    // Analyze instructions and find calls
    printf("    Calls:\n");
    int call_count = 0;
    
    for (LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(func);
         bb != NULL;
         bb = LLVMGetNextBasicBlock(bb)) {
        
        for (LLVMValueRef inst = LLVMGetFirstInstruction(bb);
             inst != NULL;
             inst = LLVMGetNextInstruction(inst)) {
            
            LLVMOpcode opcode = LLVMGetInstructionOpcode(inst);
            
            if (opcode == LLVMCall) {
                LLVMValueRef callee = LLVMGetCalledValue(inst);
                
                if (LLVMIsAFunction(callee)) {
                    const char *callee_name = LLVMGetValueName(callee);
                    printf("      -> @%s\n", callee_name);
                    call_count++;
                } else {
                    printf("      -> (indirect call)\n");
                    call_count++;
                }
            }
        }
    }
    
    if (call_count == 0) {
        printf("      (none - leaf function)\n");
    }
}

// Analyze global variables
void analyze_globals(LLVMModuleRef module) {
    print_separator("GLOBAL VARIABLES");
    
    int count = 0;
    for (LLVMValueRef global = LLVMGetFirstGlobal(module);
         global != NULL;
         global = LLVMGetNextGlobal(global)) {
        
        const char *name = LLVMGetValueName(global);
        int is_const = LLVMIsGlobalConstant(global);
        
        printf("  @%s (%s)\n", name, is_const ? "constant" : "mutable");
        count++;
    }
    
    if (count == 0) {
        printf("  (none)\n");
    }
    printf("\n  Total: %d global(s)\n", count);
}

// Categorize and list all functions
void analyze_functions(LLVMModuleRef module) {
    print_separator("FUNCTION ANALYSIS");
    
    int def_count = 0;
    int decl_count = 0;
    
    for (LLVMValueRef func = LLVMGetFirstFunction(module);
         func != NULL;
         func = LLVMGetNextFunction(func)) {
        
        analyze_function(func);
        
        if (LLVMIsDeclaration(func)) {
            decl_count++;
        } else {
            def_count++;
        }
    }
    
    print_separator("SUMMARY");
    printf("  Definitions count:                   %d\n", def_count);
    printf("  Declarations (extern/imports) count: %d\n", decl_count);
    printf("  Total functions:                     %d\n", def_count + decl_count);
}

int main(int argc, char *argv[]) {
    const char *filename = argv[1];

    // Create LLVM context
    LLVMContextRef context = LLVMContextCreate();
    LLVMMemoryBufferRef mem_buf;
    char *err_msg = NULL;

    // Read file into memory buffer
    if (LLVMCreateMemoryBufferWithContentsOfFile(filename, &mem_buf, &err_msg)) {
        fprintf(stderr, "Error reading file '%s': %s\n", filename, err_msg);
        LLVMDisposeMessage(err_msg);
        LLVMContextDispose(context);
        return 1;
    }

    // Parse IR into module
    LLVMModuleRef module;
    if (LLVMParseIRInContext(context, mem_buf, &module, &err_msg)) {
        fprintf(stderr, "Error parsing IR: %s\n", err_msg);
        LLVMDisposeMessage(err_msg);
        LLVMContextDispose(context);
        return 1;
    }

    // Print module info
    print_separator("MODULE INFO");
    size_t id_len = 0;
    const char *module_id = LLVMGetModuleIdentifier(module, &id_len);
    const char *target = LLVMGetTarget(module);
    printf("  Module ID: %s\n", module_id ? module_id : "(unnamed)");
    printf("  Target:    %s\n", (target && strlen(target)) ? target : "(default)");

    // Analyze
    analyze_globals(module);
    analyze_functions(module);

    // Cleanup
    LLVMDisposeModule(module);
    LLVMContextDispose(context);

    return 0;
}