package main

import (
	"bufio"
	"fmt"
	"os"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	for {
		fmt.Print("Enter a postfix expression with $ at the end: ")
		if !scanner.Scan() {
			break
		}

		doExpr(scanner.Text())

		fmt.Print("Continue(Y/n)? ")
		if !scanner.Scan() || scanner.Text() == "n" {
			break
		}
	}
}

func doExpr(text string) {
	token, err := Parse(text)
	if err != nil {
		fmt.Println("\tError: parse:", err)
		return
	}

	value, err := Evaluate(token, map[Value]int{
		"a": 5,
		"b": 7,
		"c": 2,
		"d": 4,
	})
	if err != nil {
		fmt.Println("\tError: evaluate:", err)
		return
	}

	fmt.Println("\tValue =", value)
}
