package main

import (
	"fmt"
	"strings"
)

// Node is an AST node in a postfix expression.
type Node interface {
	node()
	fmt.Stringer
}

// Value is a node that represents a value.
type Value string

func (v Value) node()          {}
func (v Value) String() string { return string(v) }

// Operator is an operator.
type Operator byte

const (
	Plus     Operator = '+'
	Minus    Operator = '-'
	Multiply Operator = '*'
	Divide   Operator = '/'
	End      Operator = '$'
)

// Operation represents an operation. It is a node by itself.
type Operation struct {
	Left  Node
	Right Node
	Op    Operator
}

func (op Operation) node() {}
func (op Operation) String() string {
	return fmt.Sprintf("(%s %s %s)", op.Left, op.Right, string(op.Op))
}

// Parse parses the given list of words postfix-style into a stack of AST nodes.
func Parse(in string) (Node, error) {
	words := strings.Split(in, "")
	var stack Stack[Node]

loop:
	for _, word := range words {
		switch word {
		case "+", "-", "*", "/":
			op := Operator(word[0])
			// Left is inserted first. Right is inserted after, so it is next to
			// left. When popping, we pop the right one first.
			r := stack.pop()
			l := stack.pop()
			stack.push(Operation{Left: l, Right: r, Op: op})
		case "$":
			break loop
		default:
			stack.push(Value(word))
		}
	}

	if len(stack) != 1 {
		// We should have exactly one token left, but we have more. The given
		// expression is invalid.
		return nil, fmt.Errorf("invalid expression %v", stack)
	}

	return stack[0], nil
}

type Stack[T any] []T

func (s *Stack[T]) pop() T {
	v := (*s)[len(*s)-1]
	*s = (*s)[:len(*s)-1]
	return v
}

func (s *Stack[T]) push(v T) {
	*s = append(*s, v)
}
