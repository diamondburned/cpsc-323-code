#pragma once

enum Operator {
  OP_PLUS = '+',
  OP_MINUS = '-',
  OP_MULTIPLY = '*',
  OP_DIVIDE = '/',
  OP_END = '$',
};

enum NodeType {
  NODE_VALUE,
  NODE_OPERATION,
};

typedef enum NodeType NodeType;
typedef enum Operator Operator;
typedef struct Value Value;
typedef struct Operation Operation;
typedef struct Node Node;

struct Value {
  char name;
};

struct Operation {
  Node* left;
  Node* right;
  enum Operator op;
};

struct Node {
  enum NodeType type;
  union {
    Value value;
    Operation operation;
  };
};

Node* node_parse(const char* input);
void node_free(Node* node);
