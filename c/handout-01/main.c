#include <sched.h>
#include <stdbool.h>
#include <stdio.h>

#include "eval.h"
#include "parser.h"

static const int variables[255] = {
    ['a'] = 5,
    ['b'] = 7,
    ['c'] = 2,
    ['d'] = 4,
};

void do_eval(const char expr[]) {
  Node* node = node_parse(expr);
  if (node == NULL) {
    printf("\tError: invalid expression given\n");
    return;
  }

  int value = eval(node, variables);
  printf("\tValue = %d\n", value);

  node_free(node);
}

int main() {
  char buf[256];

  while (true) {
    printf("Enter a postfix expression with $ at the end: ");
    scanf("%254s", buf);

    do_eval(buf);

    printf("Continue? (Y/n): ");
    scanf("%254s", buf);

    if (buf[0] == 'n') {
      break;
    }
  }
}
