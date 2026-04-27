#include "ptxc/lexer/Lexer.h"
#include "ptxc/parser/Parser.h"
#include <iostream>

using namespace ptxc;

void runTest(const std::string& src) {
    std::cout << "\nSource: " << src << std::endl;

    Lexer lex1(src, "debug.ptx");
    std::cout << "Tokens:" << std::endl;
    for (const auto& t : lex1.tokenize()) {
        std::cout << "  " << tokenKindName(t.kind) << " '" << t.lexeme << "'" << std::endl;
    }

    Lexer lex2(src, "debug.ptx");
    Parser parser(lex2);
    auto module = parser.parseModule();

    if (parser.hasErrors()) {
        std::cout << "Parser Errors:" << std::endl;
        for (const auto& err : parser.errors()) {
            std::cout << "  " << err << std::endl;
        }
    }

    if (!module->functions.empty()) {
        auto& func = module->functions[0];
        std::cout << "Function: '" << func->name << "'" << std::endl;
        for (const auto& stmt : func->body) {
            if (auto inst = dynamic_cast<InstructionStmtNode*>(stmt.get())) {
                std::cout << "  Inst: opcode='" << inst->opcode << "' operands=" << inst->operands.size() << std::endl;
                for (size_t i = 0; i < inst->operands.size(); ++i) {
                     if (auto reg = dynamic_cast<RegisterOpNode*>(inst->operands[i].get()))
                         std::cout << "    Op[" << i << "]: Register '" << reg->name << "'" << std::endl;
                     else if (auto imm = dynamic_cast<ImmediateOpNode*>(inst->operands[i].get()))
                         std::cout << "    Op[" << i << "]: Immediate '" << imm->value << "'" << std::endl;
                }
            }
        }
    } else {
        std::cout << "No functions parsed." << std::endl;
    }
}

int main() {
    runTest(".visible .entry my_kernel() { ret; }");
    runTest(".entry k() { add.s32 %r1, %r2, 42; }");
    return 0;
}
