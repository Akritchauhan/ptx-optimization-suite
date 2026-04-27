#include "ptxc/lexer/Lexer.h"
#include <cctype>
#include <algorithm>
#include <unordered_map>

namespace ptxc {

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename) {}

std::vector<ptxc::Token> Lexer::tokenize() {
    std::vector<ptxc::Token> tokens;
    ptxc::Token tok;
    do {
        tok = nextToken();
        tokens.push_back(tok);
    } while (!tok.is(ptxc::TokenKind::Eof));
    return tokens;
}

ptxc::Token Lexer::nextToken() {
    skipWhitespace();
    tokenLine_ = line_;
    tokenCol_  = col_;
    if (isAtEnd()) return makeToken(ptxc::TokenKind::Eof, "");

    char c = peek();
    if (c == '/' && pos_ + 1 < source_.size()) {
        if (source_[pos_ + 1] == '/') { skipLineComment(); return nextToken(); }
        if (source_[pos_ + 1] == '*') { skipBlockComment(); return nextToken(); }
    }
    if (c == '.') return lexDotKeyword();
    if (c == '%') return lexPercentRegister();
    if (std::isdigit(c)) return lexNumber();
    if (std::isalpha(c) || c == '_' || c == '$') return lexIdentifierOrOpcode();
    if (c == '"') return lexStringLiteral();

    advance();
    switch (c) {
        case '{': return makeToken(ptxc::TokenKind::LBrace,    "{");
        case '}': return makeToken(ptxc::TokenKind::RBrace,    "}");
        case '(': return makeToken(ptxc::TokenKind::LParen,    "(");
        case ')': return makeToken(ptxc::TokenKind::RParen,    ")");
        case '[': return makeToken(ptxc::TokenKind::LBracket,  "[");
        case ']': return makeToken(ptxc::TokenKind::RBracket,  "]");
        case '<': return makeToken(ptxc::TokenKind::LAngle,    "<");
        case '>': return makeToken(ptxc::TokenKind::RAngle,    ">");
        case ',': return makeToken(ptxc::TokenKind::Comma,     ",");
        case ';': return makeToken(ptxc::TokenKind::Semicolon, ";");
        case '+': return makeToken(ptxc::TokenKind::Plus,      "+");
        case '-':
            if (!isAtEnd() && std::isdigit(peek())) {
                pos_--; col_--;
                return lexNumber();
            }
            return makeToken(ptxc::TokenKind::Minus, "-");
        case ':': return makeToken(ptxc::TokenKind::Colon,     ":");
        case '!': return makeToken(ptxc::TokenKind::Bang,      "!");
        case '|': return makeToken(ptxc::TokenKind::Pipe,      "|");
        case '@': return makeToken(ptxc::TokenKind::At,        "@");
    }
    std::string msg = "unexpected character '"; msg += c; msg += "'";
    errors_.push_back(filename_ + ":" + std::to_string(tokenLine_) + ":" + std::to_string(tokenCol_) + ": error: " + msg);
    return errorToken(msg);
}

bool Lexer::isAtEnd() const { return pos_ >= source_.size(); }
char Lexer::peek() const { return isAtEnd() ? '\0' : source_[pos_]; }
char Lexer::peekNext() const { return (pos_ + 1 >= source_.size()) ? '\0' : source_[pos_ + 1]; }
char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') { line_++; col_ = 1; } else { col_++; }
    return c;
}
bool Lexer::match(char expected) { if (isAtEnd() || source_[pos_] != expected) return false; advance(); return true; }
void Lexer::skipWhitespace() { while (!isAtEnd() && std::isspace(source_[pos_])) advance(); }
void Lexer::skipLineComment() { advance(); advance(); while (!isAtEnd() && peek() != '\n') advance(); }
void Lexer::skipBlockComment() { advance(); advance(); 
    while (!isAtEnd()) { 
        if (peek() == '*' && peekNext() == '/') { advance(); advance(); return; } 
        advance(); 
    } 
    errors_.push_back(filename_ + ":" + std::to_string(tokenLine_) + ":" + std::to_string(tokenCol_) + ": error: unterminated block comment");
}

