#ifndef PTXC_LEXER_LEXER_H
#define PTXC_LEXER_LEXER_H

#include "ptxc/lexer/Token.h"
#include <string>
#include <vector>
#include <cstddef>

namespace ptxc {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<input>");
    ptxc::Token nextToken();
    std::vector<ptxc::Token> tokenize();
    bool isAtEnd() const;
    const std::vector<std::string>& errors() const { return errors_; }

private:
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();

    ptxc::Token makeToken(ptxc::TokenKind kind, const std::string& lexeme) const;
    ptxc::Token makeToken(ptxc::TokenKind kind, const std::string& lexeme, const ptxc::SourceLocation& loc) const;
    ptxc::Token errorToken(const std::string& message) const;

    ptxc::Token lexNumber();
    ptxc::Token lexDotKeyword();
    ptxc::Token lexPercentRegister();
    ptxc::Token lexIdentifierOrOpcode();
    ptxc::Token lexStringLiteral();

    static ptxc::TokenKind lookupDotKeyword(const std::string& word);
    static ptxc::TokenKind lookupOpcode(const std::string& word);

    std::string source_;
    std::string filename_;
    std::size_t pos_  = 0;
    int line_         = 1;
    int col_          = 1;
    int tokenLine_    = 1;
    int tokenCol_     = 1;
    std::vector<std::string> errors_;
};

} // namespace ptxc

#endif // PTXC_LEXER_LEXER_H
