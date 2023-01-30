#include "eval.h"

#include "parser.h"

int eval(const Node* node, const int variables[255]) {
  switch (node->type) {
    case NODE_VALUE: {
      return variables[(int)node->value.name];
    }
    case NODE_OPERATION: {
      int l = eval(node->operation.left, variables);
      int r = eval(node->operation.right, variables);
      switch (node->operation.op) {
        case OP_PLUS:
          return l + r;
        case OP_MINUS:
          return l - r;
        case OP_MULTIPLY:
          return l * r;
        case OP_DIVIDE:
          return l / r;
      }
      break;
    }
  }
  return 0;
}
