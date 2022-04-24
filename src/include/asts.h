#ifndef ASTS_H
#define ASTS_H
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include "ParsingEngine.h"

// Include all of the llvm modules
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/FPEnv.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

void split(std::string str, std::string splitBy, std::vector<std::string>& tokens)
{
    /* Store the original string in the array, so we can loop the rest
     * of the algorithm. */
    tokens.push_back(str);

    // Store the split index in a 'size_t' (unsigned integer) type.
    size_t splitAt;
    // Store the size of what we're splicing out.
    size_t splitLen = splitBy.size();
    // Create a string for temporarily storing the fragment we're processing.
    std::string frag;
    // Loop infinitely - break is internal.
    while(true)
    {
        /* Store the last string in the vector, which is the only logical
         * candidate for processing. */
        frag = tokens.back();
        /* The index where the split is. */
        splitAt = frag.find(splitBy);
        // If we didn't find a new split point...
        if(splitAt == std::string::npos)
        {
            // Break the loop and (implicitly) return.
            break;
        }
        /* Put everything from the left side of the split where the string
         * being processed used to be. */
        tokens.back() = frag.substr(0, splitAt);
        /* Push everything from the right side of the split to the next empty
         * index in the vector. */
        tokens.push_back(frag.substr(splitAt+splitLen, frag.size()-(splitAt+splitLen)));
    }
}


namespace ErrorHandler {
    std::string cur_file_path = "";
    int cur_col = 0;
    int cur_line = 1;
    bool preverr = false;
    bool errored = false;

    bool isErr() {
        bool iserr = preverr;
        preverr = false;
        return iserr;
    }
}

namespace ASTS {
      // +++++++++++++++++++++++++++++++++ //
     // ++++++++++ AST CLASSES ++++++++++ //
    // +++++++++++++++++++++++++++++++++ //

    // The base class for all expressions
    class ExpAST {
    public:
        virtual ~ExpAST() = default;
        // The getType function will be found in all of the classes and will simply return a string correlating to the type of AST the current AST is
        virtual char *getType() = 0;

        // The irgen function is the function that will be found in all asts and generates the llvm ir
        virtual llvm::Value *irgen() = 0;
    };

    std::unique_ptr<ASTS::ExpAST> PrintERR(std::string err_msg) {
        std::cout << err_msg << std::endl;
        ErrorHandler::preverr = true;
        ErrorHandler::errored = true;
        return nullptr;
    }

    // A simple ast for storing a number
    class NumberExpAST : public ExpAST {
        double value;

    public:
        NumberExpAST(double V) 
            : value(V) {}
        
        double getVal() { return value; }
        char *getType();
        llvm::Value *irgen();
    };

    // A simple ast for storing a string
    class StringExpAST : public ExpAST {
        std::string value;

    public:
        StringExpAST(std::string V) 
            : value(std::move(V)) {}
        
        std::string getVal() { return value; }
        char *getType();
        llvm::Value *irgen();
    };

    // An ast for referencing a variable
    class VariableRefExpAST : public ExpAST {
        std::string var_name;
    
    public:
        VariableRefExpAST(std::string &v_name)
            : var_name(v_name) {}

        std::string getVarName() { return var_name; }
        char *getType();
        llvm::Value *irgen();
    };

    // For declaring/changing a variable
    class VariableExpAST : public ExpAST {
        std::string                 var_name;
        std::pair<int, std::string> var_type; //0: Number | 1: String
        std::unique_ptr<ExpAST>     var_value;

    public:
        VariableExpAST(std::string v_name, std::pair<int, std::string> v_type, std::unique_ptr<ExpAST> val)
            : var_name(v_name), var_type(std::move(v_type)), var_value(std::move(val)) {}

        char *getType();
        std::string getName() { return var_name; }
        ExpAST *getVAST() { return var_value.get(); }
        std::string getVarType() { return var_type.second; }
        llvm::Value *irgen();
    };

    // Stores an operand and the two other expression parts
    class BinaryExpAST : public ExpAST {   
    public:
        std::string op;
        std::unique_ptr<ExpAST> LHS, RHS;

        BinaryExpAST(std::string op, std::unique_ptr<ExpAST> lhs, std::unique_ptr<ExpAST> rhs)
            : op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}

