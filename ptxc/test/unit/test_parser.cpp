#include "SimpleTest.h"
#include "ptxc/lexer/Lexer.h"
#include "ptxc/parser/Parser.h"
#include <iostream>
#include <fstream>

using namespace ptxc;

TEST(ParserTest, EmptyModule) {
    Lexer lex("", "empty.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    EXPECT_TRUE(module != nullptr);
    EXPECT_TRUE(module->functions.empty());
    EXPECT_FALSE(parser.hasErrors());
}

TEST(ParserTest, GlobalDirectives) {
    std::string src = ".version 8.0\n.target sm_80\n.address_size 64\n";
    Lexer lex(src, "global.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    EXPECT_FALSE(parser.hasErrors());
    EXPECT_EQ(module->version, "8.0");
    EXPECT_EQ(module->target, "sm_80");
    EXPECT_EQ(module->addressSize, 64);
}

TEST(ParserTest, SimpleFunction) {
    std::string src = ".visible .entry my_kernel() { ret; }";
    Lexer lex(src, "simple.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    if (parser.hasErrors() || module->functions.empty() || module->functions[0]->name != "my_kernel") {
        std::ofstream log("token_dump.txt");
        log << "Parser errors:" << std::endl;
        for (const auto& err : parser.errors()) log << "  " << err << std::endl;
        log << "Tokens:" << std::endl;
        Lexer lex2(src, "dump.ptx");
        for (const auto& t : lex2.tokenize()) {
            log << "  " << tokenKindName(t.kind) << " '" << t.lexeme << "'" << std::endl;
        }
        log.close();
    }

    EXPECT_FALSE(parser.hasErrors());
    ASSERT_GE(module->functions.size(), 1u);
    
    if (!module->functions.empty()) {
        auto& func = module->functions[0];
        EXPECT_EQ(func->name, "my_kernel");
        EXPECT_TRUE(func->isEntry);
        EXPECT_TRUE(func->isVisible);
        ASSERT_GE(func->body.size(), 1u);
        
        if (!func->body.empty()) {
            auto inst = dynamic_cast<InstructionStmtNode*>(func->body[0].get());
            EXPECT_TRUE(inst != nullptr);
            if (inst) EXPECT_EQ(inst->opcode, "ret");
        }
    }
}

TEST(ParserTest, InstructionWithoperandList) {
    std::string src = ".entry k() { add.s32 %r1, %r2, 42; }";
    Lexer lex(src, "inst.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    EXPECT_FALSE(parser.hasErrors());
    if (module->functions.empty()) return;
    auto& func = module->functions[0];
    if (func->body.empty()) return;
    auto inst = dynamic_cast<InstructionStmtNode*>(func->body[0].get());
    if (!inst) return;
    
    EXPECT_EQ(inst->opcode, "add.s32");
    EXPECT_GE(inst->operandList.size(), 3u);
}

TEST(ParserTest, PredicatedInstruction) {
    std::string src = ".entry k() { @%p0 bra label; }";
    Lexer lex(src, "pred.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    EXPECT_FALSE(parser.hasErrors());
    if (!module->functions.empty() && !module->functions[0]->body.empty()) {
        auto inst = dynamic_cast<InstructionStmtNode*>(module->functions[0]->body[0].get());
        if (inst) {
            EXPECT_EQ(inst->predicateReg, "%p0");
            EXPECT_FALSE(inst->predicateNegated);
            EXPECT_EQ(inst->opcode, "bra");
        }
    }
}

TEST(ParserTest, AddressingModes) {
    std::string src = ".entry k() { ld.global.f32 %f0, [%rd0+16]; }";
    Lexer lex(src, "addr.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    if (!module->functions.empty() && !module->functions[0]->body.empty()) {
        auto inst = dynamic_cast<InstructionStmtNode*>(module->functions[0]->body[0].get());
        if (inst && inst->operandList.size() >= 2) {
            auto addr = dynamic_cast<AddressOpNode*>(inst->operandList[1].get());
            EXPECT_TRUE(addr != nullptr);
            if (addr) {
                EXPECT_EQ(addr->baseReg, "%rd0");
                EXPECT_EQ(addr->offset, 16);
            }
        }
    }
}

TEST(ParserTest, RegisterDeclaration) {
    std::string src = ".entry k() { .reg .f32 %f<16>; }";
    Lexer lex(src, "decl.ptx");
    Parser parser(lex);
    auto module = parser.parseModule();
    
    if (!module->functions.empty() && !module->functions[0]->body.empty()) {
        auto decl = dynamic_cast<DirectiveStmtNode*>(module->functions[0]->body[0].get());
        EXPECT_TRUE(decl != nullptr);
        if (decl) {
            EXPECT_EQ(decl->directive, ".reg");
            EXPECT_EQ(decl->type, ".f32");
        }
    }
}
