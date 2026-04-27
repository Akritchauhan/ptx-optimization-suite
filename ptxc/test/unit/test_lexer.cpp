#include "SimpleTest.h"
#include "ptxc/lexer/Lexer.h"
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>

using namespace ptxc;

// ═══════════════════════════════════════════════════════════════════
// Helper: read a file into a string
// ═══════════════════════════════════════════════════════════════════
static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// ═══════════════════════════════════════════════════════════════════
// Basic token tests
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, EmptyInput) {
    Lexer lex("", "empty.ptx");
    auto tok = lex.nextToken();
    EXPECT_EQ(tok.kind, ptxc::TokenKind::Eof);
}

TEST(LexerTest, WhitespaceOnly) {
    Lexer lex("   \t  \n  \r\n  ", "ws.ptx");
    auto tok = lex.nextToken();
    EXPECT_EQ(tok.kind, ptxc::TokenKind::Eof);
}

TEST(LexerTest, LineComment) {
    Lexer lex("// this is a comment\n.version", "comment.ptx");
    auto tok = lex.nextToken();
    EXPECT_EQ(tok.kind, ptxc::TokenKind::DotVersion);
    EXPECT_EQ(tok.lexeme, ".version");
}

TEST(LexerTest, BlockComment) {
    Lexer lex("/* block\n   comment */ .target", "block.ptx");
    auto tok = lex.nextToken();
    EXPECT_EQ(tok.kind, ptxc::TokenKind::DotTarget);
}

TEST(LexerTest, UnterminatedBlockComment) {
    Lexer lex("/* unterminated", "bad.ptx");
    auto tok = lex.nextToken();
    EXPECT_EQ(tok.kind, ptxc::TokenKind::Eof);
    EXPECT_FALSE(lex.errors().empty());
}

// ═══════════════════════════════════════════════════════════════════
// Directive tokens
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, Directives) {
    Lexer lex(".version .target .address_size .reg .param .func .entry .visible .extern",
              "dirs.ptx");
    auto tokens = lex.tokenize();

    ASSERT_GE(tokens.size(), 10u); // 9 directives + Eof
    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::DotVersion);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotTarget);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotAddressSize);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::DotReg);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::DotParam);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::DotFunc);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::DotEntry);
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::DotVisible);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::DotExtern);
    EXPECT_EQ(tokens[9].kind, ptxc::TokenKind::Eof);
}

// ═══════════════════════════════════════════════════════════════════
// Type modifier tokens
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, TypeModifiers) {
    Lexer lex(".f32 .f64 .s32 .u32 .u64 .pred", "types.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::DotF32);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotF64);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotS32);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::DotU32);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::DotU64);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::DotPred);
}

// ═══════════════════════════════════════════════════════════════════
// Rounding and comparison modifiers
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, RoundingModifiers) {
    Lexer lex(".lo .hi .rn .rz .rm .rp .wide", "rounding.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::DotLo);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotHi);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotRn);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::DotRz);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::DotRm);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::DotRp);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::DotWide);
}

TEST(LexerTest, ComparisonModifiers) {
    Lexer lex(".eq .ne .lt .le .gt .ge", "cmp.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::DotEq);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotNe);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotLt);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::DotLe);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::DotGt);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::DotGe);
}

// ═══════════════════════════════════════════════════════════════════
// Instruction opcode tokens
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, Opcodes) {
    Lexer lex("add sub mul mad fma ld st mov cvt bra call ret setp selp",
              "ops.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::OpAdd);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::OpSub);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::OpMul);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::OpMad);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::OpFma);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::OpLd);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::OpSt);
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::OpMov);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::OpCvt);
    EXPECT_EQ(tokens[9].kind, ptxc::TokenKind::OpBra);
    EXPECT_EQ(tokens[10].kind, ptxc::TokenKind::OpCall);
    EXPECT_EQ(tokens[11].kind, ptxc::TokenKind::OpRet);
    EXPECT_EQ(tokens[12].kind, ptxc::TokenKind::OpSetp);
    EXPECT_EQ(tokens[13].kind, ptxc::TokenKind::OpSelp);
}

TEST(LexerTest, AdditionalOpcodes) {
    Lexer lex("shl shr and or xor not bar exit div rem", "ops2.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::OpShl);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::OpShr);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::OpAnd);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::OpOr);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::OpXor);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::OpNot);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::OpBar);
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::OpExit);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::OpDiv);
    EXPECT_EQ(tokens[9].kind, ptxc::TokenKind::OpRem);
}

