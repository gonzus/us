## us -- micro scheme language

For the longest time I have wanted to implement a toy version of a
[scheme](https://en.wikipedia.org/wiki/Scheme_(programming_language))-like
language.  My main reason is being totally in love with the eval-apply idea of
a [meta-circular
evaluator](https://en.wikipedia.org/wiki/Meta-circular_evaluator), which
captures in a **really** tiny description the power of a programming language.

I have read the code of many such projects, and I know it is easy to have
something minimal in a couple of hundreds lines of C code.  However, I wanted
this to be a bit more than your usual toy:

1. I wanted the code to be readable and understandable by a competent C
   programmer.
2. I wanted to provide [lexical
   scoping](https://en.wikipedia.org/wiki/Scope_(computer_science)#Lexical_scope_vs._dynamic_scope).
   This is, in my opinion, an absolute must for a programming language, and I
   wanted to re-check my knowledge of how this is done and how hard it is.
3. I wanted to include an acceptable implementation of [garbage
   collection](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)),
   even if it is slow.
4. I wanted the implementation to be easily embeddable, and allow for multiple
   instances of the interpreter to be alive at the same time in a program.

This is the result of my work so far.  As of 19 September 2018, the code has
1952 LOC.

Enjoy!
