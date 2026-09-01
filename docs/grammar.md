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


## 5. Lexical Elements

### Keywords
- int
- float

### Identifiers
- Begin with a letter
- Followed by letters or digits

Examples:
- a
- total
- value1

### Numbers
- Integer constants: 10, 25, 100
- Floating-point constants: 5.5, 10.25

### Delimiters
- `;`
- `(` 
- `)`

### Assignment Operator
- `=`

### Arithmetic Operators
- `+`
- `-`
- `*`
- `/`


**Important:** Make sure the Markdown code fences are closed properly. Your existing file may already have the examples section below what is visible in the screenshot, so **don't duplicate Section 5/6 if they're already there**. Just check first.

---

# Then SAVE — and this becomes Commit #2

Once the grammar file is complete:

**Ctrl + S**

Then open **Source Control**.

You should see:

```text
Changes

M  docs/grammar.md
