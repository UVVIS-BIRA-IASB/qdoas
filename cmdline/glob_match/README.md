# glob_match

Simple "glob" pattern matching.

* '*' matches zero or more of any character.
* '?' matches any single character.

Example:

```
if (glob_match("hello*", arg)) ...
```

See the unit test glob_match_test.cpp for more examples.

Build and run the unit test:

```
g++ -Wall -Werror -std=c++17 glob_match_test.cpp glob_match.cpp && ./a.out
```
