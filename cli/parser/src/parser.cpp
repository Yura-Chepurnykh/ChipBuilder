#include "parser.hpp"
#include "lexer_validation.hpp"

Parser::Parser(const std::string& text)  
{
    Lexer lexer(text);


    auto token = lexer.getCurrentToken();

    while (token.type != Type::EofToken) 
    {
        // DEBUGGING 
        // std::cout << token << std::endl;

        m_tokens.push_back(token);

        token = lexer.getCurrentToken();
    }

    // TODO: move catch statements to the controller 
    // catch(const EmptyCommandError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl; 
    //     return EmptyCommand;
    // }
    // catch(const UnexpectedSymbolError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl; 
    //     return UnexpectedSymbol;
    // }
    // catch(const UnterminatedStringError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl; 
    //     return UnterminatedString;
    // }
    // catch(const EmptyDashWordError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return EmptyDashWord;
    // }
    // catch(const EmptyDoubleDashWordError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return EmptyDoubleDashWord;
    // }
    // catch(const EmptyStringError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return EmptyString;
    // }
    // catch(const IllegalWordError& e) 
    // {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return IllegalWord;
    // }

    // return Success;
}

CommandScheme Parser::parse() 
{
    auto validateCommand = [](const Token& token) 
    {
        constexpr std::array commands 
        {
            "create-nmos", "create-pmos", "create-n-substrate", 
            "create-p-substrate", "create-n-source", "create-p-source", 
            "create-n-drain", "create-p-drain", "create-poly", "create-oxide"
        };
        
        return std::ranges::find(commands, token.content) != commands.end();
    };

    auto validateOption = [](const Token& token)
    {
        constexpr std::array options 
        {
            "--width", "--height", "--pos", "--dopant"
        };

        return std::ranges::find(options, token.content) != options.end();
    };

    auto validateOptionMissing = [](const Token& token)
    {
        return token.type == Type::DoubleDashWord; 
    };

    auto validateArgumentMissing = [](const Token& token)
    {
        return token.type == Type::String; 
    };
    
    auto it = m_tokens.begin();
    auto end = m_tokens.end();

    const Token& command = *it++;

    CommandScheme scheme;

    if (!validateCommand(command))
        throw UnknownCommandError("Unknown command '" + m_tokens.front().content + "'");
   
    scheme.command = command;

    while (it != end)
    {
        const Token& option = *it++;

        if (!validateOptionMissing(option))
            throw MissingOptionError("Missing option");
    
        if (!validateOption(option))
            throw UnknownOptionError("Unknown option '" + option.content + "'");

        if (it == end)
            throw MissingArgumentError("Missing argument");

        const Token& argument = *it++;

        if (!validateArgumentMissing(argument))
            throw MissingArgumentError("Missing argument");
        
        scheme.optionsToArguments.emplace_back(option, argument);
    }

    return scheme;
}
