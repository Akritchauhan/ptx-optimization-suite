#ifndef PTXC_PARSER_AST_H
#define PTXC_PARSER_AST_H

#include "ptxc/lexer/Token.h"
#include <string>
#include <vector>
#include <memory>

namespace ptxc {

// Forward declarations
struct ASTNode;
struct ModuleNode;
struct FunctionNode;
struct StatementNode;
struct InstructionNode;
struct OperandNode;

/// @brief Base class for all AST nodes.
struct ASTNode {
    SourceLocation loc;
    virtual ~ASTNode() = default;
};

// ── Operands ───────────────────────────────────────────────────────

enum class OperandKind {
    Register,
    Immediate,
    Label,
    Address
};

struct OperandNode : public ASTNode {
    virtual OperandKind getKind() const = 0;
};

struct RegisterOpNode : public OperandNode {
    std::string name;
    OperandKind getKind() const override { return OperandKind::Register; }
};

struct ImmediateOpNode : public OperandNode {
    std::string value; // Store as string for flexibility (hex/float/int)
    bool isFloat;
    OperandKind getKind() const override { return OperandKind::Immediate; }
};

struct LabelOpNode : public OperandNode {
    std::string name;
    OperandKind getKind() const override { return OperandKind::Label; }
};

struct AddressOpNode : public OperandNode {
    std::string baseReg;  // e.g., %rd0
    int offset = 0;       // e.g., 16
    std::string symbol;   // e.g., [sym]
    OperandKind getKind() const override { return OperandKind::Address; }
};

// ── Statements ─────────────────────────────────────────────────────

struct StatementNode : public ASTNode {
    virtual ~StatementNode() = default;
};

struct LabelStmtNode : public StatementNode {
    std::string name;
};

struct InstructionStmtNode : public StatementNode {
    std::string predicateReg; // e.g., %p0 (empty if none)
    bool predicateNegated = false;
    std::string opcode;      // e.g., add.s32
    std::vector<std::unique_ptr<OperandNode>> operandList;
};

struct DirectiveStmtNode : public StatementNode {
    std::string directive;   // e.g., .reg, .param
    std::vector<std::unique_ptr<OperandNode>> operandList;
    std::string type;        // e.g., .f32
};

// ── Top-level ──────────────────────────────────────────────────────

struct FunctionNode : public ASTNode {
    std::string name;
    bool isEntry = false;
    bool isVisible = false;
    // Parameters could be expanded into a dedicated ParamNode if needed
    std::vector<std::unique_ptr<DirectiveStmtNode>> params;
    std::vector<std::unique_ptr<StatementNode>> body;
};

struct ModuleNode : public ASTNode {
    std::string version;
    std::string target;
    int addressSize = 64;
    std::vector<std::unique_ptr<FunctionNode>> functions;
};

} // namespace ptxc

#endif // PTXC_PARSER_AST_H
