/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <iterator>
#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>

#include <dirent.h>

class dir_iter : public std::iterator<std::input_iterator_tag, std::string> {
private:
  std::string path;
  std::shared_ptr<DIR> stream;
  std::shared_ptr<dir_iter>child;
  std::string current;
  size_t maxdepth; // max depth of recursion
  void advance() {
    if (child) {
      // if we have a child iterator, advance that.
      ++(*child);
      if (*child == end()) {
        child.reset();
      } else {
        current = *(*child);
        return;
      }
    }
    // loop readdir until
    //
    // - we find a file
    //
    // - we find a non-empty child directory
    //
    // - we reach the end of directory
    bool have_file = false;
    while (!have_file) {
      auto entry = readdir(stream.get());
      if (!entry) {
        this->stream.reset();
        current = "";
        return;
      }
      switch (entry->d_type) {
      case DT_DIR:
        // If we are not yet at maximum depth, enter child directory.  Skip "." and "..".
        if (maxdepth && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
          auto dir = std::make_shared<dir_iter>(path + "/" + entry->d_name, maxdepth - 1);
          if (*dir != end()) {
            have_file = true;
            child = dir;
            current = *(*child);
          }
        }
        break;
        // symbolic links and regular files are returned as entries:
      case DT_LNK:
      case DT_REG:
        have_file = true;
        current = path + "/" + entry->d_name;
        break;
      }
    }
  }

public:
  dir_iter(std::string path, size_t depth = 0) : path(path),
                                                 stream(opendir(path.c_str()), [](DIR *dir) { dir && closedir (dir); }),
                                                 current(""), maxdepth(depth) {
    if (!stream)
      throw std::runtime_error("failed to open directory '" + path + "'");
    this->advance();
  };
  dir_iter() : stream(nullptr), current("") {
  };
  std::string const& operator*() const { return this->current; }
  const std::string * operator->() const { return &this->current; }
  dir_iter& operator++() { this->advance(); return *this; }
  dir_iter operator++(int) { //
    dir_iter rc(*this);
    this->operator++();
    return rc;
  }
  bool operator==(dir_iter const& other) const {
    return (this->stream == other.stream && this->current == other.current);
  }
  bool operator!=(dir_iter const& other) const {
    return !(*this == other);
  }

  // begin() & end() functions for range-based for loops:
  dir_iter begin() {
    return *this;
  }
  dir_iter end() {
    return dir_iter();
  }

};
