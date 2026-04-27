#include "ptxc/lexer/Token.h"
#include <iostream>

using namespace ptxc;

int main() {
    std::cout << "Eof: " << (int)TokenKind::Eof << std::endl;
    std::cout << ".f32: " << (int)TokenKind::DotF32 << std::endl;
    std::cout << ".rp: " << (int)TokenKind::DotRp << std::endl;
    std::cout << ".sync: " << (int)TokenKind::DotSync << std::endl;
    std::cout << "add: " << (int)TokenKind::OpAdd << std::endl;
    std::cout << ".eq: " << (int)TokenKind::DotEq << std::endl;
    std::cout << ".geu: " << (int)TokenKind::DotGeu << std::endl;
    std::cout << "Register: " << (int)TokenKind::Register << std::endl;
    std::cout << "Semicolon: " << (int)TokenKind::Semicolon << std::endl;
    std::cout << "RBrace: " << (int)TokenKind::RBrace << std::endl;
    
    std::cout << "isModifier(.f32): " << Token{TokenKind::DotF32}.isModifier() << std::endl;
    std::cout << "isModifier(.rp): " << Token{TokenKind::DotRp}.isModifier() << std::endl;
    std::cout << "isModifier(.eq): " << Token{TokenKind::DotEq}.isModifier() << std::endl;
    std::cout << "isModifier(.geu): " << Token{TokenKind::DotGeu}.isModifier() << std::endl;
    std::cout << "isModifier(.sync): " << Token{TokenKind::DotSync}.isModifier() << std::endl;
    std::cout << "isModifier(add): " << Token{TokenKind::OpAdd}.isModifier() << std::endl;
    std::cout << "isModifier(Register): " << Token{TokenKind::Register}.isModifier() << std::endl;
    std::cout << "isModifier(Semicolon): " << Token{TokenKind::Semicolon}.isModifier() << std::endl;
    
    return 0;
}
