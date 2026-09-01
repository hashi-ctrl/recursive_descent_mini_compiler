# Mini Compiler Grammar

## 1. Language Overview

The mini-language supports variable declarations, assignments,
arithmetic expressions, integer and floating-point constants,
identifiers, and parentheses.

## 2. Data Types

- int
- float

## 3. Supported Operators

- Addition: +
- Subtraction: -
- Multiplication: *
- Division: /

## 4. Grammar

program     → statement*

statement   → declaration | assignment

declaration → type identifier ;

assignment  → identifier = expression ;

expression  → term { (+ | -) term }

term        → factor { (* | /) factor }

factor      → identifier
            | number
            | ( expression )

type        → int | float

number      → integer | float

## 5. Examples

### Valid

int a;
float b;

a = 10;
b = a + 5.5;
a = (b + 2) * 3;

### Invalid

int;
a = ;
int a
a = 10 + ;
