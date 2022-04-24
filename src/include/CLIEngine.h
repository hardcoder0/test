#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <picojson/picojson.h>

#include <LexingEngine.h>

namespace CLIEngine {
    std::map<std::string, std::string> getCmdFlags(std::vector<PekoLexingEngine::token> input) {
        std::map<std::string, std::string> flags;
        int i = 0;
        while(i < input.size()) {
            PekoLexingEngine::token c = input[i];
            if(c.value == "-"){
                std::pair<std::string, std::string> arg_and_val;

                i++;
                if(i < input.size()) {
                    arg_and_val.first = input[i].value;

                    i++;
                    if(i < input.size()) {
                        if(input[i].value == "=") {
                            i++;
                            if(i < input.size()) {
                                arg_and_val.second = input[i].value;
                                flags[arg_and_val.first] = arg_and_val.second;
                            }
                        }
                    }
                }
            }
            i++;
            if(i >= input.size())
                break;
        }

        return flags;
    }

    std::vector<std::string> getIdentifiers(std::vector<PekoLexingEngine::token> input) {
        std::vector<std::string> identifiers;

        int i = 0;
        while(i < input.size()) {
            PekoLexingEngine::token c = input[i];

            if(c.value != "-") {
                std::string cur_ident = "";

                while(input[i].value != "-") {
                    cur_ident += input[i].value;
                    i++;
                    if(i >= input.size())
                        break;
                }
            }
            i++;
            if(i >= input.size())
                break;
        }

        return identifiers;
    }

    std::string getPekoConfigPath() {
        std::string pekopath = "";
        #ifdef __MACH__
        pekopath = getenv("HOME");
        pekopath += "/.peko";
        #elif __unix__
        pekopath = getenv("HOME");
        pekopath += "/.peko";
        #elif __linux__
        pekopath = getenv("HOME");
        pekopath += "/.peko";
        #elif _WIN32
        pekopath = getenv("HOMEDRIVE");
        pekopath += getenv("HOMEPATH");
        pekopath += "/.peko";
        #endif
        return pekopath;
    }
/*
    std::map<std::string, std::string> getPekoConfig() {
        std::map<std::string, std::string> configs;
        std::string pekoconfigfile = "";

        std::ifstream pekoconfig_file(getPekoConfigPath() + "/config.json");
        pekoconfig_file.seekg(0, std::ios::end);
        size_t size = pekoconfig_file.tellg();
        std::string buffer(size, ' ');
        pekoconfig_file.seekg(0);
        pekoconfig_file.read(&buffer[0], size);
        pekoconfigfile = buffer;

        picojson::value v;
        std::string err = picojson::parse(v, pekoconfigfile);
        if (! err.empty()) {
            std::cerr << err << std::endl;
        }

        const picojson::value::object& configsmap = v.get<picojson::object>(); 
        for (picojson::value::object::const_iterator i = configsmap.begin(); i != configsmap.end(); ++i) {
            configs[i->first] = i->second.to_str();
        }

        return configs;
    }*/
}
