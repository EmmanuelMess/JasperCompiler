#pragma once

namespace AST {
struct AST;
}

namespace Compiler {

struct Compiler;


void compileAny(AST::AST* ast, Compiler& e);
void compile(AST::AST*, Compiler&);

}
