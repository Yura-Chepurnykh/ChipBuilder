#include <unordered_map>
#include <vector>
#include <algorithm>
#include "semantic_analyzer.hpp"

// FIXME: review the code
// void SemanticAnalyzer::analyze(const CommandScheme& scheme) 
// {
//     auto matchOptionsToCommand = [](const Token& token) 
//     {
//         static const std::unordered_map<std::string, std::vector<std::string>> optsToCmd 
//         {
//             { "create", { "--width", "--height", "--pos" } },
//             { "remove", { } },
//             { "goto",   { } },
//             { "read",   { } },
//         };

//         return std::ranges::find(optsToCmd, token.content) != optsToCmd.end(); 
//     };

//     auto matchArgToCmd = [](const Token& opt, const Token& arg)
//     {
//         try 
//         {
//             if (opt.content == "--width")
//             {
//                 int width = std::stoi(arg);

//                 return width >= 0;
//             }

//             else if (opt.content == "--height")
//             {
//                 int height = std::stoi(arg);
//                 return height >= 0;
//             }

//             else if (opt.content == "--pos")
//             {
//                 auto comma = arg.find(",");
                
//                 if (comma != arg.end())
//                 {
//                     int x = std::stoi(arg.substr(0, comma));
//                     int y = std::stoi(arg.substr(comma + 1));

//                     return x >= 0 && y >= 0;
//                 }

//                 return false;
//             }
//         }
//         catch(const std::invalid_argument& e)
//         {
//             std::cerr << "Error: " << e.what() << std::endl;
//         }
//     };
// }