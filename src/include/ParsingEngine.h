#pragma once
#include <asts.h>
#include <LexingEngine.h>

#include <map>
#include <stdlib.h>
#include <iostream>

namespace PekoParsingEngine {
    // Parser utility/helper functions
    PekoLexingEngine::token    get_cur_tok();
    void            increase_index();
    int             get_prec(char op);
    bool            isop(char op);
    bool            isunop(char op);

    // Parsing of primitive types: strings and numbers
    std::unique_ptr<ASTS::ExpAST> parse_number();
    std::unique_ptr<ASTS::ExpAST> parse_string();

    // Base parsing for expressions
    std::unique_ptr<ASTS::ExpAST> primary_parse();
    std::unique_ptr<ASTS::ExpAST> secondary_parse();
    std::unique_ptr<ASTS::ExpAST> parse_rhs_binop(int exp_prec, std::unique_ptr<ASTS::ExpAST> LHS);
    std::unique_ptr<ASTS::ExpAST> parse_expr();
    std::unique_ptr<ASTS::ExpAST> parse_paren_expr();
 
    // Basic parsing for more advanced expressions (ex: return statements, variable declarations)
    std::unique_ptr<ASTS::ExpAST> parse_return();
    std::unique_ptr<ASTS::ExpAST> parse_variable();
    std::unique_ptr<ASTS::ExpAST> parse_identifier();
    std::unique_ptr<ASTS::ExpAST> parse_if_expr(bool in_function);
    std::unique_ptr<ASTS::ExpAST> parse_loop_expr();

    // Parsing for functions
    std::vector<std::unique_ptr<ASTS::ExpAST>>  parse_block(); // parses a block of code that is encased in a {}
    std::unique_ptr<ASTS::ProtoAST>             parse_proto();
    std::unique_ptr<ASTS::ExpAST>               parse_function();

    std::unique_ptr<ASTS::ExpAST>               parse_object();
    std::unique_ptr<ASTS::ExpAST>               parse_object_access();
    std::unique_ptr<ASTS::ExpAST>               parse_new();

    std::unique_ptr<ASTS::ExpAST>               parse_array_acc();
    std::unique_ptr<ASTS::ExpAST>               parse_array_lit();
    

    // Different types for functions and variables
    enum TYPES {
        number_ty,
        string_ty,
        custom_ty,
        void_ty,
        array_ty
    };

    std::vector<PekoLexingEngine::token> toks;
    int index_in_overall_tokens = 0; // the index that is used throughout the whole parser, and indicates the current token index
    bool inObject = false;

    // A helper class that makes parsing much easier
    class PekoParser {
        std::vector<PekoLexingEngine::token> tokens;
        int index_in_tokens = 0;

    public:
        PekoParser(std::vector<PekoLexingEngine::token> tokens)
            : tokens(std::move(tokens)) {}

        std::vector<std::unique_ptr<ASTS::ExpAST>> Parse() {
            std::vector<std::unique_ptr<ASTS::ExpAST>> parsed_pekoscript;

            auto cur_tok = tokens.at(index_in_overall_tokens);
            
            while(
                index_in_overall_tokens < tokens.size()-1 &&
                (
                    cur_tok.type == PekoLexingEngine::fn_tk         || 
                    cur_tok.type == PekoLexingEngine::let_tk        || 
                    cur_tok.type == PekoLexingEngine::identifier_tk || 
                    cur_tok.type == PekoLexingEngine::new_line_tk   ||
                    cur_tok.type == PekoLexingEngine::object_tk
                )
            ) {
                parsed_pekoscript.push_back(std::move(primary_parse()));

                inObject = false;
                
                tokens = toks;

                cur_tok = tokens.at(index_in_overall_tokens);
                
                if(cur_tok.value == "}" || cur_tok.value == ";" || cur_tok.type == PekoLexingEngine::new_line_tk)
                    increase_index();
                else if(index_in_overall_tokens >= toks.size()-1) {
                    int x = index_in_overall_tokens;

                    ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m unexpected syntax: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  toks.at(x-1).value + " \033[;0;31m;" + toks.at(x).value + "\033[0;0m");
                    increase_index();
                    break;
                }
                    
                
                cur_tok = tokens.at(index_in_overall_tokens);
            }

