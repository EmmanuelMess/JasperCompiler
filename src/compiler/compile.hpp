#pragma once

namespace AST {
struct AST;
}

namespace Compiler {

struct Compiler;

void compile(AST::AST*, Compiler&);

}
