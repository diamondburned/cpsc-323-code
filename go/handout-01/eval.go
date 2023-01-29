package main

import "fmt"

// Evaluate evaluates the given AST node with the given variable values. The
// returned value is the result of the evaluation.
func Evaluate(ast Node, variables map[Value]int) (int, error) {
	e := evaluator{variables: variables}
	n := e.eval(ast)
	return n, e.err
}

type evaluator struct {
	variables map[Value]int
	err       error
}

func (e *evaluator) eval(n Node) int {
	if e.err != nil {
		return 0
	}

	switch n := n.(type) {
	case Value:
		v, ok := e.variables[n]
		if ok {
			return v
		}
		e.err = fmt.Errorf("unknown variable %q", n)
		return 0

	case Operation:
		l := e.eval(n.Left)
		r := e.eval(n.Right)

		switch n.Op {
		case Plus:
			return l + r
		case Minus:
			return l - r
		case Multiply:
			return l * r
		case Divide:
			return l / r
		default:
			panic(fmt.Sprintf("unknown operator %q", n.Op))
		}

	default:
		panic(fmt.Sprintf("unknown node %T", n))
	}
}