            index_in_overall_tokens = 0;
            return parsed_pekoscript;
        }
    };

      // ++++++++++++++++++++++++++++++++++++++++++++++ //
     // ++++++++++ HELPER/UTILITY FUNCTIONS ++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++++++++ //

    /**
     * @brief Get the current token
     * 
     * @param toks the list of tokens that will be parsed
     * @return PekoLexingEngine::token 
     */
    PekoLexingEngine::token get_cur_tok() {return toks.at(index_in_overall_tokens);}

    /**
     * @brief Increases the overall index in the token list
     * 
     * @param toks 
     */
    void increase_index() { 
        if(index_in_overall_tokens < toks.size()-1) {
            index_in_overall_tokens++;
        }

        // Skip over all of the newline characters and update the current line each newline character we skip
        while(index_in_overall_tokens < toks.size()-1 && get_cur_tok().type == PekoLexingEngine::new_line_tk) {
            ErrorHandler::cur_line++;
            index_in_overall_tokens++;
        }
    }

    /**
     * @brief Returns the precedence of an operator
     * 
     * @param op the character that represents the operator of which you want to find the precedence of
     * @return int 
     */
    int get_prec(char op) {
        switch(op) {
        case '.':
            return 1;
        case '<':
        case '>':
            return 10;
        case '+':
        case '-':
            return 20;
        case '*':
            return 40;
        case '/':
        case '%':
            return 50;
        default:
            return -1;
        }
    }

    /**
     * @brief Returns true if the provided character is an operator
     * 
     * @param op the character to be tested as an operator or not
     * @return true 
     * @return false 
     */
    bool isop(char op) {
        switch(op) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>':
        case '<':
        case '.':
            return true;
        default:
            return false;
        }    
    }

    /**
     * @brief Returns true if the provided operator could be used in a unary operation (ex: -1)
     * 
     * @param op the character to be tested as a unary operand
     * @return true 
     * @return false 
     */
    bool isunop(char op) {
        switch(op) {
        case '+':
        case '-':
            return true;
        default:
            return false;
        }
    }

    /**
     * @brief Returns true if the provided operator is a comparison operator
     * 
     * @param op 
     * @return true 
     * @return false 
     */
    bool iscomp(std::string op) {
        if(
            op == "=="  ||
            op == "and" ||
            op == "or"
        ) {
            return true;
        } else {
            return true;
        }
    }

      // +++++++++++++++++++++++++++++++++++++++++++ //
     // ++++++++++ BASIC/PRIMARY PARSING ++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++ //

    /**
     * @brief Looks at the current token and parses itself and its following tokens into an AST
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> primary_parse() {
        PekoLexingEngine::token cur_tok = get_cur_tok(); // store the current token in a more easy to use variable

        if(get_cur_tok().value == "[") {
            return parse_array_lit();
        } else if(cur_tok.type == PekoLexingEngine::identifier_tk && !isop(toks.at(index_in_overall_tokens+1).value.at(0)) && !iscomp(toks.at(index_in_overall_tokens+1).value)) {
            return parse_identifier();
            
        } else if(cur_tok.type == PekoLexingEngine::identifier_tk && toks.at(index_in_overall_tokens+1).value == "[") {
            return parse_array_acc();
        } else if(cur_tok.type == PekoLexingEngine::identifier_tk && toks.at(index_in_overall_tokens+1).type == PekoLexingEngine::accessor_tk) {            
            return parse_object_access();

        // Parse numbers if the following token is not an operator
        } else if(cur_tok.type == PekoLexingEngine::num_tk && !isop(toks.at(index_in_overall_tokens+1).value.at(0)) && !iscomp(toks.at(index_in_overall_tokens+1).value)) {
            return parse_number();

        // Parse an expression in parentheses
        } else if(cur_tok.value == "(") {
            return parse_paren_expr();
        
        // Parse a variable declaration
        } else if(cur_tok.type == PekoLexingEngine::let_tk) {
            return parse_variable();

        // Parse a return statement
        } else if(cur_tok.type == PekoLexingEngine::return_tk) {
            return parse_return();

        // Parse a function declaration
        } else if(cur_tok.type == PekoLexingEngine::fn_tk) {
            return parse_function();

        // Parse a string literal if the following token is not an operator
        } else if(cur_tok.type == PekoLexingEngine::string_lit_tk && !isop(toks.at(index_in_overall_tokens+1).value.at(0)) && !iscomp(toks.at(index_in_overall_tokens+1).value)) {
            return parse_string();

        // Parse an if statement
        } else if(cur_tok.type == PekoLexingEngine::if_tk) {
            return parse_if_expr(false);

        // Parse a loop statement
        } else if(cur_tok.type == PekoLexingEngine::loop_tk) {
            return parse_loop_expr();

        // Parse a class declaration
        } else if(isunop(get_cur_tok().value.at(0))) {
            toks.insert(toks.begin() + index_in_overall_tokens, (PekoLexingEngine::token){"0", PekoLexingEngine::num_tk});
            return primary_parse();
        
        } else if(get_cur_tok().type == PekoLexingEngine::object_tk) {
            return parse_object();
        // Parse an expression
        } else {
            return parse_expr();
        }
    }

    /**
     * @brief A secondary parser for more simplistic expressions
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> secondary_parse() {
        auto cur_tok = get_cur_tok(); // save the current token in a easier to use form

        if(cur_tok.type == PekoLexingEngine::identifier_tk && toks.at(index_in_overall_tokens+1).value == "[") {
            return parse_array_acc();
        } else if(cur_tok.type == PekoLexingEngine::identifier_tk) {
            return parse_identifier();

        // Parse a number
        } else if(cur_tok.type == PekoLexingEngine::num_tk) {
            return parse_number();
        
        // Parse a parentheses expression
        } else if(cur_tok.value == "(") {
            return parse_paren_expr();

        // Parse a variable declaration
        } else if(cur_tok.type == PekoLexingEngine::let_tk) {
            return parse_variable();

        // Parse a string literal
        } else if(cur_tok.type == PekoLexingEngine::string_lit_tk) {
            return parse_string();

        } else if(isunop(get_cur_tok().value.at(0))) {
            toks.insert(toks.begin() + index_in_overall_tokens, (PekoLexingEngine::token){"0", PekoLexingEngine::num_tk});
            return secondary_parse();

        // Parse an expression
        } else {
            return parse_expr();
        }
    }

    /**
     * @brief Takes a string literal and turns it into an ast
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_string() {
        auto string_to_ast = std::make_unique<ASTS::StringExpAST>(get_cur_tok().value);

        increase_index(); // "eat" the strings value from the parser
        return std::move(string_to_ast);
    }

    /**
     * @brief Takes a number literal and converts it to an ast
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_number() {
        auto num_to_ast = std::make_unique<ASTS::NumberExpAST>(stod(get_cur_tok().value));
        
        increase_index(); // "eat" the numbers value from the parser
        return std::move(num_to_ast);
    }

    /**
     * @brief Parses the right hand side of an expression
     * 
     * @param toks  
     * @param LHS the left hand side of the expression
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_rhs_binop(int exp_prec, std::unique_ptr<ASTS::ExpAST> LHS) {
        while(true) {
            // Get the precedence of the current token/operator
            int tok_prec = 0;
            if(get_cur_tok().type != PekoLexingEngine::and_tk && get_cur_tok().type != PekoLexingEngine::or_tk && get_cur_tok().type != PekoLexingEngine::equal_to_tk)
                tok_prec = get_prec(get_cur_tok().value.at(0));
            else if(get_cur_tok().type == PekoLexingEngine::and_tk || get_cur_tok().type == PekoLexingEngine::or_tk)
                // "and" and "or" tokens have a precedence of 5
                tok_prec = 5;
            else if(get_cur_tok().type == PekoLexingEngine::equal_to_tk)
                // "==" tokens as all other comparison operators have a precedence of 10 
                tok_prec = 10;

            if(tok_prec < exp_prec) {
                return LHS;
            }

            // Save the current operator
            char bin_op = get_cur_tok().value.at(0);
            std::string bin_op_str = get_cur_tok().value;

            increase_index(); // eat the operator
            
            // get the right hand side of the expression
            auto RHS = secondary_parse();
            
            if(ErrorHandler::isErr()) {
                int i = index_in_overall_tokens;
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m incorrect rhs to expression: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "...\033[0;31m" + toks.at(i-1).value + "\033[0;0m");
                RHS = std::make_unique<ASTS::NumberExpAST>(0);
            }

            // Get the precedence of the next operator
            int new_prec = 0;
            if(get_cur_tok().type != PekoLexingEngine::and_tk && get_cur_tok().type != PekoLexingEngine::or_tk && get_cur_tok().type != PekoLexingEngine::equal_to_tk)
                new_prec = get_prec(get_cur_tok().value.at(0));
            else if(get_cur_tok().type == PekoLexingEngine::and_tk || get_cur_tok().type == PekoLexingEngine::or_tk)
                new_prec = 5;
            else if(get_cur_tok().type == PekoLexingEngine::equal_to_tk)
                new_prec = 10;

            // Parse the next part if the new token has a higher precedence than the original token
            if(tok_prec < new_prec) {
                RHS = parse_rhs_binop(tok_prec+1, std::move(RHS));
                    
                if(ErrorHandler::isErr()) {
                    int i = index_in_overall_tokens;
                    ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m incorrect rhs to expression: \n" + std::to_string(ErrorHandler::cur_line) + "| " + "...\033[0;31m" + toks.at(i-1).value + "\033[0;0m");
                    RHS = std::make_unique<ASTS::NumberExpAST>(0);
                }
            }

            // return the whole expression
            LHS = std::make_unique<ASTS::BinaryExpAST>(bin_op_str, std::move(LHS), std::move(RHS));
        }
    }

    /**
     * @brief Parses a binary expression
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_expr() {
        auto LHS = secondary_parse();
        
        if(ErrorHandler::isErr()) {
            ErrorHandler::preverr = true;
            LHS = std::make_unique<ASTS::NumberExpAST>(0);
        }

        auto RHS = parse_rhs_binop(0, std::move(LHS)); // parse the right side of the expression

        return RHS;
    }

    /**
     * @brief Parse an expression embedded in parentheses
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_paren_expr() {
        increase_index(); // eat the "("

        auto V = primary_parse();

        if(ErrorHandler::isErr()) {
            int i = index_in_overall_tokens;
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m incorrect expression: \n" + std::to_string(ErrorHandler::cur_line) + "| " + "...\033[0;31m" + toks.at(i-1).value + "\033[0;0m");
            V = std::make_unique<ASTS::NumberExpAST>(0);
        }

        if(get_cur_tok().value != ")") {
            int x = index_in_overall_tokens;

            // create spaces which will be used in the error logging
            std::string spaces = "";
            for(int i = 0; i < 4 + std::to_string(ErrorHandler::cur_line).length() + toks.at(x-1).value.length() + toks.at(x-2).value.length(); i++) {
                spaces += " ";
            }
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ')': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "..." + toks.at(x-2).value + toks.at(x-1).value + "\n" + spaces + "\033[;0;31m)^\033[0;0m");
        } else {
            increase_index(); // eat the ")"
        }

        return std::move(V);
    }

      // ++++++++++++++++++++++++++++++++++++++ //
     // ++++++++++ ADVANCED PARSING ++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++ //

    /**
     * @brief Parses a return statement
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_return() {
        increase_index(); // "eat" the return token

        // Get the value of the return'
        std::unique_ptr<ASTS::ExpAST> return_value = primary_parse();

        if(ErrorHandler::isErr()) {
            ErrorHandler::preverr = true;
            return_value = std::make_unique<ASTS::NumberExpAST>(0);
        }

        // create the AST for the return statement
        auto return_statement = std::make_unique<ASTS::ReturnExpAST>(std::move(return_value));
            
        return std::move(return_statement);
    }

    std::pair<int, std::string> parse_type() {
        std::pair<int, std::string> type;
        if(toks.at(index_in_overall_tokens+1).value == "["){
            std::string tname = get_cur_tok().value;
            increase_index();
            while(get_cur_tok().value == "[") {
                tname += " +";
                increase_index();
                if(get_cur_tok().value == "]") {
                    increase_index();
                } else {
                    return type;
                }
            }
            type = {array_ty, tname};
        } else if(get_cur_tok().type == PekoLexingEngine::number_tk) {
            type = {number_ty, "number"};
            increase_index();
        } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
            type = {string_ty, "string"};
            increase_index();
        } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
            type = {custom_ty, get_cur_tok().value};
            increase_index();
        }
        return type;
    }

    /**
     * @brief Parses a variable declaration
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::string cur_var_name = "";
    std::unique_ptr<ASTS::ExpAST> parse_variable() {
        cur_var_name = "";
        increase_index(); // eat the let token

        // Save the variables name
        std::string var_name = "";
        if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
            var_name = get_cur_tok().value;
        } else {
            int x = index_in_overall_tokens;
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected identifier: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "let \033[;0;31m" + get_cur_tok().value + "\033[0;0m=...");
            var_name = "fail";
        }

        cur_var_name = var_name;

        increase_index(); // eat the variables name

        // Continue if the next character is a semicolon
        if(get_cur_tok().value == ":") {
            increase_index();
        } else {
            int x = index_in_overall_tokens;

            // create spaces which will be used in the error logging
            std::string spaces = "";
            for(int i = 0; i < 2 + std::to_string(ErrorHandler::cur_line).length() + toks.at(x-1).value.length() + toks.at(x-2).value.length(); i++) {
                spaces += " ";
            }
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ':': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  toks.at(x-2).value + " " + toks.at(x-1).value + " " + toks.at(x).value + "\n" + spaces + "\033[;0;31m:^\033[0;0m");
        }

        // Get the varaibles value
        std::unique_ptr<ASTS::ExpAST> var_value = nullptr;

        // Save the variables type
        std::pair<int, std::string> type = parse_type();
        
        // if the declaration doesn't set the initial value
        if(get_cur_tok().value == ";") {
            // then a default value is given according to the type
            if(type.first == number_ty) {
                var_value = std::make_unique<ASTS::NumberExpAST>(0.0);
            } else if(type.first == string_ty) {
                var_value = std::make_unique<ASTS::StringExpAST>("");
            }
        
        // If the declaration does set the initial value
        } else if(get_cur_tok().value == "=") {
            increase_index(); // eat the "="

            // then the value is parsed
            var_value = primary_parse();

            increase_index();
        } else {
            int x = index_in_overall_tokens;

            // create spaces which will be used in the error logging
            std::string spaces = "";
            for(int i = 0; i < std::to_string(ErrorHandler::cur_line).length() + toks.at(x-1).value.length() - 1; i++) {
                spaces += " ";
            }
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ';' or '=': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  toks.at(x-1).value + " " + toks.at(x).value + "\n" + spaces + "\033[;0;31m;|=^\033[0;0m");
        }

        // Create the variable AST
        auto variable_ptr = std::make_unique<ASTS::VariableExpAST>(var_name, type, std::move(var_value));
        cur_var_name = "";

        return std::move(variable_ptr);
    }

    /**
     * @brief Parse an identifier to eithernumber_ty a variable re-assignment, variable reference, or a function call
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_identifier() {
        // Save the identifier
        int prev_index = index_in_overall_tokens;
        std::string identifier;
        if(get_cur_tok().type == PekoLexingEngine::identifier_tk) 
            identifier = get_cur_tok().value;
        else {
            int i = index_in_overall_tokens;
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected identifier: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "...\033[4;31m" + toks.at(i).value + "\033[0m...");
        }

        
        increase_index(); // eat the identifier
        
        // if the following tokens are +|-|/|*|=
        if(
            (
                get_cur_tok().value == "+" ||
                get_cur_tok().value == "-" ||
                get_cur_tok().value == "/" ||
                get_cur_tok().value == "*"
            ) 
            && toks.at(index_in_overall_tokens+1).value == "="
        ) {
            // skip the assignment tokens after saving the op
            std::string op = get_cur_tok().value;
            increase_index(); 
            increase_index();

            // Insert the identifier
            toks.insert(toks.begin()+index_in_overall_tokens, (PekoLexingEngine::token){identifier, PekoLexingEngine::identifier_tk});
            increase_index();

            // Insert the operand
            toks.insert(toks.begin()+index_in_overall_tokens, (PekoLexingEngine::token){op, PekoLexingEngine::unknown_tk});
            index_in_overall_tokens--;

            // Create a variable redef
            return std::make_unique<ASTS::VariableExpAST>(identifier, std::pair<int, std::string>({-1, "redec"}), std::move(primary_parse()));

        // If it is a lone identifier
        } else if(get_cur_tok().value != "(" && get_cur_tok().value != "=") {
            //toks.at(index_in_overall_tokens-2).value != "."  && 
            // Then it is a variable reference so create a variable reference
            return std::make_unique<ASTS::VariableRefExpAST>(identifier);

        // Otherwise we create a variable re-assignment
        } else if(get_cur_tok().value == "=" && !inObject) {
            increase_index();
            return std::make_unique<ASTS::VariableExpAST>(identifier, std::pair<int, std::string>({-1, "redec"}), std::move(primary_parse()));
        } else if(get_cur_tok().value == "=" && inObject) {
            return std::make_unique<ASTS::VariableRefExpAST>(identifier);
        } else if(get_cur_tok().value != "(") {
            int i = index_in_overall_tokens;
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected operator: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "...\033[4;31m" + toks.at(i).value + "\033[0m...");
        }

        increase_index(); // eat the "("

        // Parse the arguments
        std::vector<std::unique_ptr<ASTS::ExpAST>> arguments;

        while(get_cur_tok().value != ")") {
            auto arg = primary_parse();
            if(!ErrorHandler::isErr()) {
                // save the arguments value in a vector
                arguments.push_back(std::move(arg));
            } else if(ErrorHandler::isErr()) {
                int i = index_in_overall_tokens;
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m invalid argument: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "...\033[4;31m" + toks.at(i).value + "\033[0m...");
                break;
            } 

            // If the next token after parsing the first argument isn't a "," or ")", then an error should be returned
            if(get_cur_tok().value != "," && get_cur_tok().value != ")") {
                int x = index_in_overall_tokens;

                // create spaces which will be used in the error logging
                std::string spaces = "";
                for(int i = 0; i < std::to_string(ErrorHandler::cur_line).length() + toks.at(x-1).value.length() - 1; i++) {
                    spaces += " ";
                }
                
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ',' or ')': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  toks.at(x-1).value + " " + toks.at(x).value + "\n" + spaces + "\033[;0;31m,|)^\033[0;0m");
                break;
            }

            // Continue if ","
            if(get_cur_tok().value == ",") 
                increase_index();

            // Break if ")"
            if(get_cur_tok().value == ")") {
                break;
            }
        }

        if(get_cur_tok().value != ")") {
            int x = index_in_overall_tokens;

            // create spaces which will be used in the error logging
            std::string spaces = "";
            for(int i = 0; i < std::to_string(ErrorHandler::cur_line).length() + toks.at(x).value.length(); i++) {
                spaces += " ";
            }
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ')': \n" + std::to_string(ErrorHandler::cur_line) + "| ..." + toks.at(x).value + "\n" + spaces + "\033[;0;31m)^\033[0;0m");
        }
        
        increase_index();
        if(get_cur_tok().type == PekoLexingEngine::accessor_tk && !inObject) {
            index_in_overall_tokens = prev_index;
            return parse_object_access();
        }
        // Create the call
        return std::make_unique<ASTS::CallExpAST>(identifier, std::move(arguments));
    }

    /**
     * @brief Parses and if statement and converts it to an AST
     * 
     * @param toks 
     * @param in_function 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_if_expr(bool nonext=false) {
        increase_index(); // eat the if tokens

        auto condition = parse_paren_expr(); // parse the condition

        if(ErrorHandler::isErr()) {
            condition = std::make_unique<ASTS::NumberExpAST>(0);
        }

        std::vector<std::unique_ptr<ASTS::ExpAST>> body = parse_block(); // parse the body
        std::vector<std::unique_ptr<ASTS::ExpAST>> els = {};
        std::vector<std::unique_ptr<ASTS::IfExpAST>> els_if = {};
        increase_index(); // eat the }

        // parse an if else statement
        while(true) {
            if(get_cur_tok().type == PekoLexingEngine::else_tk && toks.at(index_in_overall_tokens+1).type == PekoLexingEngine::if_tk && !nonext) {
                increase_index();
                auto ifexp = parse_if_expr(true);
                std::unique_ptr<ASTS::IfExpAST> ifexpconv(dynamic_cast<ASTS::IfExpAST*>(ifexp.release()));
                els_if.push_back(std::move(ifexpconv));

            // parse an else statement
            } else if(get_cur_tok().type == PekoLexingEngine::else_tk && !nonext) {
                increase_index();
                els = parse_block();
                increase_index();
                break;
            } else {
                break;
            }
        }

        // parse the next commands after the block
        std::vector<std::unique_ptr<ASTS::ExpAST>> next = {};
        if(!nonext) {
            next = parse_block();
        }

        
        auto iff = std::make_unique<ASTS::IfExpAST>(std::move(condition), std::move(body), std::move(els), std::move(next), std::move(els_if));
        // Create the AST for the if statement
        return std::move(iff);
    }

    /**
     * @brief Parses a loop expression into an AST
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_loop_expr() {
        increase_index(); // eat the loop token

        auto condition = parse_paren_expr(); // parse the condition
        if(ErrorHandler::isErr()) {
            condition = std::make_unique<ASTS::NumberExpAST>(0);
        }

        auto for_body  = parse_block(); // parse the code to be ran
        increase_index(); // eat the "}"
        auto cont      = parse_block(); // get the code after the for loop

        return std::make_unique<ASTS::LoopExpAST>(std::move(condition), std::move(for_body), std::move(cont));
    }

      // ++++++++++++++++++++++++++++++++++++++ //
     // ++++++++++ FUNCTION PARSING ++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++ //

    /**
     * @brief Parses code encapsulated in a {}
     * 
     * @param toks 
     * @return std::vector<std::unique_ptr<ASTS::ExpAST>> 
     */
    std::vector<std::unique_ptr<ASTS::ExpAST>> parse_block() {
        std::vector<std::unique_ptr<ASTS::ExpAST>> block; // stores a list of expressions that will be parsed from this block
        
        if(get_cur_tok().value == "{")
            increase_index(); // eat the "{"
        
        while(index_in_overall_tokens < toks.size() && get_cur_tok().value != "}") {
            if(get_cur_tok().value == ";" || get_cur_tok().value == "\n" || get_cur_tok().value == ")")
                increase_index();
            
            if(get_cur_tok().value == "}")
                break;

            block.push_back(std::move(primary_parse())); // add the expression to the block list
            inObject = false;
            
            if(get_cur_tok().value == ";" || get_cur_tok().value == "\n" || get_cur_tok().value == ")")
                increase_index();
                
            if(get_cur_tok().value == "}")
                break;
        }
        
        return block;
    }

    /**
     * @brief Parses a functions prototype into an AST
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ProtoAST> 
     */
    std::unique_ptr<ASTS::ProtoAST> parse_proto() {
        increase_index(); // eat the function token

        std::string proto_name;

        // if the next token is an identifer
        if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
            proto_name = get_cur_tok().value; // then we set the prototypes name to the identifier            
        
        // Otherwise print an error
        } else {
            int i = index_in_overall_tokens;
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected identifier: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn \033[4;31m" + toks.at(i).value + "\033[0m(...");
        }

        increase_index(); // eat the identifier

        if(get_cur_tok().value == "(") {
            increase_index(); // continue if the next token is a "("
        
        // print an error
        } else {
            int x = index_in_overall_tokens;

            // create spaces which will be used in the error logging
            std::string spaces = "";
            for(int i = 0; i < 2 + std::to_string(ErrorHandler::cur_line).length() + toks.at(x-2).value.length() + toks.at(x-1).value.length(); i++) {
                spaces += " ";
            }
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected '(': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + toks.at(x-1).value + toks.at(x).value + "...\n" + spaces + "\033[;0;31m(^\033[0;0m");
        }
        
        std::vector<std::pair<std::string, std::pair<int, std::string>>> proto_args;

        // Parse the arguments
        while(get_cur_tok().value != ")") {
            std::pair<std::string, std::pair<int, std::string>> cur_arg;

            // the argument name should be an identifer
            if(get_cur_tok().type == PekoLexingEngine::identifier_tk) { 
                cur_arg.first = get_cur_tok().value; // set the arguments name to the current tokens value
                increase_index();
            
            // print an error
            } else {
                int x = index_in_overall_tokens;
                
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected an identifier: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(..." + "\033[0;31m" + toks.at(x).value + "\033[0;0m...");
            }
            
            // The next token should be a ':' to indicate the next token is a type
            if(get_cur_tok().value == ":") { 
                increase_index();

            // print an error if otherwise
            } else {
                int x = index_in_overall_tokens;

                // create spaces for error logging
                std::string spaces;
                for(int i = 0; i < 8 + std::to_string(ErrorHandler::cur_line).length() + proto_name.length() + cur_arg.first.length(); i++) {
                    spaces += " ";
                }
                
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ':': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(..." + cur_arg.first + " " + toks.at(x).value + "...)" + "\n" + spaces + "\033[;0;31m:^\033[0;0m");
            }

            // The next token should be a type
            if(get_cur_tok().type == PekoLexingEngine::number_tk) {
                cur_arg.second = {number_ty, "number"};
            } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
                cur_arg.second = {string_ty, "string"};
            } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
                cur_arg.second = {custom_ty, get_cur_tok().value};

            // if otherwise print an errorindex_in_overall_tokens < toks.size() && get_cur_tok().value != "}"
            } else {
                int x = index_in_overall_tokens;
                
                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected a type: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(..." + cur_arg.first + ": \033[0;31m" + toks.at(x).value + "\033[0;0m...)");
            }

            // add the current argument to the list of args for this prototype
            proto_args.push_back(std::move(cur_arg));

            increase_index(); // eat the number/string token

            // The next token should either be a ',' or a ')'
            if(get_cur_tok().value == ",") {
                increase_index(); // continue if ','
            
            // Print an error if the next token isn't a ")" or a ","
            } else if(get_cur_tok().value != ")") {
                int x = index_in_overall_tokens;

                // Create spaces
                std::string spaces;
                for(int i = 0; i < 8 + std::to_string(ErrorHandler::cur_line).length() + proto_name.length(); i++) {
                    spaces += " ";
                }

                ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ')': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(...\n" + spaces + "\033[;0;31m)^\033[0;0m");
            }
        }

        increase_index(); // eat the ')'

        // The next token should also be a ':' to indicate that the function type will be given next
        if(get_cur_tok().value == ":") {
            increase_index(); // continue if ':'
        
        // Otherwise print error
        } else {
            int x = index_in_overall_tokens;
            std::string spaces = "";
            for(int i = 0; i < 9 + std::to_string(ErrorHandler::cur_line).length() + proto_name.length() + toks.at(x).value.length(); i++) {
                spaces += " ";
            }

            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected ':': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(...) " + toks.at(x).value + "...\n" + spaces + "\033[;0;31m:^\033[0;0m");
        }

        // Stores the type of the prototype
        std::pair<int, std::string> proto_type;

        // Store the type of the function
        if(get_cur_tok().type == PekoLexingEngine::number_tk) {
            proto_type = {number_ty, "number"};
        } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
            proto_type = {string_ty, "string"};
        } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
            proto_type = {custom_ty, get_cur_tok().value};
        } else if(get_cur_tok().type == PekoLexingEngine::void_tk) {
            proto_type = {void_ty, "void"};

        // print an error otherwise
        } else {
            int x = index_in_overall_tokens;
            
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected type: \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + proto_name + "(...): " + "\033[;0;31m" + toks.at(x).value + "\033[0;0m");
        }
        
        // create the AST for the prototype
        auto proto = std::make_unique<ASTS::ProtoAST>(proto_name, proto_args, proto_type);
        
        return proto;
    }

    /**
     * @brief Parses a function into an AST
     * 
     * @param toks 
     * @return std::unique_ptr<ASTS::ExpAST> 
     */
    std::unique_ptr<ASTS::ExpAST> parse_function() {
        std::unique_ptr<ASTS::ProtoAST> fn_proto = parse_proto(); // parse the functions prototype

        if(!fn_proto) {
            fn_proto = std::make_unique<ASTS::ProtoAST>("failed", (std::vector<std::pair<std::string, std::pair<int, std::string>>>){}, (std::pair<int, std::string>){void_ty, "void"});
        }

        increase_index(); // eat the type token

        // print an error if the current token is not a "{"
        if(get_cur_tok().value != "{") {
            int x = index_in_overall_tokens;
            std::string spaces;
            for(int i = 0; i < 9 + std::to_string(ErrorHandler::cur_line).length() + fn_proto->getName().length(); i++) {
                spaces += " ";
            }
            ASTS::PrintERR(ErrorHandler::cur_file_path + ":" + std::to_string(ErrorHandler::cur_line) + " \033[0;31merror:\033[0;0m expected '{': \n" + std::to_string(ErrorHandler::cur_line) + "| " +  "fn " + fn_proto->getName() + "(...)\n" + spaces + "\033[;0;31m{^\033[0;0m");
        }

        std::vector<std::unique_ptr<ASTS::ExpAST>> fn_body = parse_block(); // parse the functions contents

        // create the functions AST
        auto fn_ast = std::make_unique<ASTS::FunctionExpAST>(std::move(fn_proto), std::move(fn_body));

        return fn_ast;
    }

    struct custom_type_attr {
        std::string name;
        std::pair<int, std::string> type;
    };

    struct custom_type {
        std::string name;
        std::vector<custom_type_attr> attributes;
    };

    std::unique_ptr<ASTS::ExpAST> parse_object() {
        increase_index();
        if(get_cur_tok().type != PekoLexingEngine::identifier_tk) {
            return nullptr;
        }

        std::string object_name = get_cur_tok().value;

        increase_index();

        if(get_cur_tok().value != "{") {
            return nullptr;
        }
    
        increase_index();

        if(get_cur_tok().type != PekoLexingEngine::identifier_tk) {
            return nullptr;
        }


        std::vector<std::pair<std::string, std::pair<int, std::string>>> object_attributes;

        while(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
            std::string id_name;
            std::pair<int, std::string> type;

            id_name = get_cur_tok().value;

            increase_index();

            if(get_cur_tok().value != ":") {
                return nullptr;
            } else {
                increase_index();
            }

            if(get_cur_tok().type == PekoLexingEngine::number_tk) {
                type.first = 0;
                type.second = "number";
            } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
                type.first = 1;
                type.second = "string";
            } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
                type.first = 2;
                type.second = get_cur_tok().value;
            }

            object_attributes.push_back({id_name, type});
            increase_index();
            if(get_cur_tok().value == ",") {
                increase_index();
            } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
                break;
            } else if(get_cur_tok().value != "}") {
                return nullptr;
            }
        }
        
        std::vector<std::unique_ptr<ASTS::FunctionExpAST>> functions;
        while(get_cur_tok().type == PekoLexingEngine::identifier_tk && get_cur_tok().type == PekoLexingEngine::identifier_tk && toks.at(index_in_overall_tokens+1).value == "(") {
            std::string fn_name = object_name + "." + get_cur_tok().value;
            std::vector<std::pair<std::string, std::pair<int, std::string>>> args;

            increase_index();
            increase_index();

            if(get_cur_tok().value != ")") {
                do {
                    if(get_cur_tok().type != PekoLexingEngine::identifier_tk) {
                        return nullptr;
                    }

                    std::string arg_name = get_cur_tok().value;
                    std::pair<int, std::string> type;

                    increase_index();

                    if(get_cur_tok().value != ":") {
                        return nullptr;
                    }

                    increase_index();

                    if(get_cur_tok().type == PekoLexingEngine::number_tk) {
                        type.second = "number";
                        type.first = 0;
                    } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
                        type.second = "string";
                        type.first = 1;
                    } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
                        type.second = get_cur_tok().value;
                        type.first = 2;
                    } else {
                        return nullptr;
                    }

                    args.push_back({arg_name, type});
                    increase_index();
                    if(get_cur_tok().value == ",") {
                        increase_index();
                    }
                } while(get_cur_tok().value != ")");
            }

            increase_index();

            if(get_cur_tok().value != ":") {
                return nullptr;
            }

            increase_index();
            
            std::pair<int, std::string> type;

            if(get_cur_tok().type == PekoLexingEngine::number_tk) {
                type.second = "number";
                type.first = 0;
            } else if(get_cur_tok().type == PekoLexingEngine::string_tk) {
                type.second = "string";
                type.first = 1;
            } else if(get_cur_tok().type == PekoLexingEngine::identifier_tk) {
                type.second = get_cur_tok().value;
                type.first = 2;
            } else if(get_cur_tok().type == PekoLexingEngine::void_tk) {
                type.second = "void";
                type.first = 3;
            } else {
                return nullptr;
            }

            increase_index();

            if(get_cur_tok().value != "{") {
                return nullptr;
            }

            auto fn_body = parse_block();
            //args.append()
            auto fn_proto = std::make_unique<ASTS::ProtoAST>(fn_name, args, type);
            auto func = std::make_unique<ASTS::FunctionExpAST>(std::move(fn_proto), std::move(fn_body));
            functions.push_back(std::move(func));

            increase_index();
        }

        increase_index();
        auto returnObject = std::make_unique<ASTS::ObjExpAST>(object_name, object_attributes, std::move(functions));
        return returnObject;
    }

    std::unique_ptr<ASTS::ExpAST> parse_object_access() {
        inObject = true;
        if(get_cur_tok().type != PekoLexingEngine::identifier_tk) {
            return nullptr;
        }

        std::unique_ptr<ASTS::ExpAST> LHS = parse_identifier();
        if(!strcmp(LHS->getType(), "var_ref")) {
            auto lhs_to_varref = dynamic_cast<ASTS::VariableRefExpAST*>(LHS.get());
            LHS = std::make_unique<ASTS::IdHolder>(lhs_to_varref->getVarName());
        }

        if(get_cur_tok().value == "=") {
            increase_index();
            inObject = false;
            auto varValue = primary_parse();

            return std::make_unique<ASTS::VariableExpAST>(toks[index_in_overall_tokens-3].value, (std::pair<int, std::string>){-1, ""}, std::move(varValue));
        } else if(get_cur_tok().type != PekoLexingEngine::accessor_tk) {
            return LHS;
        } else {
            increase_index();
        }

        if(get_cur_tok().type != PekoLexingEngine::identifier_tk) {
            return nullptr;
        }

        std::unique_ptr<ASTS::ExpAST> RHS = parse_object_access();
        

        return std::make_unique<ASTS::ObjectAccAST>(std::move(LHS), std::move(RHS));
    }

    std::unique_ptr<ASTS::ExpAST> parse_array_acc() {
        inObject = true; 
        std::unique_ptr<ASTS::ExpAST> LHS;
        
        if(get_cur_tok().value == "[") {
            increase_index();
            LHS = std::make_unique<ASTS::NumberExpAST>(stod(get_cur_tok().value));
            increase_index();
            increase_index();
        } else {
            LHS = parse_identifier();

            if(!strcmp(LHS->getType(), "var_ref")) {
                auto lhs_to_varref = dynamic_cast<ASTS::VariableRefExpAST*>(LHS.get());
                LHS = std::make_unique<ASTS::IdHolder>(lhs_to_varref->getVarName());
            }
        }
        //std::cout << "asdf" << std::endl;
        
        std::unique_ptr<ASTS::ExpAST> RHS;
        if(get_cur_tok().value == "[") {
            increase_index();
            RHS = std::make_unique<ASTS::NumberExpAST>(stod(get_cur_tok().value));

            increase_index();
            if(get_cur_tok().value == "]")
                increase_index();

            if(get_cur_tok().value == "[") {
                RHS = std::make_unique<ASTS::ArrayAccAST>(std::move(RHS), parse_array_acc());
            } else if(get_cur_tok().value == "=") {
                auto lhs_to_num = dynamic_cast<ASTS::NumberExpAST*>(RHS.get());
                increase_index();
                inObject = false;
                RHS = std::make_unique<ASTS::VariableExpAST>(std::to_string(lhs_to_num->getVal()), (std::pair<int, std::string>){-1, ""}, primary_parse());
            }
                
        } else if(get_cur_tok().value == "=") {
            auto lhs_to_num = dynamic_cast<ASTS::NumberExpAST*>(LHS.get());
            increase_index();
            inObject = false;
            RHS = std::make_unique<ASTS::VariableExpAST>(std::to_string(lhs_to_num->getVal()), (std::pair<int, std::string>){-1, ""}, primary_parse());
        }

        inObject = false;

        return std::make_unique<ASTS::ArrayAccAST>(std::move(LHS), std::move(RHS));
    }

    std::unique_ptr<ASTS::ExpAST> parse_array_lit() {
        increase_index();

        std::vector<std::unique_ptr<ASTS::ExpAST>> elements;

        while(get_cur_tok().value != "]") {
            elements.push_back(primary_parse());
            if(get_cur_tok().value == ",") {
                increase_index();
            }
        }

        increase_index();
        return std::make_unique<ASTS::ArrayLitAST>(std::move(elements));
    }
}