// ═══════════════════════════════════════════════════════════════════
// Register tokens
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, GeneralRegisters) {
    Lexer lex("%r0 %r999 %rd0 %rd15 %f0 %f7 %p0 %p3", "regs.ptx");
    auto tokens = lex.tokenize();

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(tokens[i].kind, ptxc::TokenKind::Register);
    }
    EXPECT_EQ(tokens[0].lexeme, "%r0");
    EXPECT_EQ(tokens[1].lexeme, "%r999");
    EXPECT_EQ(tokens[2].lexeme, "%rd0");
    EXPECT_EQ(tokens[3].lexeme, "%rd15");
    EXPECT_EQ(tokens[4].lexeme, "%f0");
    EXPECT_EQ(tokens[5].lexeme, "%f7");
    EXPECT_EQ(tokens[6].lexeme, "%p0");
    EXPECT_EQ(tokens[7].lexeme, "%p3");
}

TEST(LexerTest, SpecialRegisters) {
    Lexer lex("%tid.x %ctaid.y %ntid.z %nctaid.x %laneid", "specregs.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::SpecialReg);
    EXPECT_EQ(tokens[0].lexeme, "%tid.x");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::SpecialReg);
    EXPECT_EQ(tokens[1].lexeme, "%ctaid.y");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::SpecialReg);
    EXPECT_EQ(tokens[2].lexeme, "%ntid.z");
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::SpecialReg);
    EXPECT_EQ(tokens[3].lexeme, "%nctaid.x");
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::SpecialReg);
    EXPECT_EQ(tokens[4].lexeme, "%laneid");
}

// ═══════════════════════════════════════════════════════════════════
// Literal tokens
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, IntegerLiterals) {
    Lexer lex("0 42 0xFF 0b1010 1024", "ints.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[0].lexeme, "0");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[1].lexeme, "42");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[2].lexeme, "0xFF");
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[3].lexeme, "0b1010");
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[4].lexeme, "1024");
}

TEST(LexerTest, FloatLiterals) {
    Lexer lex("3.14 1.0e-5 0F3f800000 0D3ff0000000000000", "floats.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[0].lexeme, "3.14");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[1].lexeme, "1.0e-5");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[2].lexeme, "0F3f800000");
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[3].lexeme, "0D3ff0000000000000");
}

TEST(LexerTest, NegativeNumbers) {
    Lexer lex("-1 -42 -3.14", "neg.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[0].lexeme, "-1");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[1].lexeme, "-42");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[2].lexeme, "-3.14");
}

// ═══════════════════════════════════════════════════════════════════
// Labels
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, Labels) {
    Lexer lex("loop_start: done:", "labels.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::Label);
    EXPECT_EQ(tokens[0].lexeme, "loop_start");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::Label);
    EXPECT_EQ(tokens[1].lexeme, "done");
}

// ═══════════════════════════════════════════════════════════════════
// Punctuation
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, Punctuation) {
    Lexer lex("{ } ( ) , ; [ ] < > + @", "punct.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::LBrace);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::RBrace);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::LParen);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::RParen);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::Comma);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::Semicolon);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::LBracket);
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::RBracket);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::LAngle);
    EXPECT_EQ(tokens[9].kind, ptxc::TokenKind::RAngle);
    EXPECT_EQ(tokens[10].kind, ptxc::TokenKind::Plus);
    EXPECT_EQ(tokens[11].kind, ptxc::TokenKind::At);
}

// ═══════════════════════════════════════════════════════════════════
// Source location tracking
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, SourceLocation) {
    Lexer lex(".version\n  add", "loc.ptx");
    auto tokens = lex.tokenize();

    // .version at line 1, col 1
    EXPECT_EQ(tokens[0].loc.line, 1);
    EXPECT_EQ(tokens[0].loc.col, 1);
    // add at line 2, col 3 (two spaces indentation)
    EXPECT_EQ(tokens[1].loc.line, 2);
    EXPECT_EQ(tokens[1].loc.col, 3);
}

// ═══════════════════════════════════════════════════════════════════
// Predicated instruction sequence
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, PredicatedInstruction) {
    Lexer lex("@%p0 bra done;", "pred.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::At);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[1].lexeme, "%p0");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::OpBra);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::Identifier);
    EXPECT_EQ(tokens[3].lexeme, "done");
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::Semicolon);
}

// ═══════════════════════════════════════════════════════════════════
// Complex instruction line
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, ComplexInstruction) {
    Lexer lex("fma.rn.f32 %f3, %f0, %f1, %f2;", "complex.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::OpFma);
    EXPECT_EQ(tokens[0].lexeme, "fma");
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotRn);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotF32);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[3].lexeme, "%f3");
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::Comma);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::Comma);
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::Comma);
    EXPECT_EQ(tokens[9].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[10].kind, ptxc::TokenKind::Semicolon);
}

// ═══════════════════════════════════════════════════════════════════
// Address expression: [%rd0+16]
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, AddressExpression) {
    Lexer lex("ld.global.f32 %f1, [%rd3];", "addr.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::OpLd);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotGlobal);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::DotF32);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::Comma);
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::LBracket);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[6].lexeme, "%rd3");
    EXPECT_EQ(tokens[7].kind, ptxc::TokenKind::RBracket);
    EXPECT_EQ(tokens[8].kind, ptxc::TokenKind::Semicolon);
}

