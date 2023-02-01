#-------------------------------------------------------------------------------
#  Group names: Ethan Safai
#  Assignment : No.1
#  Due date   : 02/02/23
# Purpose: this program reads an expression in postfix form, evaluates the 
# expression, and displays its value.
#-------------------------------------------------------------------------------
from postfixparser import PostfixParser

def main():
  """Receive, evaluate, and display the result of postfix expressions from the
  user until they wish to exit the program"""

  parser = PostfixParser()

  while True:
    # read and evaluate user input
    expr = input('Enter a postfix expression with $ at the end: ')
    if not expr.endswith('$') or len(expr) < 2:
      raise Exception('Invalid input')

    result = parser.eval_postfix_expr(expr[:len(expr)-1])
    print(f'Value = {result}')

    # check if the user would like to continue
    if input('CONTINUE(y/n)? ') != 'y':
      break

if __name__ == "__main__":
  main()
