class PostfixParser:
  """Parses, validates, and and evaluates a postfix expression"""
  
  # the values that the program accepts
  values = {
    'a': 5,
    'b': 7,
    'c': 2,
    'd': 4
  }
  # the set of all valid inputs
  valid_inputs = set(list(values.keys()) + ['+', '-', '*', '/'])

  def __eval_binary_expr(self, first: float, second: float, op: str) -> float:
    """Evaluate a binary expression given two ordered values and an operator and 
    return the result
    
    :param first: the first operand
    :type first: float
    :param second: the second operand
    :type second: float
    :param op: the operator
    :type op: str
    :returns: the result of the expression
    :rtype: float
    """

    if op == '+':
      return first + second
    if op == '-':
      return first - second
    if op == '*':
      return first * second
    if op == '/':
      return first / second
    raise Exception(f'Invalid operator \'{op}\'')

  def eval_postfix_expr(self, expr: str) -> float:
    """Parse a postfix expression and return the result
    
    :param expr: the expression to evaluate
    :type expr: str
    :returns: the result of the expression
    :rtype: float
    """

    if len(expr) == 0:
      raise Exception('Empty expression')

    stack = []
    for char in expr:
      # validate character
      if char not in self.valid_inputs:
        raise Exception(f'Valid expression inputs: {self.valid_inputs}')
      # check if current character is a number
      if char in self.values:
        stack.append(self.values[char])
      # current character is an operator
      else:
        # pop last two elements from the stack
        if len(stack) < 2:
          raise Exception('Invalid expression')
        last, second_last = stack.pop(), stack.pop()
        # evaluate the binary expression with the current operator
        result = self.__eval_binary_expr(
          first=second_last, op=char, second=last
        )
        # push the result onto the stack
        stack.append(result)
    
    # there should be exactly one element left in the stack or the expression 
    # was invalid
    if len(stack) != 1:
      raise Exception('Invalid expression')
    return stack[0]