TEST(LexerTest, AddressWithOffset) {
    Lexer lex("[%rd0+16]", "offset.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::LBracket);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[1].lexeme, "%rd0");
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::Plus);
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[3].lexeme, "16");
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::RBracket);
}

// ═══════════════════════════════════════════════════════════════════
// Register declaration with angle brackets
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, RegDeclaration) {
    Lexer lex(".reg .f32 %f<16>;", "regdecl.ptx");
    auto tokens = lex.tokenize();

    EXPECT_EQ(tokens[0].kind, ptxc::TokenKind::DotReg);
    EXPECT_EQ(tokens[1].kind, ptxc::TokenKind::DotF32);
    EXPECT_EQ(tokens[2].kind, ptxc::TokenKind::Register);
    EXPECT_EQ(tokens[2].lexeme, "%f");
    EXPECT_EQ(tokens[3].kind, ptxc::TokenKind::LAngle);
    EXPECT_EQ(tokens[4].kind, ptxc::TokenKind::IntegerLiteral);
    EXPECT_EQ(tokens[4].lexeme, "16");
    EXPECT_EQ(tokens[5].kind, ptxc::TokenKind::RAngle);
    EXPECT_EQ(tokens[6].kind, ptxc::TokenKind::Semicolon);
}

// ═══════════════════════════════════════════════════════════════════
// Token convenience methods
// ═══════════════════════════════════════════════════════════════════

TEST(TokenTest, ConvenienceMethods) {
    Token tok{ptxc::TokenKind::OpAdd, "add", {"test.ptx", 1, 1}};
    EXPECT_TRUE(tok.is(ptxc::TokenKind::OpAdd));
    EXPECT_TRUE(tok.isNot(ptxc::TokenKind::OpSub));
    EXPECT_TRUE(tok.isOneOf(ptxc::TokenKind::OpAdd, ptxc::TokenKind::OpSub));
    EXPECT_TRUE(tok.isInstruction());
    EXPECT_FALSE(tok.isTypeModifier());
}

TEST(TokenTest, TypeModifierCheck) {
    Token tok{ptxc::TokenKind::DotF32, ".f32", {"test.ptx", 1, 1}};
    EXPECT_TRUE(tok.isTypeModifier());
    EXPECT_FALSE(tok.isInstruction());
}

// ═══════════════════════════════════════════════════════════════════
// Full fixture file tokenization (smoke test)
// ═══════════════════════════════════════════════════════════════════

TEST(LexerTest, TokenizeSaxpyFixture) {
    std::string src = readFile("test/fixtures/saxpy.ptx");
    if (src.empty()) {
        std::cerr << "saxpy.ptx fixture not found" << std::endl;
        GTEST_SKIP();
    }
    Lexer lex(src, "saxpy.ptx");
    auto tokens = lex.tokenize();

    EXPECT_TRUE(lex.errors().empty());
    // Must have many tokens and end with Eof
    EXPECT_GT(tokens.size(), 50u);
    EXPECT_EQ(tokens.back().kind, ptxc::TokenKind::Eof);

    // Check that we got key tokens from saxpy
    bool hasEntry = false, hasFma = false, hasRet = false;
    for (const auto& t : tokens) {
        if (t.kind == ptxc::TokenKind::DotEntry) hasEntry = true;
        if (t.kind == ptxc::TokenKind::OpFma) hasFma = true;
        if (t.kind == ptxc::TokenKind::OpRet) hasRet = true;
    }
    EXPECT_TRUE(hasEntry);
    EXPECT_TRUE(hasFma);
    EXPECT_TRUE(hasRet);
}

TEST(LexerTest, TokenizeGemmFixture) {
    std::string src = readFile("test/fixtures/gemm_inner.ptx");
    if (src.empty()) {
        std::cerr << "gemm_inner.ptx fixture not found" << std::endl;
        GTEST_SKIP();
    }
    Lexer lex(src, "gemm_inner.ptx");
    auto tokens = lex.tokenize();

    EXPECT_TRUE(lex.errors().empty());
    EXPECT_GT(tokens.size(), 80u);
    EXPECT_EQ(tokens.back().kind, ptxc::TokenKind::Eof);
}

TEST(LexerTest, TokenizeBranchHeavyFixture) {
    std::string src = readFile("test/fixtures/branch_heavy.ptx");
    if (src.empty()) {
        std::cerr << "branch_heavy.ptx fixture not found" << std::endl;
        GTEST_SKIP();
    }
    Lexer lex(src, "branch_heavy.ptx");
    auto tokens = lex.tokenize();

    EXPECT_TRUE(lex.errors().empty());
    EXPECT_GT(tokens.size(), 60u);
    EXPECT_EQ(tokens.back().kind, ptxc::TokenKind::Eof);
}
