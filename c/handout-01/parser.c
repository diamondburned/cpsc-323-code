#include "parser.h"

#include <stddef.h>
#include <stdlib.h>

Node* node_parse(const char* input) {
  Node* stack[512];
  int nstack = 0;
  for (int i = 0; input[i]; i++) {
    char word = input[i];
    switch (word) {
      case OP_PLUS:
      case OP_MINUS:
      case OP_MULTIPLY:
      case OP_DIVIDE: {
        if (nstack < 2) {
          for (int j = 0; j < nstack; j++) {
            node_free(stack[j]);
          }
          return NULL;
        }

        Node* r = stack[--nstack];
        Node* l = stack[--nstack];

        Operation op = {
            .left = l,
            .right = r,
            .op = (Operator)word,
        };

        Node* node = calloc(sizeof(Node), 1);
        node->type = NODE_OPERATION;
        node->operation = op;

        stack[nstack++] = node;
        break;
      }
      case OP_END: {
        goto done;
      }
      default: {
        Value value = {.name = word};
        Node* node = calloc(sizeof(Node), 1);
        node->type = NODE_VALUE;
        node->value = value;

        stack[nstack++] = node;
        break;
      }
    }
  }
done:
  if (nstack != 1) {
    for (int j = 0; j < nstack; j++) {
      node_free(stack[j]);
    }
    return NULL;
  }
  return stack[0];
}

void node_free(Node* node) {
  switch (node->type) {
    case NODE_VALUE:
      break;
    case NODE_OPERATION:
      node_free(node->operation.left);
      node_free(node->operation.right);
      break;
  }
  free(node);
}
