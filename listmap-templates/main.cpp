// $Id: main.cpp,v 1.8 2015-04-28 19:23:13-07 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string, string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
      case '@':
         traceflags::setflags (optarg);
         break;
      default:
         complain() << "-" << (char) optopt << ": invalid option"
                    << endl;
         break;
      }
   }
}

void key_options(string input, str_str_map& test) {
   if (input[input.find_first_not_of(" ")] == '#' or input.find_first_not_of(" ") == '\n' 
      or input.find_first_not_of(" ") == string::npos) { //Ignore all the comments and empty lines
   } else if (input.find_first_of("=") != string::npos) {
      if (input.find_first_of("=") == 0) { //If there's an equal sign at the beginning.
         if (input.find_first_not_of("= ") == string::npos) { // "= value" command
            for (str_str_map::iterator itor = test.begin(); itor != test.end(); ++itor) {
               cout << itor->first << " = " << itor->second << endl;
            }
         } else { // "=" command
            auto wanted = input.substr(input.find_first_of("= ") + 1, string::npos);
            for (auto value = test.begin(); value != test.end(); ++value) {
               if (wanted == value->second) {
                  cout << value->first << " = " << value->second << endl;
               }
            }
         }
      } else { //If the equal sign is in the middle.
         auto key = input.substr(input.find_first_not_of("= "), input.find_first_of("= ")); //key is stuff before the equal sign
         if (input.find_first_not_of("= ", input.find_first_of("= ")) != string::npos) { // "key = value" command
            auto value = input.substr(input.find_first_not_of("= ", input.find_first_of("= ")) , string::npos);
            str_str_pair pair (key, value);
            test.insert(pair);
            cout << key << " = " << value << endl;
         } else { //"key =" command
            auto item = test.find(key);
            if (item != test.end()) {
               test.erase(item);
            }
         }
      }
   } else { //If there's no equal sign, that mean we need to find the key.
      auto item = test.find(input);
      if (item != test.end()) {
         cout << item->first << " = " << item->second << endl;
      } else {
         cout << input << ": key not found" << endl;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   if (argc < 2) { //cin input
      str_str_map test;
      char buffer[1024];
      for (int i = 1;; ++i) {
         if (cin.eof()) break;
         cin.getline (buffer, 1024);
         string input = buffer;
         if (input == "") continue;
         cout << "-: " << i << ": " << input << endl;
         key_options(input, test);
      }
   } else { //file input
      for (int i = 1; i < argc; ++i) {
         str_str_map test;
         int counter = 1;
         fstream file;
         string input {};
         file.open(argv[i]);
         if (file.is_open()) { //make sure the file is vaild
            while (getline(file, input)) {
               cout << argv[i] << ": " << counter++ << ": " << input << endl;
               key_options(input, test);
            }
         } else {
            cout << argv[i] << ": No such file." << endl;
         }
         file.close();
      }
   }
   return EXIT_SUCCESS;
}

