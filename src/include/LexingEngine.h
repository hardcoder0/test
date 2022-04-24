#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <map>

namespace PekoLexingEngine {
    // All token types
    enum Tokens {
        let_tk          = 1,
        identifier_tk   = 2,
        fn_tk           = 3,
        num_tk          = 4,
        number_tk       = 5,
        string_tk       = 6,
        unknown_tk      = 7,
        string_lit_tk   = 8,
        return_tk       = 9,
        new_line_tk     = 10,
        if_tk           = 11,
        void_tk         = 12,
        else_tk         = 13,
        loop_tk         = 14,
        equal_to_tk     = 16,
        or_tk           = 17,
        and_tk          = 17,
        new_tk          = 18,
        object_tk       = 19,
        accessor_tk     = 20,
    };

    // Used to save the value and type of a variable
    struct token {
        std::string value;
        int         type;
    };

    // Lexes a string into a list of tokens
    std::vector<token> lex_str(std::string peko) {
        int index_in_string = 0;
        std::vector<token> tokenized;

        // Loop through the string
        while(index_in_string < peko.size()) {
            // Skip spaces
            if(peko.at(index_in_string) == '\n') {
                while(peko.at(index_in_string) == '\n') {
                    tokenized.push_back((token){"\n", new_line_tk});
                    if(index_in_string < peko.size()-1) index_in_string++;
                }
            }
            
            if(isalpha(peko.at(index_in_string)) || peko.at(index_in_string) == '_') {
                std::string identifier = "";

                // Lex the identifier
                while(isalnum(peko.at(index_in_string)) || peko.at(index_in_string) == '_') {
                    identifier += peko.at(index_in_string);
                    if(index_in_string < peko.size()) index_in_string++;
                }

                // Check if the identifier is any of the built in tokens
                if(identifier == "let") {
                    tokenized.push_back((token){identifier, let_tk});
                } else if(identifier == "fn") {
                    tokenized.push_back((token){identifier, fn_tk});
                } else if(identifier == "number") {
                    tokenized.push_back((token){identifier, number_tk});
                } else if(identifier == "string") {
                    tokenized.push_back((token){identifier, string_tk});
                } else if(identifier == "return") {
                    tokenized.push_back((token){identifier, return_tk});
                } else if(identifier == "void") {
                    tokenized.push_back((token){identifier, void_tk});
                } else if(identifier == "if") {
                    tokenized.push_back((token){identifier, if_tk});
                } else if(identifier == "else") {
                    tokenized.push_back((token){identifier, else_tk});
                } else if(identifier == "loop") {
                    tokenized.push_back((token){identifier, loop_tk});
                } else if(identifier == "and") {
                    tokenized.push_back((token){identifier, and_tk});
                } else if(identifier == "or") {
                    tokenized.push_back((token){identifier, or_tk});
                } else if(identifier == "new") {
                    tokenized.push_back((token){identifier, new_tk});
                } else if(identifier == "object") {
                    tokenized.push_back((token){identifier, object_tk});
                } else {
                    tokenized.push_back((token){identifier, identifier_tk});
                }

                index_in_string--;
                
            // Tokenize numbers
            } else if(isdigit(peko.at(index_in_string))) {
                // Make sure there is only one decimal
                int is_dec = peko.at(index_in_string) == '.';
                std::string Number;

                // Lex the number into a string
                do {
                    if(peko.at(index_in_string) == '.') {
                        is_dec = true;
                    }

                    Number += peko.at(index_in_string);


                    if(index_in_string < peko.size()) index_in_string++;
                } while(isdigit(peko.at(index_in_string)) || (peko.at(index_in_string) == '.' && !is_dec));

                index_in_string--;

                tokenized.push_back((token){Number, num_tk});
            } else if(peko.at(index_in_string) == '"') {
                std::string str_lit = "";
                if(index_in_string < peko.size()) index_in_string++;

                while(peko.at(index_in_string) != '"') {
                    if(peko.at(index_in_string) == '\\') {
                        if(index_in_string < peko.size()) index_in_string++;
                    }

                    str_lit += peko.at(index_in_string);

                    if(index_in_string < peko.size()) index_in_string++;
                }

                tokenized.push_back((token){str_lit, string_lit_tk});

            // Skip line comments
            } else if(peko.at(index_in_string) == '/' && peko.at(index_in_string+1) == '/') {
                do {
                    if(index_in_string < peko.size()-1) index_in_string++;
                } while(peko.at(index_in_string) != '\n' && peko.at(index_in_string) != EOF && peko.at(index_in_string) != '\r');

                if(peko.at(index_in_string) == EOF) {
                    return tokenized;
                }

            // Skip block comments
            } else if(peko.at(index_in_string) == '/' && peko.at(index_in_string+1) == '*') {
                index_in_string++;

                do {
                    if(index_in_string < peko.size()-1) index_in_string++;
                } while((peko.at(index_in_string) != '*' && peko.at(index_in_string+1) != '/') && peko.at(index_in_string) != EOF);

                if(peko.at(index_in_string) == EOF) {
                    return tokenized;
                }

            // If the token is unknown, just add its value
            } else if(peko.at(index_in_string) == '=' && peko.at(index_in_string+1) == '=') {
                index_in_string++;
                tokenized.push_back((token){"==", equal_to_tk});
            } else if(peko.at(index_in_string) == '.') {
                tokenized.push_back((token){".", accessor_tk});
            } else if(!isspace(peko.at(index_in_string))) {
                tokenized.push_back((token){std::string(1, peko.at(index_in_string)), unknown_tk});
            }

            if(index_in_string < peko.size()-1) {index_in_string++;} else if(index_in_string >= peko.size()-1) {break;}
        }

        return tokenized;
    }
}
