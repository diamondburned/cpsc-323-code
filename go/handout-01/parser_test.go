package main

import (
	"fmt"
	"testing"
)

func TestParse(t *testing.T) {
	type test struct {
		in  string
		out string
	}

	tests := []test{
		{"ab+cd*+$", "((a b +) (c d *) +)"},
		{"abcd+++$", "(a (b (c d +) +) +)"},
		{"abcd*-*$", "(a (b (c d *) -) *)"},
	}

	for i, tc := range tests {
		t.Run(fmt.Sprintf("%d", i), func(t *testing.T) {
			n, err := Parse(tc.in)
			if err != nil {
				t.Fatal("cannot parse:", err)
			}
			if tc.out != n.String() {
				t.Fatalf("for input %q:\n"+
					"expected %q\n"+
					"got      %q", tc.in, tc.out, n.String())
			}

			t.Logf("%q -> %q", tc.in, n.String())
		})
	}
}
