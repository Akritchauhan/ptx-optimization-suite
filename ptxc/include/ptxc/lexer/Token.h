#ifndef PTXC_LEXER_TOKEN_H
#define PTXC_LEXER_TOKEN_H

#include <cstdint>
#include <string>
#include <ostream>

namespace ptxc {

struct SourceLocation {
    std::string file;
    int line = 1;
    int col  = 1;
    std::string toString() const { return file + ":" + std::to_string(line) + ":" + std::to_string(col); }
};

enum class TokenKind : uint8_t {
    Eof, Error,
    DotVersion, DotTarget, DotAddressSize, DotReg, DotParam, DotFunc, DotEntry, DotVisible, DotExtern, DotLocal, DotShared, DotGlobal, DotConst, DotAlign,
    DotF32, DotF64, DotS32, DotS64, DotU32, DotU64, DotU16, DotS16, DotB32, DotB64, DotPred,
    DotLo, DotHi, DotWide, DotRn, DotRz, DotRm, DotRp,
    DotSync,
    OpAdd, OpSub, OpMul, OpMad, OpFma, OpLd, OpSt, OpMov, OpCvt, OpBra, OpCall, OpRet, OpSetp, OpSelp, OpShl, OpShr, OpAnd, OpOr, OpXor, OpNot, OpBar, OpExit, OpMin, OpMax, OpAbs, OpNeg, OpRem, OpDiv,
    DotEq, DotNe, DotLt, DotLe, DotGt, DotGe, DotLs, DotHs, DotEqu, DotNeu, DotLtu, DotLeu, DotGtu, DotGeu,
    Register, SpecialReg,
    IntegerLiteral, FloatLiteral,
    Identifier, Label,
    At, LBrace, RBrace, LParen, RParen, LBracket, RBracket, LAngle, RAngle, Comma, Semicolon, Plus, Minus, Colon, Dot, Bang, Pipe,
    _Count
};

const char* tokenKindName(TokenKind kind);

inline std::ostream& operator<<(std::ostream& os, TokenKind kind) {
    return os << tokenKindName(kind);
}

struct Token {
    TokenKind kind = TokenKind::Eof;
    std::string lexeme;
    SourceLocation loc;
    bool is(TokenKind k) const { return kind == k; }
    bool isNot(TokenKind k) const { return kind != k; }
    template <typename... Kinds> bool isOneOf(Kinds... kinds) const { return (is(kinds) || ...); }
    bool isInstruction() const;
    bool isDirective() const;
    bool isModifier() const;
    bool isTypeModifier() const;
    friend std::ostream& operator<<(std::ostream& os, const Token& tok);
};

} // namespace ptxc

#endif // PTXC_LEXER_TOKEN_H
