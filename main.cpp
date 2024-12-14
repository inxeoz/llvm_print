#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

int main() {
    // Step 1: Initialize the LLVM context and module
    // LLVMContext represents the global state for LLVM IR generation.
    LLVMContext Context;

    // A Module is a collection of LLVM functions, global variables, and other IR constructs.
    Module *ModuleOb = new Module("FileReadAndPrint", Context);

    // IRBuilder<> helps in constructing the LLVM IR incrementally. It's the primary tool for IR generation.
    IRBuilder<> Builder(Context);

    // Step 2: Declare external functions
    // Declaring external functions like fopen, printf, and fgetc which will be used in the program.

    // Declare 'fopen' function with the signature: const char* fopen(const char*, const char*)
    FunctionType *FopenType = FunctionType::get(Builder.getInt8PtrTy(), {Builder.getInt8PtrTy(), Builder.getInt8PtrTy()}, false);
    Function::Create(FopenType, Function::ExternalLinkage, "fopen", ModuleOb);

    // Declare 'printf' function with the signature: int printf(const char*, ...)
    FunctionType *PrintfType = FunctionType::get(Builder.getInt32Ty(), {Builder.getInt8PtrTy()}, true);
    Function::Create(PrintfType, Function::ExternalLinkage, "printf", ModuleOb);

    // Declare 'fgetc' function with the signature: int fgetc(FILE*)
    FunctionType *FgetcType = FunctionType::get(Builder.getInt32Ty(), {Builder.getInt8PtrTy()}, false);
    Function::Create(FgetcType, Function::ExternalLinkage, "fgetc", ModuleOb);

    // Step 3: Create global constants
    // These global variables will hold strings that are used in the program, such as file mode, error message, and format string for printing characters.

    // Create global "mode" for file opening mode (string "r")
    ModuleOb->getOrInsertGlobal("mode", ArrayType::get(Builder.getInt8Ty(), 2));
    GlobalVariable *Mode = ModuleOb->getNamedGlobal("mode");
    Mode->setInitializer(ConstantDataArray::getString(Context, "r", true));

    // Create global "err_msg" for the error message string
    ModuleOb->getOrInsertGlobal("err_msg", ArrayType::get(Builder.getInt8Ty(), 19));
    GlobalVariable *ErrMsg = ModuleOb->getNamedGlobal("err_msg");
    ErrMsg->setInitializer(ConstantDataArray::getString(Context, "File open failed!\n", true));

    // Create global "fmt_char" for the format string to print a character
    ModuleOb->getOrInsertGlobal("fmt_char", ArrayType::get(Builder.getInt8Ty(), 3));
    GlobalVariable *FmtChar = ModuleOb->getNamedGlobal("fmt_char");
    FmtChar->setInitializer(ConstantDataArray::getString(Context, "%c", true));

    // Step 4: Define the 'main' function
    // The 'main' function will take argc and argv as arguments and return an integer (int).
    FunctionType *MainType = FunctionType::get(Builder.getInt32Ty(), {Builder.getInt32Ty(), Builder.getInt8PtrTy()->getPointerTo()}, false);
    Function *MainFunc = Function::Create(MainType, Function::ExternalLinkage, "main", ModuleOb);

    // The entry point of the main function, where execution starts.
    BasicBlock *Entry = BasicBlock::Create(Context, "entry", MainFunc);
    Builder.SetInsertPoint(Entry);

    // Step 5: Extract arguments
    // Extract the arguments passed to the 'main' function: argc and argv.
    auto ArgIter = MainFunc->arg_begin();
    Value *Argc = ArgIter++;  // argc
    Value *Argv = ArgIter++;  // argv

    // Step 6: Check if argc > 1 (i.e., if there are command-line arguments)
    // If no arguments are passed, an error will occur.
    Value *ArgCheck = Builder.CreateICmpSGT(Argc, Builder.getInt32(1), "arg_check");

    // Create basic blocks for conditional execution: open_file or error
    BasicBlock *OpenFile = BasicBlock::Create(Context, "open_file", MainFunc);
    BasicBlock *Error = BasicBlock::Create(Context, "error", MainFunc);
    Builder.CreateCondBr(ArgCheck, OpenFile, Error);

    // Step 7: Open the file block
    // In this block, we extract the filename from argv[1] and call fopen.
    Builder.SetInsertPoint(OpenFile);
    Value *FilenamePtr = Builder.CreateInBoundsGEP(Builder.getInt8PtrTy(), Argv, Builder.getInt32(1), "filename_ptr");
    Value *Filename = Builder.CreateLoad(Builder.getInt8PtrTy(), FilenamePtr, "filename");

    // Call 'fopen' to open the file in read mode ("r")
    Value *File = Builder.CreateCall(ModuleOb->getFunction("fopen"), {Filename, Builder.CreateGlobalStringPtr("r")}, "file");

    // Check if fopen failed (if File is null)
    Value *FileCheck = Builder.CreateICmpEQ(File, Constant::getNullValue(Builder.getInt8PtrTy()), "file_check");

    // Create conditional branches based on file open success: either jump to the read loop or error.
    BasicBlock *ReadLoop = BasicBlock::Create(Context, "read_loop", MainFunc);
    Builder.CreateCondBr(FileCheck, Error, ReadLoop);

    // Step 8: Read loop
    // In this block, we allocate space to store characters read from the file and enter a loop.
    Builder.SetInsertPoint(ReadLoop);
    Value *CharPtr = Builder.CreateAlloca(Builder.getInt8Ty(), nullptr, "char_ptr");
    BasicBlock *Loop = BasicBlock::Create(Context, "loop", MainFunc);
    Builder.CreateBr(Loop);

    // Step 9: Loop body
    // The loop reads one character at a time from the file using 'fgetc' and stores it in CharPtr.
    Builder.SetInsertPoint(Loop);
    Value *Read = Builder.CreateCall(ModuleOb->getFunction("fgetc"), File, "read");

    // Truncate the result to an 8-bit character
    Value *Char = Builder.CreateTrunc(Read, Builder.getInt8Ty(), "char");
    Builder.CreateStore(Char, CharPtr);

    // Check for end of file (EOF) by comparing the result of 'fgetc' to -1
    Value *EOFCheck = Builder.CreateICmpEQ(Read, Builder.getInt32(-1), "eof_check");

    // Create conditional branches for either ending the loop or printing the character
    BasicBlock *Done = BasicBlock::Create(Context, "done", MainFunc);
    BasicBlock *PrintChar = BasicBlock::Create(Context, "print_char", MainFunc);
    Builder.CreateCondBr(EOFCheck, Done, PrintChar);

    // Step 10: Print the character
    Builder.SetInsertPoint(PrintChar);
    Value *LoadedChar = Builder.CreateLoad(Builder.getInt8Ty(), CharPtr, "loaded_char");

    // Call 'printf' to print the character
    Builder.CreateCall(ModuleOb->getFunction("printf"), {Builder.CreateGlobalStringPtr("%c"), LoadedChar});

    // Continue the loop
    Builder.CreateBr(Loop);

    // Step 11: Error block
    // If file open fails, print an error message and return a non-zero value.
    Builder.SetInsertPoint(Error);
    Builder.CreateCall(ModuleOb->getFunction("printf"), {Builder.CreateGlobalStringPtr("File open failed!\n")});
    Builder.CreateRet(Builder.getInt32(1));

    // Step 12: Done block
    // If the loop completes, return a zero value to indicate success.
    Builder.SetInsertPoint(Done);
    Builder.CreateRet(Builder.getInt32(0));

    // Step 13: Print the module's IR to the standard output
    // This allows you to see the generated LLVM IR code.
    ModuleOb->print(outs(), nullptr);

    // Clean up the module
    delete ModuleOb;

    return 0;
}
