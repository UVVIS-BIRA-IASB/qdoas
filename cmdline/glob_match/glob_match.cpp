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
#include <cstring>
#include <cctype>

bool glob_match(char const* n, char const* ne,
                char const* h, char const* he)
{
  return glob_match(n, ne, h, he, '*', '?',
                    [](char a, char b) {
                      return a == b;
                    });
}

bool glob_match(char const* needle, char const* haystack)
{
  return glob_match(needle, needle + strlen(needle),
                    haystack, haystack + strlen(haystack));
}

bool glob_match(std::string const& needle, std::string const& haystack)
{
  auto n = needle.data();
  auto h = haystack.data();
  return glob_match(n, n + needle.size(),
                    h, h + haystack.size());
}

bool glob_match_caseless(char const* n, char const* ne,
                         char const* h, char const* he)
{
  return glob_match(n, ne, h, he, '*', '?',
                    [](char a, char b) {
                      return tolower((unsigned char) a) ==
                             tolower((unsigned char) b);
                    });
}

bool glob_match_caseless(char const* needle, char const* haystack)
{
  return glob_match_caseless(needle, needle + strlen(needle),
                             haystack, haystack + strlen(haystack));
}

bool glob_match_caseless(std::string const& needle, std::string const& haystack)
{
  auto n = needle.data();
  auto h = haystack.data();
  return glob_match_caseless(n, n + needle.size(),
                             h, h + haystack.size());
}
