#include "parser.hpp"
#include "lexer_validation.hpp"

Parser::Parser(const std::string& text)  
{
    Lexer lexer(text);

    try {
        auto token = lexer.getCurrentToken();

        while (token.type != Type::EofToken) 
        {
            std::cout << token << std::endl;
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

