#include "ptxc/lexer/Token.h"

namespace ptxc {

const char* tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::Eof: return "Eof"; case TokenKind::Error: return "Error";
        case TokenKind::DotVersion: return ".version"; case TokenKind::DotTarget: return ".target"; case TokenKind::DotAddressSize: return ".address_size";
        case TokenKind::DotReg: return ".reg"; case TokenKind::DotParam: return ".param"; case TokenKind::DotFunc: return ".func";
        case TokenKind::DotEntry: return ".entry"; case TokenKind::DotVisible: return ".visible"; case TokenKind::DotExtern: return ".extern";
        case TokenKind::DotF32: return ".f32"; case TokenKind::DotF64: return ".f64"; case TokenKind::DotS32: return ".s32";
        case TokenKind::DotU32: return ".u32"; case TokenKind::DotU64: return ".u64"; case TokenKind::DotPred: return ".pred";
        case TokenKind::DotLo: return ".lo"; case TokenKind::DotHi: return ".hi"; case TokenKind::DotWide: return ".wide";
        case TokenKind::DotRn: return ".rn"; case TokenKind::DotRz: return ".rz"; case TokenKind::DotRm: return ".rm"; case TokenKind::DotRp: return ".rp";
        case TokenKind::DotSync: return ".sync";
        case TokenKind::DotEq: return ".eq"; case TokenKind::DotNe: return ".ne"; case TokenKind::DotLt: return ".lt";
        case TokenKind::DotLe: return ".le"; case TokenKind::DotGt: return ".gt"; case TokenKind::DotGe: return ".ge";
        case TokenKind::OpAdd: return "add"; case TokenKind::OpSub: return "sub"; case TokenKind::OpMul: return "mul";
        case TokenKind::OpMad: return "mad"; case TokenKind::OpLd: return "ld"; case TokenKind::OpSt: return "st";
        case TokenKind::OpMov: return "mov"; case TokenKind::OpBra: return "bra"; case TokenKind::OpCall: return "call";
        case TokenKind::OpRet: return "ret"; case TokenKind::OpSetp: return "setp"; case TokenKind::OpExit: return "exit";
        case TokenKind::Register: return "Register"; case TokenKind::SpecialReg: return "SpecialReg";
        case TokenKind::IntegerLiteral: return "IntegerLiteral"; case TokenKind::FloatLiteral: return "FloatLiteral";
        case TokenKind::Identifier: return "Identifier"; case TokenKind::Label: return "Label";
        case TokenKind::At: return "@"; case TokenKind::LBrace: return "{"; case TokenKind::RBrace: return "}";
        case TokenKind::LParen: return "("; case TokenKind::RParen: return ")"; case TokenKind::LBracket: return "[";
        case TokenKind::RBracket: return "]"; case TokenKind::LAngle: return "<"; case TokenKind::RAngle: return ">";
        case TokenKind::Comma: return ","; case TokenKind::Semicolon: return ";"; case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-"; case TokenKind::Colon: return ":"; case TokenKind::Dot: return ".";
        case TokenKind::Bang: return "!"; case TokenKind::Pipe: return "|";
        default: return "unknown";
    }
}

bool Token::isInstruction() const { return kind >= TokenKind::OpAdd && kind <= TokenKind::OpDiv; }
bool Token::isDirective() const { return kind >= TokenKind::DotVersion && kind <= TokenKind::DotAlign; }
bool Token::isModifier() const {
    return (kind >= TokenKind::DotF32 && kind <= TokenKind::DotRp) ||
           (kind >= TokenKind::DotEq && kind <= TokenKind::DotGeu) ||
           kind == TokenKind::DotSync;
}
bool Token::isTypeModifier() const { return kind >= TokenKind::DotF32 && kind <= TokenKind::DotPred; }

std::ostream& operator<<(std::ostream& os, const Token& tok) {
    os << tok.loc.toString() << ": " << tokenKindName(tok.kind) << " '" << tok.lexeme << "'";
    return os;
}

} // namespace ptxc
