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

#include "glob_match.hpp"
#include <cctype>

using std::string_view;

namespace
{

template<typename EQUAL>
bool glob_match(string_view pattern, string_view target, EQUAL&& equal)
{
  auto p = pattern.begin();
  auto pe = pattern.end();
  auto q = target.begin();
  auto qe = target.end();
  for (;;) {
    if (p == pe)
      return q == qe;
    if (*p == '*') {
      ++p;
      for (auto backtracker = qe; backtracker >= q; --backtracker)
        if (glob_match(string_view(p, pe - p),
                       string_view(backtracker, qe - backtracker),
                       std::forward<EQUAL>(equal)))
          return true;
      break;
    }
    if (q == qe)
      break;
    if (*p != '?' && !equal(*p, *q))
      break;
    ++p, ++q;
  }
  return false;
}

} // namespace

bool glob_match(string_view pattern, string_view target)
{
  return glob_match(pattern, target,
                    [](char a, char b) {
                      return a == b;
                    });
}

bool glob_match_caseless(string_view pattern, string_view target)
{
  return glob_match(pattern, target,
                    [](char a, char b) {
                      return tolower(static_cast<unsigned char>(a)) ==
                             tolower(static_cast<unsigned char>(b));
                    });
}
