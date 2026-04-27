#ifndef PTXC_PARSER_PARSER_H
#define PTXC_PARSER_PARSER_H

#include "ptxc/lexer/Lexer.h"
#include "ptxc/parser/AST.h"
#include <vector>
#include <string>
#include <memory>

namespace ptxc {

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<ModuleNode> parseModule();

    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    Token peek() const;
    Token advance();
    bool match(TokenKind kind);
    bool check(TokenKind kind) const;
    Token consume(TokenKind kind, const std::string& message);

    void parseGlobalDirective(ModuleNode& module);
    std::unique_ptr<FunctionNode> parseFunctionOrEntry();
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<InstructionStmtNode> parseInstruction();
    std::unique_ptr<DirectiveStmtNode> parseLocalDirective();
    std::unique_ptr<OperandNode> parseOperand();

    void error(const Token& token, const std::string& message);
    void synchronize();

    Lexer& lexer_;
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::vector<std::string> errors_;
};

} // namespace ptxc

#endif // PTXC_PARSER_PARSER_H