        char *getType();
        llvm::Value *irgen();
    };

    // Allows for calling of a function
    class CallExpAST : public ExpAST {
        std::string callee;

    public:
        std::vector<std::unique_ptr<ExpAST>> args;
        std::vector<llvm::Value *> argsv_extra;

        CallExpAST(std::string &c, std::vector<std::unique_ptr<ExpAST>> a)
            : callee(c), args(std::move(a)) {}

        char *getType();
        void setCallee(std::string new_callee) { callee = new_callee; }
        std::string getCallee() { return callee; }
        ExpAST *getArg(int i) { return args.at(i).get(); }
        llvm::Value *irgen();
    };

    // Contains a function prototype
    class ProtoAST : public ExpAST {
        std::string func_name;
        std::vector<std::pair<std::string, std::pair<int, std::string>>> args; // the int part is the type the argument is

    public:
        std::pair<int, std::string> fn_type;

        ProtoAST(std::string name, std::vector<std::pair<std::string, std::pair<int, std::string>>> a, std::pair<int, std::string> type)
            : func_name(name), args(std::move(a)), fn_type(type) {}

        // Get the functions name
        std::string getName() { return func_name; }

        void appendArg(std::pair<std::string, std::pair<int, std::string>> arg) { args.push_back(arg); }
        void setName(std::string new_name) { func_name = new_name; }
        std::vector<std::pair<std::string, std::pair<int, std::string>>> getArgs() { return args; }
        char *getType();
        llvm::Function *irgen();
    };

    // Contains a function
    class FunctionExpAST : public ExpAST {
        std::unique_ptr<ProtoAST> Proto;
        std::vector<std::unique_ptr<ExpAST>> Body;
    public:
         FunctionExpAST(std::unique_ptr<ProtoAST> proto, std::vector<std::unique_ptr<ExpAST>> body)
            : Proto(std::move(proto)), Body(std::move(body)) {}

        ProtoAST *getProto() { return Proto.get(); }
        void setProto(std::unique_ptr<ProtoAST> new_proto) { Proto = std::move(new_proto); }
        std::vector<std::unique_ptr<ExpAST>> getBody() { return std::move(Body); }
        char *getType();
        llvm::Function *irgen();
    };

    // Store a returns value
    class ReturnExpAST : public ExpAST {
    public:
        std::unique_ptr<ExpAST> Ret_value;

        ReturnExpAST(std::unique_ptr<ExpAST> ret_value)
            : Ret_value(std::move(ret_value)) {}

        char *getType();

        // There is no irgeneration for this ast, it simply stores a return value
        llvm::Value *irgen();
    };

    // Stores an if statement
    class IfExpAST : public ExpAST {
        std::unique_ptr<ExpAST> condition;
        std::vector<std::unique_ptr<IfExpAST>> els_if;
        std::vector<std::unique_ptr<ExpAST>> body, els, then;

    public:
        IfExpAST(std::unique_ptr<ExpAST> cond, std::vector<std::unique_ptr<ExpAST>> b, std::vector<std::unique_ptr<ExpAST>> e, std::vector<std::unique_ptr<ExpAST>> t, std::vector<std::unique_ptr<IfExpAST>> eif)
            : condition(std::move(cond)), body(std::move(b)), els(std::move(e)), then(std::move(t)), els_if(std::move(eif)) {}

        char *getType();
        ExpAST* getCondition() { return condition.get(); }
        llvm::Value *irgen();
    };

    // Stores a loop expression
    class LoopExpAST : public ExpAST {
        std::unique_ptr<ExpAST> condition;
        std::vector<std::unique_ptr<ExpAST>> body, cont;

    public:
        LoopExpAST(std::unique_ptr<ExpAST> cond, std::vector<std::unique_ptr<ExpAST>> bod, std::vector<std::unique_ptr<ExpAST>> con)
            : condition(std::move(cond)), body(std::move(bod)), cont(std::move(con)) {}
        
        char *getType();
        std::vector<std::unique_ptr<ExpAST>> getBody() { return std::move(body); }
        std::vector<std::unique_ptr<ExpAST>> getAfter() { return std::move(cont); }
        llvm::Value *irgen();
    };

    class ObjExpAST : public ExpAST {
        std::string object_name;
        std::vector<std::pair<std::string, std::pair<int, std::string>>> object_attributes;
        std::vector<std::unique_ptr<ASTS::FunctionExpAST>> functions;

    public:
        ObjExpAST(std::string object_name, std::vector<std::pair<std::string, std::pair<int, std::string>>> object_attributes, std::vector<std::unique_ptr<ASTS::FunctionExpAST>> functions)
            : object_name(object_name), object_attributes(object_attributes), functions(std::move(functions)) {}
        
        char *getType();
        llvm::Value *irgen();
    };

    class IdHolder : public ExpAST {
        std::string id;

    public:
        IdHolder(std::string id)
            : id(id) {}
        
        char *getType();
        llvm::Value *irgen();
        std::string getId() { return id; }
    };

    class ObjectAccAST : public ExpAST {
        std::unique_ptr<ExpAST> LHS, RHS;

    public:
        ObjectAccAST(std::unique_ptr<ExpAST> LHS, std::unique_ptr<ExpAST> RHS)
            : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        
        char *getType();
        llvm::Value *irgen();
        ExpAST *GetLHS() { return LHS.get(); }
        ExpAST *GetRHS() { return RHS.get(); }
    };

    class ArrayAccAST : public ExpAST {
        std::unique_ptr<ExpAST> LHS, RHS;

    public:
        ArrayAccAST(std::unique_ptr<ExpAST> LHS, std::unique_ptr<ExpAST> RHS)
            : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        
        char *getType();
        llvm::Value *irgen();
        ExpAST *GetLHS() { return LHS.get(); }
        ExpAST *GetRHS() { return RHS.get(); }
    };

    class ArrayLitAST : public ExpAST {
        std::vector<std::unique_ptr<ExpAST>> elements;

    public:
        ArrayLitAST(std::vector<std::unique_ptr<ExpAST>> elem)
            : elements(std::move(elem)) {}
        
        char *getType();
        llvm::Value *irgen();
        ExpAST *getElement(int i) { return elements.at(i).get(); }
        int getSize() { return elements.size(); }
    };
    

    // The type getting for each ast
    char *NumberExpAST::getType()       { return (char *)"number"; }
 
    char *StringExpAST::getType()       { return (char *)"string"; }

    char *VariableRefExpAST::getType()  { return (char *)"var_ref"; }

    char *VariableExpAST::getType()     { return (char *)"variable"; }

    char *BinaryExpAST::getType()       { return (char *)"binary"; }

    char *CallExpAST::getType()         { return (char *)"call"; }

    char *ProtoAST::getType()           { return (char *)"prototype"; }

    char *FunctionExpAST::getType()     { return (char *)"function"; }

    char *ReturnExpAST::getType()       { return (char *)"return"; }

    char *IfExpAST::getType()           { return (char *)"if"; }

    char *LoopExpAST::getType()         { return (char *)"loop"; }

    char *ObjExpAST::getType()          { return (char *)"object"; }

    char *IdHolder::getType()           { return (char *)"id"; }

    char *ObjectAccAST::getType()       { return (char *)"object_acc"; }

    char *ArrayAccAST::getType()       { return (char *)"array_acc"; }

    char *ArrayLitAST::getType()       { return (char *)"arr_lit"; }

      // ++++++++++++++++++++++++++++++++++++++++ //
     // ++++++++++ LLVM IR GENERATION ++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //

    enum TYPES {
        number_ty,
        string_ty,
        custom_ty,
        void_ty,
        array_ty
    };

    // A structure for storing an llvm value and its type
    struct llvm_var {
        llvm::Value *val;
        llvm::Type  *type;
        bool        global;
    };

    struct global_llvm_var { 
        std::string name;
        std::unique_ptr<ExpAST> value;
        llvm::Type *type;
        bool redec;
    };

    struct class_type {
        llvm::Type* struct_ty;
        llvm::Type* struct_ptr_ty;
        std::map<std::string, int> type_name_map;
    };

    // Initialize all llvm components
    static llvm::LLVMContext TheContext;
    static std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("Epic pekoscript app", ASTS::TheContext);
    static llvm::IRBuilder<> Builder(TheContext);
    static std::map<std::string, llvm_var> NamedValues;
    static std::map<std::string, llvm_var> GlobalNamedValues;
    static std::map<std::string, class_type> allocatedObjects;
    static std::map<std::string, class_type> allocatedArrays;
    static std::map<std::string, std::vector<int>> ArraySizes;
    std::vector<ExpAST *> global_expressions;
    std::vector<global_llvm_var> global_vars;
    llvm::BasicBlock *Cur_BB;
    llvm::Value *RetVal = nullptr;
    bool inVarExp = false;

    /**
     * @brief Declares all of the stdlib functions to be called
     * 
     * @return int
     */
    int CreateSTDLibFuncs() {
        // For printing numbers
        std::vector<llvm::Type *> printnumargs;
        printnumargs.push_back(llvm::Type::getDoubleTy(TheContext));
        llvm::FunctionType *printnumType = llvm::FunctionType::get(Builder.getDoubleTy(), printnumargs, true);
        llvm::Function::Create(printnumType, llvm::Function::ExternalLinkage, "printnum", TheModule.get());

        // For printing numbers
        std::vector<llvm::Type *> inputargs;
        inputargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        llvm::FunctionType *inputType = llvm::FunctionType::get(Builder.getInt8PtrTy(), inputargs, true);
        llvm::Function::Create(inputType, llvm::Function::ExternalLinkage, "input", TheModule.get());

        // For printing numbers
        std::vector<llvm::Type *> inputnumargs;
        inputnumargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        llvm::FunctionType *inputnumType = llvm::FunctionType::get(Builder.getDoubleTy(), inputargs, true);
        llvm::Function::Create(inputnumType, llvm::Function::ExternalLinkage, "inputnum", TheModule.get());

        // For printing strings
        std::vector<llvm::Type *> printstrargs;                    
        printstrargs.push_back(llvm::Type::getInt8PtrTy(TheContext));                                      
        llvm::FunctionType *printstrType = llvm::FunctionType::get(Builder.getDoubleTy(), printstrargs, true);                                      
        llvm::Function::Create(printstrType, llvm::Function::ExternalLinkage, "printstr", TheModule.get());

        // For concatenating two strings
        std::vector<llvm::Type *> strargs;
        strargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        strargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        llvm::FunctionType *strcatType = llvm::FunctionType::get(Builder.getInt8PtrTy(), strargs, true);
        llvm::Function::Create(strcatType, llvm::Function::ExternalLinkage, "addstr", TheModule.get());

        // For using the modulus operator
        std::vector<llvm::Type *> cmpstrargs;
        cmpstrargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        cmpstrargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        llvm::FunctionType *cmpstrType = llvm::FunctionType::get(Builder.getDoubleTy(), cmpstrargs, true);
        llvm::Function::Create(cmpstrType, llvm::Function::ExternalLinkage, "cmpstr", TheModule.get());

        // For multiplying a string by a number
        std::vector<llvm::Type *> mulargs;
        mulargs.push_back(llvm::Type::getInt8PtrTy(TheContext));
        mulargs.push_back(llvm::Type::getDoubleTy(TheContext));
        llvm::FunctionType *mulstrType = llvm::FunctionType::get(Builder.getInt8PtrTy(), mulargs, true);
        llvm::Function::Create(mulstrType, llvm::Function::ExternalLinkage, "mulstr", TheModule.get());

        // For using the modulus operator
        std::vector<llvm::Type *> modargs;
        modargs.push_back(llvm::Type::getDoubleTy(TheContext));
        modargs.push_back(llvm::Type::getDoubleTy(TheContext));
        llvm::FunctionType *modnumType = llvm::FunctionType::get(Builder.getDoubleTy(), modargs, true);
        llvm::Function::Create(modnumType, llvm::Function::ExternalLinkage, "modnum", TheModule.get());


        return 1;
    }

    /**
     * @brief This AST doesn't have any value, it just holds an identifier for object access
     * 
     * @return llvm::Value* 
     */
    llvm::Value *IdHolder::irgen() {
        return nullptr;
    }

    /**
     * @brief Get the type name from allocatedObjects of an llvm::Type passed to the function
     * 
     * @param t 
     * @return std::string 
     */
    std::string getTypeName(llvm::Type *t) {
        std::string check_type; llvm::raw_string_ostream rso(check_type);
        t->print(rso); check_type = rso.str();

        for(auto const& obj: allocatedObjects) {
            std::string obj_type_ptr; llvm::raw_string_ostream oso(obj_type_ptr);
            obj.second.struct_ptr_ty->print(oso); obj_type_ptr = oso.str();

            std::string obj_type; llvm::raw_string_ostream oto(obj_type);
            obj.second.struct_ty->print(oto); obj_type = oto.str();

            if(check_type == obj_type || check_type == obj_type_ptr) {
                return obj.first;
            }
        }

        return "";
    }

    /**
     * @brief Gets a Builder.CreateGEP capable index from an int
     * 
     * @param index 
     * @return std::vector<llvm::Value*> 
     */
    std::vector<llvm::Value*> getGEPIndex(int index) {
        return {llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), llvm::ConstantInt::get(TheContext, llvm::APInt(32, index))};
    }

    /**
     * @brief These variables are used as global variables for recursing through the object access
     * 
     */
    ExpAST *prev_exp_ast = nullptr;
    llvm::Value *prev_llvm_value = nullptr;
    ExpAST *lhs_buf = nullptr;
    ExpAST *rhs_buf = nullptr;


    bool isArrType(llvm::Type *t) {
        std::string type_name; llvm::raw_string_ostream rso(type_name);
        t->print(rso); type_name = rso.str();

        return (type_name.find("***") != std::string::npos);
    }

    /**
     * @brief Resets the previsouly declared variables for the next object access
     * 
     */
    void resetObjRecVars() {
        prev_exp_ast = nullptr;
        prev_llvm_value = nullptr;
        lhs_buf = nullptr;
        rhs_buf = nullptr;
    }

    llvm::Value *getArrElementIndex(int i) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(TheContext), llvm::APInt(64, i));
    }

    llvm::Value *inst_arr(llvm::Type *type, int depth) {
        llvm::Value *previousarr = nullptr;
        llvm::Type *previoustype = nullptr;
        llvm::Type *previoustypenoptr = nullptr;

        for(int i = 0; i < depth; i++) {
            if(previousarr == nullptr) {
                // instantiate the array
                auto type_to_alloc = llvm::PointerType::getUnqual(type);
                auto alloc_arr = Builder.CreateAlloca(type_to_alloc, 0, "");
                auto size = llvm::ConstantExpr::getSizeOf(type);

                auto malloc = llvm::CallInst::CreateMalloc(
                    Builder.GetInsertBlock(),
                    llvm::Type::getInt64Ty(TheContext),
                    type, 
                    size, 
                    nullptr,
                    nullptr, 
                    ""
                );
                auto bitcast = Builder.Insert(Builder.CreateBitCast(malloc, type_to_alloc));
                Builder.CreateStore(bitcast, alloc_arr);

                auto load_arr = Builder.CreateLoad(alloc_arr);
                auto load_elem = Builder.CreateGEP(load_arr, getArrElementIndex(0));

                previousarr = alloc_arr;
                previoustype = type_to_alloc;
            } else {
                // instantiate the array
                auto type_to_alloc = llvm::PointerType::getUnqual(previoustype);
                auto alloc_arr = Builder.CreateAlloca(type_to_alloc, 0, "");
                auto size = llvm::ConstantExpr::getSizeOf(type_to_alloc);

                auto malloc = llvm::CallInst::CreateMalloc(
                    Builder.GetInsertBlock(),
                    llvm::Type::getInt64Ty(TheContext),
                    previoustype, 
                    size, 
                    nullptr,
                    nullptr, 
                    ""
                );
                auto bitcast = Builder.Insert(Builder.CreateBitCast(malloc, type_to_alloc));
                Builder.CreateStore(bitcast, alloc_arr);

                auto load_arr = Builder.CreateLoad(alloc_arr);
                auto load_elem = Builder.CreateGEP(load_arr, getArrElementIndex(0));

                Builder.CreateStore(Builder.CreateLoad(previousarr), load_elem);
                previousarr = alloc_arr;
                previoustype = type_to_alloc;
            }
        } 

        return previousarr;
    }

    void setElementAtIndex(llvm::Value *arr, int index, llvm::Value *set) {
        auto load_arr = Builder.CreateLoad(arr);

        Builder.CreateStore(set, Builder.CreateGEP(load_arr, getArrElementIndex(index)));
    }

    llvm::Value *getElementAtIndex(llvm::Value *arr, int index) {
        auto load_arr = Builder.CreateLoad(arr);
        return Builder.CreateGEP(load_arr, getArrElementIndex(index));
    }

    void appElement(llvm::Value *arr, llvm::Value *appendee, int newSize) {
        auto elem_0 = getElementAtIndex(arr, 0);
        auto elem_type = elem_0->getType();
        auto bas_elem_type = Builder.CreateLoad(elem_0)->getType();
        auto arrbuf = Builder.CreateAlloca(elem_type, 0, "");
        Builder.CreateStore(Builder.CreateLoad(arr), arrbuf);
        auto size = Builder.CreateMul(llvm::ConstantExpr::getSizeOf(arr->getType()), llvm::ConstantInt::get(TheContext, llvm::APInt(64, newSize, true)), "");

        auto malloc = llvm::CallInst::CreateMalloc(
            Builder.GetInsertBlock(),
            llvm::Type::getInt64Ty(TheContext),
            bas_elem_type, 
            size, 
            nullptr,
            nullptr, 
            ""
        );

        auto bitcast = Builder.Insert(Builder.CreateBitCast(malloc, elem_type));
        Builder.CreateStore(bitcast, arr);

        Builder.CreateStore(Builder.CreateLoad(arrbuf), arr);
    
        Builder.CreateStore(appendee, getElementAtIndex(arr, newSize-1));
    }

    llvm::Value *ObjectAccAST::irgen() {
        if(!lhs_buf && !prev_llvm_value && !prev_exp_ast && !rhs_buf) {
            lhs_buf = LHS.get();
            rhs_buf = RHS.get();
            auto rb_buf = rhs_buf;
            auto lb_buf = lhs_buf;

            if(!strcmp(lhs_buf->getType(), "id")) {
                auto lhs_to_id = dynamic_cast<IdHolder*>(lhs_buf);
                prev_llvm_value = NamedValues[lhs_to_id->getId()].val;
            } else if(!strcmp(lhs_buf->getType(), "call")) {
                prev_llvm_value = lhs_buf->irgen();
            }

            lhs_buf = lb_buf;
            rhs_buf = rb_buf;
            
            prev_exp_ast = lhs_buf;
            lhs_buf = rhs_buf;

            return this->irgen();
        } else if(!strcmp(lhs_buf->getType(), "id")) {
            // Convert the current lhs (rhs) to and IdHolder ast
            auto lhs_to_id = dynamic_cast<IdHolder*>(lhs_buf);

            // Find out the typename of the previous object to get its type name map
            std::string prev_type_name = getTypeName(prev_llvm_value->getType());
            
            if(prev_type_name.empty()) {
                return nullptr;
            }

            int acc_num = allocatedObjects[prev_type_name].type_name_map[lhs_to_id->getId()];

            // Create a GEP to get the value out of the previous object
            auto return_gep = Builder.CreateGEP(prev_llvm_value, getGEPIndex(acc_num));

            resetObjRecVars();
            // Reset the object vars for the next object access
            return Builder.CreateLoad(return_gep);
        // 
        } else if(!strcmp(lhs_buf->getType(), "variable")) {
            auto prev_lhs_to_id = dynamic_cast<IdHolder*>(prev_exp_ast);
            auto lhs_to_var = dynamic_cast<VariableExpAST*>(lhs_buf);

            std::string prev_type_name = getTypeName(prev_llvm_value->getType());  
            int acc_num = allocatedObjects[prev_type_name].type_name_map[lhs_to_var->getName()];

            auto return_gep = Builder.CreateGEP(prev_llvm_value, getGEPIndex(acc_num));
            
            resetObjRecVars();
            auto var_val = lhs_to_var->getVAST()->irgen();
            if(!getTypeName(var_val->getType()).empty()) {
                var_val = Builder.CreateLoad(var_val);
            }
            Builder.CreateStore(var_val, return_gep);

            return nullptr;
        } else if(!strcmp(lhs_buf->getType(), "object_acc")) {
            auto prev_lhs_to_id = dynamic_cast<IdHolder*>(prev_exp_ast);
            auto lhs_to_objacc = dynamic_cast<ObjectAccAST*>(lhs_buf);

            prev_exp_ast = lhs_buf;
            lhs_buf = lhs_to_objacc->GetLHS();

            if(!strcmp(lhs_buf->getType(), "id")) {
                auto lhs_lhs_to_id = dynamic_cast<IdHolder*>(lhs_to_objacc->GetLHS());

                std::string prev_type_name = getTypeName(prev_llvm_value->getType());            
                int acc_num = allocatedObjects[prev_type_name].type_name_map[lhs_lhs_to_id->getId()];

                auto return_gep = Builder.CreateGEP(prev_llvm_value, getGEPIndex(acc_num));

                prev_llvm_value = return_gep;
            } else if(!strcmp(lhs_buf->getType(), "call")) {
                auto lhs_to_call = dynamic_cast<CallExpAST*>(lhs_to_objacc->GetLHS());
                lhs_to_call->argsv_extra.push_back(prev_llvm_value);
                lhs_to_call->setCallee(getTypeName(prev_llvm_value->getType()) + "." + lhs_to_call->getCallee());
                
                prev_llvm_value = lhs_to_call->irgen();
            }
            
            
            lhs_buf = lhs_to_objacc->GetRHS();
            prev_exp_ast    = lhs_to_objacc->GetLHS();
            
            return this->irgen();
        } else if(!strcmp(lhs_buf->getType(), "call")) {
            auto lhs_to_call = dynamic_cast<CallExpAST*>(lhs_buf);

            lhs_to_call->argsv_extra.push_back(prev_llvm_value);
            lhs_to_call->setCallee(getTypeName(prev_llvm_value->getType()) + "." + lhs_to_call->getCallee());
            auto call = lhs_to_call->irgen();
            resetObjRecVars();
            return call;
        } else {
            return nullptr;
        }
    }

    llvm::Value *ObjExpAST::irgen() {
        std::vector<llvm::Type*> types;
        std::map<std::string, int> tname_map;
        int type_num = 0;
        for(auto type : object_attributes) {
            tname_map[type.first] = type_num;
            if(type.second.first == 0) {
                types.push_back(llvm::Type::getDoubleTy(TheContext));
            } else if(type.second.first == 1) {
                types.push_back(llvm::Type::getInt8PtrTy(TheContext));
            } else {
                types.push_back(allocatedObjects[type.second.second].struct_ty);
            }
            type_num++;
        }
        
        auto newStruct = llvm::StructType::create(TheContext, object_name);
        newStruct->setBody(types);

        allocatedObjects[object_name] = {newStruct, llvm::PointerType::getUnqual(newStruct), tname_map};
        
        for(auto &func : functions) {
            func->getProto()->appendArg({"this", {2, object_name}});
            func->irgen();
        }

        return nullptr;
    }
    
    llvm::Value *ReturnExpAST::irgen() {
        return Builder.CreateRet(Ret_value->irgen());
    }

    llvm::Value *LoopExpAST::irgen() {
        if(Cur_BB) {
            auto *TheFunction = Builder.GetInsertBlock()->getParent();
            auto *PreheaderBB = Builder.GetInsertBlock();
            auto *LoopBB      = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

            Builder.CreateBr(LoopBB);
            Builder.SetInsertPoint(LoopBB);

            for(int i = 0; i < body.size(); i++)
                body.at(i)->irgen();

            auto EndCond = condition->irgen();
            EndCond = Builder.CreateFPToUI(EndCond, llvm::Type::getInt1Ty(TheContext));

            auto *LoopEndBB = Builder.GetInsertBlock();
            auto *AfterBB = llvm::BasicBlock::Create(TheContext, "afterloop", TheFunction);

            Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
            Builder.SetInsertPoint(AfterBB);

            for(int i = 0; i < cont.size(); i++) {
               cont.at(i)->irgen();
            }
        } else {
            global_expressions.push_back(this);
        }

        return nullptr;
    }
    
    llvm::BasicBlock *CurrentInsertPoint = nullptr;
    llvm::BasicBlock *createIfBranch(std::vector<llvm::BasicBlock*> eif_blocks, llvm::BasicBlock *e_block, llvm::BasicBlock *cont_block, int *x, std::vector<std::unique_ptr<IfExpAST>> eif) {
        if(*x == (int)eif_blocks.size()-1) {
            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
            llvm::BasicBlock *newbb = llvm::BasicBlock::Create(TheContext, "condbb", TheFunction);
            
            auto CurInsPointBuf = CurrentInsertPoint;
            CurrentInsertPoint = newbb;
            
            Builder.SetInsertPoint(newbb);
            *x += 1;
            Builder.CreateCondBr(eif.at(*x-1)->getCondition()->irgen(), eif_blocks.at(*x-1), createIfBranch(eif_blocks, e_block, cont_block, x, std::move(eif)));
            
            CurrentInsertPoint = CurInsPointBuf;
            Builder.SetInsertPoint(CurrentInsertPoint);
            return newbb;
        } else if(*x <= (int)eif_blocks.size()-2) {
            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
            llvm::BasicBlock *newbb = llvm::BasicBlock::Create(TheContext, "condbb", TheFunction);
            
            auto CurInsPointBuf = CurrentInsertPoint;
            CurrentInsertPoint = newbb;
            
            Builder.SetInsertPoint(newbb);
            *x += 1;
            Builder.CreateCondBr(eif.at(*x-1)->getCondition()->irgen(), eif_blocks.at(*x-1), createIfBranch(eif_blocks, e_block, cont_block, x, std::move(eif)));
            
            CurrentInsertPoint = CurInsPointBuf;
            Builder.SetInsertPoint(CurrentInsertPoint);
            return newbb;
        } else if(e_block) {
            return e_block;
        } else {
            return cont_block;
        }
    }

    llvm::BasicBlock *ContBB = nullptr;
    llvm::Value *IfExpAST::irgen() {
        if(Cur_BB) {
            auto cond_gened = condition->irgen();
            CurrentInsertPoint = Builder.GetInsertBlock();
            auto CurInsPointBuf = CurrentInsertPoint;
            
            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
            llvm::BasicBlock *IfBodyBB = llvm::BasicBlock::Create(TheContext, "body", TheFunction);
            llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont", TheFunction);
            
            llvm::BasicBlock *ElseBB = nullptr;
            if(els.size() > 0) {
                ElseBB = llvm::BasicBlock::Create(TheContext, "else", TheFunction);
                Builder.SetInsertPoint(ElseBB);
                for(auto &ast : els) {
                    ast->irgen();
                    ASTS::resetObjRecVars();
                }
                Builder.CreateBr(MergeBB);
                Builder.SetInsertPoint(CurrentInsertPoint);
            }

            Builder.SetInsertPoint(IfBodyBB);
            
            for(int i = 0; i < body.size(); i++)
                body.at(i)->irgen();

            Builder.CreateBr(MergeBB);

            //IfBodyBB = Builder.GetInsertBlock();

            std::vector<llvm::BasicBlock *> elseif_blocks = {};

            for(auto &eif : els_if) {
                auto newbb = llvm::BasicBlock::Create(TheContext, "elseif", TheFunction);
                Builder.SetInsertPoint(newbb);
                for(auto &ex : eif->body) {
                    ex->irgen();
                    ASTS::resetObjRecVars();
                }
                Builder.CreateBr(MergeBB);
                elseif_blocks.push_back(newbb);
            }

            Builder.SetInsertPoint(MergeBB);
            for(int i = 0; i < then.size(); i++) {
                then.at(i)->irgen();
            }

            CurrentInsertPoint = CurInsPointBuf;

            Builder.SetInsertPoint(CurrentInsertPoint);
            Builder.CreateAlloca(llvm::Type::getInt128Ty(TheContext), nullptr, "adsf");

            int x = 0;
            Builder.CreateCondBr(cond_gened, IfBodyBB, createIfBranch(elseif_blocks, ElseBB, MergeBB, &x, std::move(els_if)));
            
            Builder.SetInsertPoint(MergeBB);
        } else {
            global_expressions.push_back(this);
        }

        return nullptr;
    }

    // Gives a value ref to a number
    llvm::Value *NumberExpAST::irgen() {
        return llvm::ConstantFP::get(TheContext, llvm::APFloat(value));
    }

    // Gives a value ref to a string
    llvm::Value *StringExpAST::irgen() {
        return Builder.CreateGlobalStringPtr(llvm::StringRef(value));
    }

    // Returns the reference to a variable
    llvm::Value *VariableRefExpAST::irgen() {
        // Look this variable up in the function.
        auto var = NamedValues[var_name]; 
        if (!var.val) {
            ASTS::PrintERR(ErrorHandler::cur_file_path + " \033[0;31merror:\033[0;0m undefined variable reference: " + var_name);
            return nullptr;
        }
            
        if(var.global == true && var.type == llvm::Type::getInt8PtrTy(TheContext)) {                
            return Builder.CreateLoad(TheModule->getNamedGlobal(var_name));
        } else if(var.type == llvm::Type::getInt8PtrTy(TheContext) || var.type == llvm::Type::getDoubleTy(TheContext)) {
            return Builder.CreateLoad(var.val);
        } else {
            return var.val;
        }
    
        return nullptr;
    }

    // Parse a Binary expression
    llvm::Value *BinaryExpAST::irgen() {
        // Retrieve the llvm::Value of the left hand and right hand sides
        llvm::Value *L = LHS->irgen();
        ASTS::resetObjRecVars();

        llvm::Value *R = RHS->irgen();
        ASTS::resetObjRecVars();

        // If either of the sides return null, than this expression tree is invalid
        if (!L || !R)
            return nullptr;

        // Generate the proper IR for the instruction
        if(op == "+") {
            if(L->getType() == llvm::Type::getInt8PtrTy(TheContext) && R->getType() == llvm::Type::getInt8PtrTy(TheContext))
                return Builder.CreateCall(TheModule->getFunction("addstr"), (std::vector<llvm::Value *>){L, R}, "calltmp");
            else
                return Builder.CreateFAdd(L, R, "addtmp");
        } else if(op == "-") {
            return Builder.CreateFSub(L, R, "subtmp");
        } else if(op == "*") {
            if(L->getType() == llvm::Type::getInt8PtrTy(TheContext) && R->getType() == llvm::Type::getDoubleTy(TheContext))
                return Builder.CreateCall(TheModule->getFunction("mulstr"), (std::vector<llvm::Value *>){L, R}, "calltmp");
            else
                return Builder.CreateFMul(L, R, "multmp");
        } else if(op == "<") {
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder.CreateFCmpULT(L, R, "cmptmp");
        } else if(op == ">") {
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder.CreateFCmpUGT(L, R, "cmptmp");
        } else if(op == "/") {
           return Builder.CreateFDiv(L, R, "divtmp");
        } else if(op == "%") {
           return Builder.CreateCall(TheModule->getFunction("modnum"), (std::vector<llvm::Value *>){L, R}, "calltmp");
        } else if(op == "==") {
            if(L->getType() == llvm::Type::getInt8PtrTy(TheContext) && R->getType() == llvm::Type::getInt8PtrTy(TheContext)) {
                return Builder.CreateFPToUI(Builder.CreateCall(TheModule->getFunction("cmpstr"), {L, R}, "added"), llvm::Type::getInt1Ty(TheContext));
            } else
                return Builder.CreateFCmpUEQ(L, R, "eqtmp");
        } else if(op == "and") {
            return Builder.CreateAnd(L, R, "andtmp");
        } else if(op == "or") {
            return Builder.CreateOr(L, R, "ortmp");
        } else {
            return nullptr;
        }
    }

    // Call an expression
    llvm::Value *CallExpAST::irgen() {
        if(Cur_BB && !inVarExp) {
            // Look up the name in the global module table.
            llvm::Function *CalleeF = TheModule->getFunction(callee);

            // If the function doesn't exist, then we return null
            if (!CalleeF) {
                return nullptr;
            }
            
            // If argument sizes don't match
            if (CalleeF->arg_size() != args.size() && (callee.find(".") == std::string::npos)) {
                return nullptr;
            }

            // Convert the arguments into llvm::Values
            std::vector<llvm::Value *> ArgsV;

            for (int i = 0; i < args.size(); i++) {
                auto cur_arg_val = args[i]->irgen();
                
                ArgsV.push_back(cur_arg_val);

                if (!ArgsV.back()) {
                    return nullptr;
                }
            }

            for(int i = 0; i < argsv_extra.size(); i++) {
                ArgsV.push_back(argsv_extra[i]);
            }


            if(CalleeF->getReturnType() == llvm::Type::getVoidTy(TheContext)) {
                return Builder.CreateCall(CalleeF, ArgsV);
            } else {
                return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
            }
        } else {
            global_expressions.push_back(this);
            return nullptr;
        }
    }
    
    ExpAST *prev_arr_ast = nullptr;
    llvm::Value *prev_arr_value = nullptr;
    ExpAST *arr_lhs_buf = nullptr;
    ExpAST *arr_rhs_buf = nullptr;
    int arracc_depth = 0;

    void resetArrRecVars() {
        prev_arr_ast = nullptr;
        prev_arr_value = nullptr;
        arr_lhs_buf = nullptr;
        arr_rhs_buf = nullptr;
        arracc_depth = 0;
    }

    std::vector<int> sizes_tmp;
    llvm::Value *ArrayAccAST::irgen() {
        if(!prev_arr_ast && !prev_arr_value && !arr_lhs_buf && !arr_rhs_buf) {
            arr_lhs_buf = LHS.get();
            arr_rhs_buf = RHS.get();
            if(!strcmp(arr_lhs_buf->getType(), "id")) {
                auto l_buf_to_id = dynamic_cast<IdHolder*>(arr_lhs_buf);
                prev_arr_value = NamedValues[l_buf_to_id->getId()].val;
            }

            prev_arr_ast = arr_lhs_buf;
            arr_lhs_buf = arr_rhs_buf;

            arracc_depth++;
            return this->irgen();
        } else if(!strcmp(arr_lhs_buf->getType(), "number")) {
            auto l_buf_to_number = dynamic_cast<NumberExpAST*>(arr_lhs_buf);
            
            auto element = getElementAtIndex(prev_arr_value, (int)l_buf_to_number->getVal());

            
            resetArrRecVars();
            return Builder.CreateLoad(element);
        } else if(!strcmp(arr_lhs_buf->getType(), "variable")) {
            auto l_buf_to_var = dynamic_cast<VariableExpAST*>(arr_lhs_buf);

            if(l_buf_to_var->getVAST()->getType() == "arr_lit") {
                auto arr = prev_arr_value;
                auto lbuf_to_arr_lit = dynamic_cast<ArrayLitAST*>(l_buf_to_var->getVAST());
                auto lbuf_arr = lbuf_to_arr_lit->irgen();

                int arrsize = sizes_tmp[arracc_depth];
                for(int size = 0; size < lbuf_to_arr_lit->getSize(); size++) {
                    if(size < arrsize) {
                        setElementAtIndex(arr, size, lbuf_to_arr_lit->getElement(size)->irgen());
                    } else {
                        appElement(arr, lbuf_to_arr_lit->getElement(size)->irgen(), size);
                    }
                }
            } else {
                setElementAtIndex(prev_arr_value, stoi(l_buf_to_var->getName()), l_buf_to_var->getVAST()->irgen());
            }
            resetArrRecVars();
            return nullptr;
        } else if(!strcmp(arr_lhs_buf->getType(), "array_acc")) {
            auto lbuf_to_acc = dynamic_cast<ArrayAccAST*>(arr_lhs_buf);

            arr_lhs_buf = lbuf_to_acc->GetLHS();
            arr_rhs_buf = lbuf_to_acc->GetRHS();


            auto lbuf_to_num = dynamic_cast<NumberExpAST*>(arr_lhs_buf);

            if(arr_rhs_buf) {
                prev_arr_value = getElementAtIndex(prev_arr_value, (int)lbuf_to_num->getVal());
                prev_arr_ast = arr_lhs_buf;
                arr_lhs_buf = arr_rhs_buf;
                arracc_depth++;
                return this->irgen();
            } else {
                resetArrRecVars();
                return Builder.CreateLoad(getElementAtIndex(prev_arr_value, (int)lbuf_to_num->getVal()));
            }
            


        }
        return nullptr;
    }

    llvm::Value *ArrayLitAST::irgen() {
        auto first = elements.at(0)->irgen();

        auto alloc = inst_arr(first->getType(), 1);

        sizes_tmp.push_back(0);

        sizes_tmp[sizes_tmp.size()-1] += 1;
        std::cout << sizes_tmp[sizes_tmp.size()-1] << std::endl;
        appElement(alloc, first, sizes_tmp[sizes_tmp.size()-1]);
        
        for(int i = 1; i < elements.size(); i++) {
            auto cur_elem = elements.at(i)->irgen();
            if(isArrType(cur_elem->getType())) cur_elem = Builder.CreateLoad(cur_elem);
            sizes_tmp[sizes_tmp.size()-1] += 1;
            appElement(alloc, cur_elem, sizes_tmp[sizes_tmp.size()-1]);
        }

        return Builder.CreateLoad(alloc);
    }

    // This creates a variable
    llvm::Value *VariableExpAST::irgen() {
        if(!Cur_BB) {
            inVarExp = true;
        }
        if(var_type.first == -1) {
            if(Cur_BB) {
                if(ArraySizes.find(var_name) != ArraySizes.end()) {
                    var_value->irgen();
                } else {
                    llvm::Value *V = var_value->irgen();

                    Builder.CreateStore(V, NamedValues[var_name].val);  
                }
                
            } else {
                global_vars.push_back((global_llvm_var){var_name, std::move(var_value), NamedValues[var_name].type, true});
            }
        } else {
            if(Cur_BB) {
                allocatedObjects["string"] = {llvm::Type::getInt8PtrTy(TheContext), llvm::Type::getInt8PtrTy(TheContext), {}};
                allocatedObjects["number"] = {llvm::Type::getDoubleTy(TheContext), llvm::Type::getDoubleTy(TheContext), {}};
                if(var_type.first == number_ty || var_type.first == string_ty) {
                    auto alloc = Builder.CreateAlloca(allocatedObjects[var_type.second].struct_ty, 0, var_name.c_str());
                    NamedValues[var_name] = {alloc, allocatedObjects[var_type.second].struct_ty, false};

                    llvm::Value *V = var_value->irgen();

                    if(!V) {
                        inVarExp = true;
                        return nullptr;
                    }

                    Builder.CreateStore(V, alloc);                    
                } else if(var_type.first == array_ty) {
                    std::vector<std::string> splitt;
                    split(var_type.second, " ", splitt);
                    llvm::Type *T = allocatedObjects[splitt.at(0)].struct_ty;
                    splitt.erase(splitt.begin());
                    int depth = 0;
                    for(auto plus : splitt) {
                        depth++;
                    }
                    auto alloc = inst_arr(T, depth);
                    NamedValues[var_name] = {alloc, alloc->getType(), false};
                    std::vector<int> sizes;

                    for(int i = 0; i < depth; i++) {
                        sizes.push_back(0);
                    }
                    ArraySizes[var_name] = sizes;
                    sizes_tmp = sizes;

                    if(var_value) {
                        if(!strcmp(var_value->getType(), "arr_lit")) {
                            auto v_to_arr_lit = dynamic_cast<ArrayLitAST*>(var_value.get());
                            
                            for(int i = 0; i < v_to_arr_lit->getSize(); i++) {
                                auto cur_elem = v_to_arr_lit->getElement(i)->irgen();
                                ArraySizes[var_name] = sizes_tmp;
                                ArraySizes[var_name][0] += 1;
                                if(isArrType(cur_elem->getType())) {
                                    cur_elem = Builder.CreateLoad(cur_elem);
                                }

                                appElement(NamedValues[var_name].val, cur_elem, ArraySizes[var_name][0]);
                            }
                        }
                    }
                    
                    for(auto size : sizes) sizes.pop_back();
                } else {
                    auto alloc = Builder.CreateAlloca(allocatedObjects[var_type.second].struct_ty, 0, var_name.c_str());
                    NamedValues[var_name] = {alloc, allocatedObjects[var_type.second].struct_ty, false};
                    llvm::Value *V;
                    if(var_value && !strcmp(var_value->getType(), "call")) {
                        auto val_to_call = dynamic_cast<CallExpAST*>(var_value.get());
                        val_to_call->setCallee(val_to_call->getCallee() + "._init");
                        val_to_call->argsv_extra.push_back(alloc);
                        val_to_call->irgen();
                        V = alloc;
                    }

                    V = alloc;


                    NamedValues[var_name] = {V, allocatedObjects[var_type.second].struct_ty, false};
                }
                
            } else {
                llvm::Type *gType;
                
                if(var_type.first == number_ty) {
                    gType = llvm::Type::getDoubleTy(TheContext);
                } else if(var_type.first == string_ty) {
                    gType = llvm::Type::getInt8PtrTy(TheContext);
                } else {
                    gType = allocatedObjects[var_type.second].struct_ty;
                }

                TheModule->getOrInsertGlobal(llvm::StringRef(var_name.c_str()), gType);
                auto gvar = TheModule->getNamedGlobal(llvm::StringRef(var_name.c_str()));

                gvar->setDSOLocal(true);
                gvar->setAlignment(llvm::MaybeAlign(8));
                if(var_type.first == number_ty)
                    gvar->setInitializer(llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)));
                else if(var_type.first == string_ty)
                    gvar->setInitializer(llvm::ConstantPointerNull::get(llvm::PointerType::get(Builder.getInt8PtrTy(), 0)));
                else
                    gvar->setInitializer(llvm::ConstantAggregateZero::get(llvm::PointerType::get(gType, 0)));
                
                GlobalNamedValues[var_name] = {gvar, gType, true};
                global_vars.push_back({var_name, std::move(var_value), gType, false});
            }
        }

        inVarExp = false;
        return nullptr;
    }

    llvm::Function *ProtoAST::irgen() {
        // Make the function type
        llvm::FunctionType *FT = nullptr;

        // If the function is main
        if(func_name == "main") {
            // we set its type to void
            FT = llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), {}, false);

        // Otherwise
        } else {
            // get the argument types
            std::vector<llvm::Type *> types;
            for(auto arg : args) {
                if(arg.second.first == number_ty)
                    types.push_back(llvm::Type::getDoubleTy(TheContext));
                else if(arg.second.first == string_ty)
                    types.push_back(llvm::Type::getInt8PtrTy(TheContext));
                else if(arg.second.first == custom_ty)
                    types.push_back(allocatedObjects[arg.second.second].struct_ptr_ty);
            } 

            // If the functions type is a number
            if(fn_type.first == number_ty) {
                // We set its type to double
                FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), types, false);
            } else if(fn_type.first == void_ty) {
                FT = llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), types, false);
            } else if(fn_type.first == string_ty) {
                FT = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(TheContext), types, false);
            } else if(fn_type.first == custom_ty) {
                FT = llvm::FunctionType::get(allocatedObjects[fn_type.second].struct_ptr_ty, types, false);
            }
        }

        llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, func_name, TheModule.get());
        // Set names for all arguments.
        unsigned Idx = 0;
        for (auto &Arg : F->args())
            Arg.setName(args[Idx++].first);

        return F;
    }

    llvm::Function *FunctionExpAST::irgen() {
        for(int i = 0; i < Body.size(); i++) {
            if(!strcmp(Body.at(i)->getType(), "function")) {
                Body.at(i)->irgen();
                Body.erase(Body.begin()+i);
                i--;
            }
        }

        // First, check for an existing function from a previous 'extern' declaration.
        llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());
        
        if (!TheFunction)
            TheFunction = Proto->irgen();

        if (!TheFunction)            
            return nullptr;        
   
        // Create a new basic block to start insertion into.
        Cur_BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
        Builder.SetInsertPoint(Cur_BB);
        
        if(Proto->getName() == "main") {
            for(int i = 0; i < global_vars.size(); i++) {
                if(global_vars.at(i).redec == false) {
                    if(global_vars.at(i).type == llvm::Type::getDoubleTy(TheContext)) {
                        auto number_val = global_vars.at(i).value->irgen();
                        auto store_number = Builder.CreateStore(number_val, TheModule->getNamedGlobal(global_vars.at(i).name));

                        NamedValues[global_vars.at(i).name] = {TheModule->getNamedGlobal(global_vars.at(i).name), llvm::Type::getDoubleTy(TheContext), true};
                    } else if(global_vars.at(i).type == llvm::Type::getInt8PtrTy(TheContext)) {
                        auto string_val = global_vars.at(i).value->irgen();
                        //auto string_all = Builder.CreateGlobalStringPtr("asdf"); // Create the global string

                        auto store_string = Builder.CreateStore(string_val, TheModule->getNamedGlobal(global_vars.at(i).name));

                        NamedValues[global_vars.at(i).name] = {std::move(TheModule->getNamedGlobal(global_vars.at(i).name)), llvm::Type::getInt8PtrTy(TheContext), true};
                    } else if(global_vars.at(i).type == llvm::Type::getInt32PtrTy(TheContext)) {
                        llvm::Function *CalleeF = TheModule->getFunction(global_vars.at(i).name);
                        std::vector<llvm::Value *> ArgsV;
                        for (unsigned i = 0, e = dynamic_cast<CallExpAST *>(global_vars.at(i).value.get())->args.size(); i != e; ++i) {
                            ArgsV.push_back(dynamic_cast<CallExpAST *>(global_vars.at(i).value.get())->args[i]->irgen());
                            if (!ArgsV.back())
                                return nullptr;
                        }
                        
                        Builder.CreateCall(CalleeF, ArgsV, "calltmp");
                    }
                } else {
                    Builder.CreateStore(global_vars.at(i).value->irgen(), TheModule->getNamedGlobal(global_vars.at(i).name));
                    NamedValues[global_vars.at(i).name] = {std::move(TheModule->getNamedGlobal(global_vars.at(i).name)), global_vars.at(i).type, true};
                }
            }

            for(int i = 0; i < Body.size(); i++) {
                Body.at(i)->irgen();
            }

            llvm::verifyFunction(*TheFunction);
            llvm::verifyModule(*TheModule);
            
            if(Proto->fn_type.first == void_ty) {
                Builder.CreateRetVoid();
            }

            Cur_BB = nullptr;
            
            return TheFunction;
        } else {
            // Record the function arguments in the NamedValues map.
            NamedValues.clear();
            NamedValues.insert(GlobalNamedValues.begin(), GlobalNamedValues.end());
            for (auto &Arg : TheFunction->args()) {
                llvm::IRBuilder<> TmpB(Cur_BB, Cur_BB->begin());
                if(Arg.getType() == llvm::Type::getInt8PtrTy(TheContext)) {
                    auto alloca = Builder.CreateAlloca(llvm::Type::getInt8PtrTy(TheContext), 0, Arg.getName());
                    Builder.CreateStore(Arg.getValueName()->second, alloca);

                    NamedValues[std::string(Arg.getName())] = {alloca, llvm::Type::getInt8PtrTy(TheContext), false};    
                } else if(Arg.getType() == llvm::Type::getDoubleTy(TheContext)) {

                    auto alloca = Builder.CreateAlloca(llvm::Type::getDoubleTy(TheContext), 0, Arg.getName());
                    auto store_value = Builder.CreateStore(Arg.getValueName()->second, alloca);
                    NamedValues[std::string(Arg.getName())] = {alloca, llvm::Type::getDoubleTy(TheContext), false};
                } else {
                    auto t = Arg.getType();
                    std::string tname = getTypeName(t);

                    NamedValues[std::string(Arg.getName())] = {Arg.getValueName()->second, allocatedObjects[tname].struct_ty, false};
                }
            }

            for(int i = 0; i < Body.size(); i++) {
                Body.at(i)->irgen();
            }

            if(Proto->fn_type.first == void_ty) {
                Builder.CreateRetVoid();
            }
            llvm::verifyFunction(*TheFunction);

            Cur_BB = nullptr;
            return TheFunction;
        } 
    }
}
#endif