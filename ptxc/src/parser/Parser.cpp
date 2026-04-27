#include "ptxc/parser/Parser.h"
#include <iostream>

namespace ptxc {

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    tokens_ = lexer_.tokenize();
}

Token Parser::peek() const {
    if (current_ >= tokens_.size()) return tokens_.back();
    return tokens_[current_];
}

Token Parser::advance() {
    if (current_ < tokens_.size()) return tokens_[current_++];
    return tokens_.back();
}

bool Parser::check(TokenKind kind) const {
    return peek().is(kind);
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenKind kind, const std::string& message) {
    if (check(kind)) return advance();
    error(peek(), message);
    return tokens_.back();
}

std::unique_ptr<ModuleNode> Parser::parseModule() {
    auto module = std::make_unique<ModuleNode>();
    while (!check(TokenKind::Eof)) {
        if (!peek().lexeme.empty() && peek().lexeme[0] == '.') {
            if (peek().isOneOf(TokenKind::DotEntry, TokenKind::DotFunc, TokenKind::DotVisible, TokenKind::DotExtern)) {
                auto func = parseFunctionOrEntry();
                module->functions.push_back(std::move(func));
            } else {
                parseGlobalDirective(*module);
            }
        } else if (peek().isInstruction()) {
             error(peek(), "Instructions must be inside a function");
             advance();
        } else {
            error(peek(), "Expected directive or function at global scope");
            advance();
        }
    }
    return module;
}

void Parser::parseGlobalDirective(ModuleNode& module) {
    Token tok = advance();
    module.loc = tok.loc;
    if (tok.is(TokenKind::DotVersion)) {
        Token version = consume(TokenKind::FloatLiteral, "Expected version number");
        module.version = version.lexeme;
    } else if (tok.is(TokenKind::DotTarget)) {
        Token target = consume(TokenKind::Identifier, "Expected target name");
        module.target = target.lexeme;
    } else if (tok.is(TokenKind::DotAddressSize)) {
        Token size = consume(TokenKind::IntegerLiteral, "Expected address size");
        module.addressSize = std::stoi(size.lexeme);
    } else {
        while (!match(TokenKind::Semicolon) && !check(TokenKind::Eof)) advance();
    }
}

std::unique_ptr<FunctionNode> Parser::parseFunctionOrEntry() {
    auto func = std::make_unique<FunctionNode>();
    func->loc = peek().loc;
    while (peek().isOneOf(TokenKind::DotVisible, TokenKind::DotExtern)) {
        if (advance().is(TokenKind::DotVisible)) func->isVisible = true;
    }
    Token kindTok = advance();
    if (kindTok.is(TokenKind::DotEntry)) func->isEntry = true;
    Token nameTok = consume(TokenKind::Identifier, "Expected function name");
    func->name = nameTok.lexeme;
    if (match(TokenKind::LParen)) {
        while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
            if (peek().is(TokenKind::DotParam)) {
                func->params.push_back(parseLocalDirective());
            } else { advance(); }
            match(TokenKind::Comma);
        }
        consume(TokenKind::RParen, "Expected ')'");
    }
    if (match(TokenKind::LBrace)) {
        while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
            func->body.push_back(parseStatement());
        }
        consume(TokenKind::RBrace, "Expected '}'");
    } else {
        match(TokenKind::Semicolon);
    }
    return func;
}

std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (check(TokenKind::Label)) {
        auto node = std::make_unique<LabelStmtNode>();
        Token tok = advance();
        node->loc = tok.loc;
        node->name = tok.lexeme;
        return node;
    }
    if (!peek().lexeme.empty() && peek().lexeme[0] == '.') return parseLocalDirective();
    if (check(TokenKind::At) || peek().isInstruction()) return parseInstruction();
    error(peek(), "Expected instruction, directive or label");
    advance();
    return nullptr;
}

std::unique_ptr<DirectiveStmtNode> Parser::parseLocalDirective() {
    auto node = std::make_unique<DirectiveStmtNode>();
    Token tok = advance();
    node->loc = tok.loc;
    node->directive = tok.lexeme;
    if (peek().isTypeModifier()) node->type = advance().lexeme;
    while (!check(TokenKind::Semicolon) && !check(TokenKind::Eof)) {
        node->operandList.push_back(parseOperand());
        match(TokenKind::Comma);
    }
    consume(TokenKind::Semicolon, "Expected ';'");
    return node;
}

std::unique_ptr<InstructionStmtNode> Parser::parseInstruction() {
    auto node = std::make_unique<InstructionStmtNode>();
    node->loc = peek().loc;
    if (match(TokenKind::At)) {
        if (match(TokenKind::Bang)) node->predicateNegated = true;
        node->predicateReg = consume(TokenKind::Register, "Expected pred reg").lexeme;
    }
    Token opcodeTok = consume(peek().kind, "Expected opcode");
    node->opcode = opcodeTok.lexeme;
    while (peek().isModifier()) node->opcode += advance().lexeme;
    while (!check(TokenKind::Semicolon) && !check(TokenKind::Eof)) {
        auto op = parseOperand();
        if (!op) break;
        node->operandList.push_back(std::move(op));
        match(TokenKind::Comma);
    }
    consume(TokenKind::Semicolon, "Expected ';'");
    return node;
}

std::unique_ptr<OperandNode> Parser::parseOperand() {
    Token tok = peek();
    if (tok.isOneOf(TokenKind::Register, TokenKind::SpecialReg)) {
        advance();
        auto node = std::make_unique<RegisterOpNode>();
        node->loc = tok.loc;
        node->name = tok.lexeme;
        if (match(TokenKind::LAngle)) {
            node->name += "<" + consume(TokenKind::IntegerLiteral, "size").lexeme + ">";
            consume(TokenKind::RAngle, ">");
        }
        return node;
    }
    if (tok.isOneOf(TokenKind::IntegerLiteral, TokenKind::FloatLiteral)) {
        advance();
        auto node = std::make_unique<ImmediateOpNode>();
        node->loc = tok.loc;
        node->value = tok.lexeme;
        node->isFloat = tok.is(TokenKind::FloatLiteral);
        return node;
    }
    if (tok.is(TokenKind::LBracket)) {
        advance();
        auto node = std::make_unique<AddressOpNode>();
        if (peek().isOneOf(TokenKind::Register, TokenKind::SpecialReg)) {
            node->baseReg = advance().lexeme;
            if (match(TokenKind::Plus)) node->offset = std::stoi(consume(TokenKind::IntegerLiteral, "offset").lexeme);
            else if (match(TokenKind::Minus)) node->offset = -std::stoi(consume(TokenKind::IntegerLiteral, "offset").lexeme);
        } else if (peek().is(TokenKind::Identifier)) node->symbol = advance().lexeme;
        else if (peek().is(TokenKind::IntegerLiteral)) node->offset = std::stoi(advance().lexeme);
        consume(TokenKind::RBracket, "]");
        return node;
    }
    if (tok.is(TokenKind::Identifier)) {
        advance();
        auto node = std::make_unique<LabelOpNode>();
        node->loc = tok.loc;
        node->name = tok.lexeme;
        return node;
    }
    return nullptr;
}

void Parser::error(const Token& token, const std::string& message) {
    errors_.push_back(token.loc.toString() + ": error: " + message);
}

void Parser::synchronize() {
    advance();
    while (!check(TokenKind::Eof)) {
        if (tokens_[current_-1].is(TokenKind::Semicolon)) return;
        if (peek().isOneOf(TokenKind::DotFunc, TokenKind::DotEntry)) return;
        advance();
    }
}

} // namespace ptxc