ptxc::Token Lexer::makeToken(ptxc::TokenKind kind, const std::string& lexeme) const { return ptxc::Token{kind, lexeme, {filename_, tokenLine_, tokenCol_}}; }
ptxc::Token Lexer::makeToken(ptxc::TokenKind kind, const std::string& lexeme, const ptxc::SourceLocation& loc) const { return ptxc::Token{kind, lexeme, loc}; }
ptxc::Token Lexer::errorToken(const std::string& message) const { return ptxc::Token{ptxc::TokenKind::Error, message, {filename_, tokenLine_, tokenCol_}}; }

ptxc::Token Lexer::lexNumber() {
    std::size_t start = pos_;
    if (peek() == '-') advance();
    if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X')) { advance(); advance(); while (!isAtEnd() && std::isxdigit(peek())) advance(); return makeToken(ptxc::TokenKind::IntegerLiteral, source_.substr(start, pos_ - start)); }
    if (peek() == '0' && (peekNext() == 'b' || peekNext() == 'B')) { advance(); advance(); while (!isAtEnd() && (peek() == '0' || peek() == '1')) advance(); return makeToken(ptxc::TokenKind::IntegerLiteral, source_.substr(start, pos_ - start)); }
    if (peek() == '0' && (peekNext() == 'f' || peekNext() == 'F')) { advance(); advance(); while (!isAtEnd() && std::isxdigit(peek())) advance(); return makeToken(ptxc::TokenKind::FloatLiteral, source_.substr(start, pos_ - start)); }
    if (peek() == '0' && (peekNext() == 'd' || peekNext() == 'D')) { advance(); advance(); while (!isAtEnd() && std::isxdigit(peek())) advance(); return makeToken(ptxc::TokenKind::FloatLiteral, source_.substr(start, pos_ - start)); }
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '.' || (peek() == '-' && (source_[pos_-1] == 'e' || source_[pos_-1] == 'E')) || (peek() == '+' && (source_[pos_-1] == 'e' || source_[pos_-1] == 'E')))) { advance(); }
    std::string lexeme = source_.substr(start, pos_ - start);
    if (lexeme.find('.') != std::string::npos || lexeme.find('e') != std::string::npos || lexeme.find('E') != std::string::npos) return makeToken(ptxc::TokenKind::FloatLiteral, lexeme);
    return makeToken(ptxc::TokenKind::IntegerLiteral, lexeme);
}

ptxc::Token Lexer::lexDotKeyword() {
    std::size_t start = pos_; advance(); while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) advance();
    std::string word = source_.substr(start, pos_ - start);
    return makeToken(lookupDotKeyword(word), word);
}

ptxc::Token Lexer::lexPercentRegister() {
    std::size_t start = pos_; advance(); while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_' || peek() == '.')) advance();
    std::string lexeme = source_.substr(start, pos_ - start);
    if (lexeme.find("%tid") == 0 || lexeme.find("%ntid") == 0 || lexeme.find("%ctaid") == 0 || lexeme.find("%nctaid") == 0 || lexeme.find("%laneid") == 0) {
        return makeToken(ptxc::TokenKind::SpecialReg, lexeme);
    }
    return makeToken(ptxc::TokenKind::Register, lexeme);
}

ptxc::Token Lexer::lexIdentifierOrOpcode() {
    std::size_t start = pos_; while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_' || peek() == '$')) advance();
    std::string word = source_.substr(start, pos_ - start);
    if (!isAtEnd() && peek() == ':') { advance(); return makeToken(ptxc::TokenKind::Label, word); }
    return makeToken(lookupOpcode(word), word);
}

ptxc::Token Lexer::lexStringLiteral() {
    advance(); std::size_t start = pos_; while (!isAtEnd() && peek() != '"') { if (peek() == '\\') advance(); advance(); }
    std::string val = source_.substr(start, pos_ - start); if (!isAtEnd()) advance();
    return makeToken(ptxc::TokenKind::Identifier, val);
}

