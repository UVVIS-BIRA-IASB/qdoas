/*
  Simple "glob" pattern matching.
  '*' matches zero or more of any character.
  '?' matches any single character.

  Example:

    if (glob_match("hello*", arg)) ...
*/

/*
 This software is distributed under the "Simplified BSD license":

 Copyright Michael Cook <michael@waxrat.com>. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <string>

template <typename P, typename Q, typename EQUAL>
bool glob_match(P p, P pe,                 // the pattern
                Q q, Q qe,                 // the string we're trying to match
                decltype(*p) closure_char, // e.g., '*'
                decltype(*p) any_char,     // e.g., '?'
                EQUAL equal)               // compare two chars
{
  for (;;) {
    if (p == pe)
      return q == qe;
    if (*p == closure_char) {
      ++p;
      for (auto backtracker = qe; backtracker >= q; --backtracker)
        if (glob_match(p, pe, backtracker, qe, closure_char, any_char, equal))
          return true;
      break;
    }
    if (q == qe)
      break;
    if (*p != any_char && !equal(*p, *q))
      break;
    ++p, ++q;
  }
  return false;
}

bool glob_match(std::string const& needle, std::string const& haystack);

bool glob_match(char const* needle, char const* haystack);

bool glob_match(char const* n, char const* ne,
                char const* h, char const* he);

bool glob_match_caseless(std::string const& needle,
                         std::string const& haystack);

bool glob_match_caseless(char const* needle, char const* haystack);

bool glob_match_caseless(char const* n, char const* ne,
                         char const* h, char const* he);
