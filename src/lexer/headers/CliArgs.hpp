#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

struct CliArgs {
    std::string inputPath;
    std::string outputPath;
    std::string grammar = "1";
};


inline std::optional<CliArgs> parseCliArgs(int argc, char* argv[], std::string_view outFlag = "", 
                                                                   std::string_view outputPathDefault = "", 
                                                                   std::string_view inputPathDefault = "") {
    int i = 1;
    auto arg = [argv, argc, &i]() -> std::string_view { 
        return (i < argc) ? argv[i] : ""; 
    };

    CliArgs args;
    if (arg() == "-g" && (i + 1) < argc) { args.grammar = argv[++i]; ++i; } 
    if (arg() == outFlag && (i + 1) < argc) { args.outputPath = argv[++i]; ++i; } 
    if (!arg().starts_with('-')) { args.inputPath = arg(); }

    if (args.inputPath.empty()) {
        std::cerr << "Usage: " << argv[0] << " [-g <grammar>]" 
                  << " [" << outFlag << " <" << outputPathDefault << ">]" 
                  << " <" << inputPathDefault << ">\n";
        return std::nullopt;
    }

    return args;
}