ptxc::TokenKind Lexer::lookupDotKeyword(const std::string& word) {
    static const std::unordered_map<std::string, ptxc::TokenKind> table = {
        {".version", ptxc::TokenKind::DotVersion}, {".target", ptxc::TokenKind::DotTarget}, 
        {".address_size", ptxc::TokenKind::DotAddressSize}, {".reg", ptxc::TokenKind::DotReg}, 
        {".param", ptxc::TokenKind::DotParam}, {".func", ptxc::TokenKind::DotFunc}, 
        {".entry", ptxc::TokenKind::DotEntry}, {".visible", ptxc::TokenKind::DotVisible}, 
        {".extern", ptxc::TokenKind::DotExtern}, {".local", ptxc::TokenKind::DotLocal},
        {".shared", ptxc::TokenKind::DotShared}, {".global", ptxc::TokenKind::DotGlobal},
        {".const", ptxc::TokenKind::DotConst}, {".align", ptxc::TokenKind::DotAlign},
        {".f32", ptxc::TokenKind::DotF32}, {".f64", ptxc::TokenKind::DotF64}, 
        {".s32", ptxc::TokenKind::DotS32}, {".s64", ptxc::TokenKind::DotS64}, 
        {".u32", ptxc::TokenKind::DotU32}, {".u64", ptxc::TokenKind::DotU64}, 
        {".u16", ptxc::TokenKind::DotU16}, {".s16", ptxc::TokenKind::DotS16},
        {".b32", ptxc::TokenKind::DotB32}, {".b64", ptxc::TokenKind::DotB64},
        {".pred", ptxc::TokenKind::DotPred}, 
        {".lo", ptxc::TokenKind::DotLo}, {".hi", ptxc::TokenKind::DotHi},
        {".wide", ptxc::TokenKind::DotWide}, {".rn", ptxc::TokenKind::DotRn},
        {".rz", ptxc::TokenKind::DotRz}, {".rm", ptxc::TokenKind::DotRm},
        {".rp", ptxc::TokenKind::DotRp}, {".sync", ptxc::TokenKind::DotSync}, 
        {".eq", ptxc::TokenKind::DotEq}, {".ne", ptxc::TokenKind::DotNe}, 
        {".lt", ptxc::TokenKind::DotLt}, {".le", ptxc::TokenKind::DotLe}, 
        {".gt", ptxc::TokenKind::DotGt}, {".ge", ptxc::TokenKind::DotGe},
        {".ls", ptxc::TokenKind::DotLs}, {".hs", ptxc::TokenKind::DotHs},
        {".equ", ptxc::TokenKind::DotEqu}, {".neu", ptxc::TokenKind::DotNeu},
        {".ltu", ptxc::TokenKind::DotLtu}, {".leu", ptxc::TokenKind::DotLeu},
        {".gtu", ptxc::TokenKind::DotGtu}, {".geu", ptxc::TokenKind::DotGeu}
    };
    auto it = table.find(word); return (it != table.end()) ? it->second : ptxc::TokenKind::Identifier;
}

ptxc::TokenKind Lexer::lookupOpcode(const std::string& word) {
    static const std::unordered_map<std::string, ptxc::TokenKind> table = {
        {"add", ptxc::TokenKind::OpAdd}, {"sub", ptxc::TokenKind::OpSub}, 
        {"mul", ptxc::TokenKind::OpMul}, {"mad", ptxc::TokenKind::OpMad},
        {"fma", ptxc::TokenKind::OpFma}, {"ld", ptxc::TokenKind::OpLd}, 
        {"st", ptxc::TokenKind::OpSt}, {"mov", ptxc::TokenKind::OpMov}, 
        {"cvt", ptxc::TokenKind::OpCvt}, {"bra", ptxc::TokenKind::OpBra},
        {"call", ptxc::TokenKind::OpCall}, {"ret", ptxc::TokenKind::OpRet}, 
        {"setp", ptxc::TokenKind::OpSetp}, {"selp", ptxc::TokenKind::OpSelp},
        {"shl", ptxc::TokenKind::OpShl}, {"shr", ptxc::TokenKind::OpShr},
        {"and", ptxc::TokenKind::OpAnd}, {"or", ptxc::TokenKind::OpOr},
        {"xor", ptxc::TokenKind::OpXor}, {"not", ptxc::TokenKind::OpNot},
        {"bar", ptxc::TokenKind::OpBar}, {"exit", ptxc::TokenKind::OpExit},
        {"min", ptxc::TokenKind::OpMin}, {"max", ptxc::TokenKind::OpMax},
        {"abs", ptxc::TokenKind::OpAbs}, {"neg", ptxc::TokenKind::OpNeg},
        {"div", ptxc::TokenKind::OpDiv}, {"rem", ptxc::TokenKind::OpRem}
    };
    auto it = table.find(word); return (it != table.end()) ? it->second : ptxc::TokenKind::Identifier;
}

} // namespace ptxc
