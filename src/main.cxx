#include <LexingEngine.h>
#include <ParsingEngine.h>
#include <CLIEngine.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>
#include <dirent.h>
#include <filesystem>
#include <cstdlib> 
#include <ctime> 
#include <iostream>


void output_peko(std::string tmpdir, std::string filename) {
    std::string Str;
    llvm::raw_string_ostream OS(Str);
    OS << *ASTS::TheModule;
    OS.flush();

    std::ofstream pekoll_file;
    pekoll_file.open(tmpdir + "/" + filename);
    pekoll_file << Str << std::endl;
    pekoll_file.close();
}

int main(int argc, char *argv[]) {
    // Read the pekoscript file into a string
    
    std::ifstream peko_file(argv[1]);
    std::stringstream peko_buf;
    peko_buf << peko_file.rdbuf();

    std::string peko_str = peko_buf.str();
    while(peko_str.back() == '\n')
        peko_str.pop_back();
    
    while(peko_str.front() == '\n')
        peko_str.erase(0,1);

    //std::string stdlib_adv_classes = "class string {@vars{this.val:string_lit;}fn _init(val:string_lit):void{this.val=val;}fn _retval():string_lit{ret this.val;}}class number {@vars{this.val:number_lit;}fn _init(val:number_lit):void{this.val=val;}fn _retval():number_lit{ret this.val;}}";
    //peko_str.insert(0, stdlib_adv_classes);

    // Lex the pekoscript code
    std::vector<PekoLexingEngine::token> toks = PekoLexingEngine::lex_str(peko_str);
    
    std::vector<PekoLexingEngine::token> add_toks;
    for(int i = 0; i < toks.size(); i++) {
        if(toks[i].value == "#" && toks[i+1].value == "import" && toks[i+2].type == PekoLexingEngine::string_lit_tk) {
            i += 2;
            std::ifstream mod_file(toks[i].value);
            std::stringstream mod_buf;
            mod_buf << mod_file.rdbuf();

            std::string mod_str = mod_buf.str();
            auto mod_toks = PekoLexingEngine::lex_str(mod_str);
            add_toks.insert(add_toks.end(), mod_toks.begin(), mod_toks.end());
            i -= 2;
            for(int x = 0; x < 3; x++) {
                toks.erase(toks.begin()+i);
            }
        }
    }
     
    toks.insert(std::begin(toks), std::begin(add_toks), std::end(add_toks));
    PekoParsingEngine::toks = toks;
    
    PekoParsingEngine::PekoParser parser(toks);

    ASTS::CreateSTDLibFuncs();
 
    auto parsed_tokens = parser.Parse();

    if(!ErrorHandler::errored) {
        for(auto &ast : parsed_tokens) {
            ast->irgen();
        }

        std::string args_string = "";

        for(int i = 1; i < argc; i++) {
            args_string += argv[i];
            args_string += " ";
        }

        std::vector<PekoLexingEngine::token> args = PekoLexingEngine::lex_str(args_string);
        auto cmdflags = CLIEngine::getCmdFlags(args);

        std::string tmpdir = "/tmp";
        std::string filename = "peko.ll";
        
        /*auto config = CLIEngine::getPekoConfig();
        std::string clang = config["clang-12"];*/
        std::string clang = "clang-12";
        std::string pekopath = CLIEngine::getPekoConfigPath();

        std::string stdlibpath = pekopath + "/stdlib/stdlib.c";
        std::string osxtoolchain = pekopath + "/ostoolchains/osx";
        std::string wintoolchain = pekopath + "/ostoolchains/win32";
        
        std::string target_os = "";

        if(cmdflags.find("os") != cmdflags.end()) {
            target_os = cmdflags["os"];
            if(target_os != "linux" && target_os != "osx" && target_os != "win32") {
                char *string;
                while(strcmp(string, "linux") && strcmp(string, "osx") && strcmp(string, "win32")) {
                    printf("What's your target os (linux/mac/win32)? ");
                    scanf(" %m[^\n]", &string);
                }
                target_os = string;
            }
            
        } else {
            char *string;
            while(strcmp(string, "linux") && strcmp(string, "osx") && strcmp(string, "win32")) {
                printf("What's your target os (linux/osx/win32)? ");
                scanf(" %m[^\n]", &string);
            }
            target_os = string;
        }

        if(target_os == "linux") {
            ASTS::TheModule->setTargetTriple("x86_64-pc-linux-gnu");
            output_peko(tmpdir, filename);
            std::string cmd = clang +  + " " + tmpdir + "/" + filename + " " + stdlibpath;
            system(cmd.c_str());
        } else if(target_os == "osx") {
            ASTS::TheModule->setTargetTriple("x86_64-apple-macosx11.3.0-macho");
            output_peko(tmpdir, filename);
            std::string cmd = clang +  + " " + tmpdir + "/" + filename + " " + stdlibpath + " --target=x86_64-apple-darwin-macho -I " + osxtoolchain + "/MacOSX.sdk/usr/include -isysroot " + osxtoolchain + "/MacOSX.sdk -lto_library -lcrt1.o -fuse-ld=lld " + osxtoolchain + "/libclang_rt.osx.a";
            system(cmd.c_str());
        } else if(target_os == "win32") {
            ASTS::TheModule->setTargetTriple("i686-pc-windows-msvc19.11.0");
            output_peko(tmpdir, filename);
            std::string cmd = clang +  + " -Wno-deprecated-declarations -Wno-ignored-attributes " + tmpdir + "/" + filename + " " + stdlibpath + " -target i686-pc-win32 -fuse-ld=lld-link -I " + wintoolchain + "/include -L " + wintoolchain + "/lib";
            system(cmd.c_str());
        }
    }
}