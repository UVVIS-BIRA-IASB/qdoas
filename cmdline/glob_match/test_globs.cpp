#include <iostream>
#include <string>
#include <vector>

#include "glob_match.hpp"


using namespace std;

int main() {
  cout << "Hello, World!" << endl;

  string pat1 = "myfile?.*";
  string pat2 = "myfile*.*";

  vector<string> filenames { "myfile.nc", "myfile1.nc", "myfile2.jpeg", "myfile10.png"};

  for (const auto& f: filenames) {
    cout << pat1 << " matches " << f << "? " << glob_match(pat1, f) << "\n"
         << pat2 << " matches " << f << "? " << glob_match(pat2, f) << "\n" << endl;
  }
}
