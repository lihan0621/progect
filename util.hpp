#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#ifndef __MY_UTIL__
#define __MY_UTIL__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
#include <sys/stat.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

using namespace std;

namespace cloud{
  class FileUtil{
    private:
      string _name;
    public:
      FileUtil(const string &name): _name(name){}
      string Name() {
        return fs::path(_name).filename().string();
      }
      bool Exists() {
        return fs::exists(_name);
      }
      size_t Size() {
        if (this->Exists() == false) {
          return 0;
        }
        return fs::file_size(_name);
      }
      time_t MTime() {
        if (this->Exists() == false) {
          return 0;
        }
        auto ftime = fs::last_write_time(_name);
        time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        return cftime;
      }
      time_t ATime() {
        if (this->Exists() == false) {
          return 0;
        }
        struct stat st;
        stat(_name.c_str(), &st);
        return st.st_atime;
      }
      bool Read(string *body) {
        if (this->Exists() == false) {
          return false;
        }
        ifstream ifs;
        ifs.open(_name, ios::binary);
        if (ifs.is_open() == false) {                                                                        
          cout << "open failed!\n";
          return false;
        }

        size_t fsize = this->Size();
        body->resize(fsize);
        ifs.read(&(*body)[0], fsize);
        if (ifs.good() == false) {
          cout << "read file failed!\n";
          ifs.close();
          return false;
        }
        ifs.close();
        return true;
      }
      bool Writer(const string &body) {
        ofstream ofs;
        ofs.open(_name, ios::binary);
        if (ofs.is_open() == false) {
          cout << "open failed!\n";
          return false;
        }
        ofs.write(body.c_str(), body.size());
        if (ofs.good() == false) {
          cout << "read file failed!\n";
          ofs.close();
          return false;
        }
        ofs.close();
        return true;
      }
      bool MCreateDirectory() {
        if (this->Exists()) {
          return true;
        }
        fs::create_directories(_name);
        return true;
      }
      bool ScanDirectory(vector<string> *array) {
        if (this->Exists() == false) {
          return false;
        }
        for(auto &a : fs::directory_iterator(_name)) {
          if(fs::is_directory(a) == true) {
            continue;
          }
          string pathname = fs::path(a).relative_path().string();
          array->push_back(pathname);
        }
      } 
      bool Remove() {
        if (this->Exists() == false) {
          return true;
        }
        fs::remove_all(_name);
        return true;
      }
  };
}

#endif
