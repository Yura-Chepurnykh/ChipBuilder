#include "parser.hpp"
#include "lexer_validation.hpp"

Parser::Parser(const std::string& text)  
{
    Lexer lexer(text);

    try {
        auto token = lexer.getCurrentToken();

        while (token.type != Type::EofToken) 
        {
            // DEBUGGING 
            // std::cout << token << std::endl;

            m_tokens.push_back(token);

            token = lexer.getCurrentToken();
        }
    }
    catch(const EmptyCommandError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const UnexpectedSymbolError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const UnterminatedStringError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const EmptyDashWordError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const EmptyDoubleDashWordError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const EmptyStringError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
    catch(const IllegalWordError& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
}

CommandScheme Parser::parse() 
{
    CommandScheme scheme;
    scheme.command = m_tokens[0];
    scheme.subCommand = m_tokens[1];

    for (size_t i = 2; i < m_tokens.size() - 1; i += 2)
        scheme.optionsToArguments.push_back({m_tokens[i], m_tokens[i+1]});

    scheme.flag = m_tokens.back();

    return scheme;
}