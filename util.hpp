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
#include <jsoncpp/json/json.h>
#include "bundle.h"
namespace fs = std::experimental::filesystem;

using namespace std;

namespace cloud{
  class FileUtil{
    private:
      string _name;
    public:
      FileUtil(const string &name): _name(name){}
      string Name() {
        return fs::path(_name).filename().string();//返回去除路径的纯文件名
      }
      //文件是否存在
      bool Exists() {
        return fs::exists(_name);
      }
      //获取文件大小
      size_t Size() {
        if (this->Exists() == false) {
          return 0;
        }
        return fs::file_size(_name);
      }
      //最后一次修改时间
      time_t MTime() {
        if (this->Exists() == false) {
          return 0;
        }
        auto ftime = fs::last_write_time(_name);
        time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        return cftime;
      }
      //最后一次访问时间
      time_t ATime() {
        if (this->Exists() == false) {
          return 0;
        }
        struct stat st;
        stat(_name.c_str(), &st);
        return st.st_atime;
      }
      //读取文件所有数据到body中
      bool Read(string *body) {
        if (this->Exists() == false) {
          return false;
        }
        ifstream ifs;
        ifs.open(_name, ios::binary);//以二进制方式打开文件
        if (ifs.is_open() == false) {                                                                        
          cout << "open failed!\n";
          return false;
        }

        size_t fsize = this->Size();
        body->resize(fsize);
        ifs.read(&(*body)[0], fsize);//string::c_str() 返回值 const char*
        if (ifs.good() == false) {
          cout << "read file failed!\n";
          ifs.close();
          return false;
        }
        ifs.close();
        return true;
      }
      //将body中的数据写入文件
      bool Writer(const string &body) {
        ofstream ofs;
        ofs.open(_name, ios::binary);//以二进制方式打开文件
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
      //创建目录
      bool CreateDirectory() {
        if (this->Exists()) {
          return true;
        }
        fs::create_directories(_name);
        return true;
      }
      //遍历目录，获取目录下的所有文件路径名
      bool ScanDirectory(vector<string> *array) {
        if (this->Exists() == false) {
          return false;
        }
        //目录迭代器当前默认只能遍历深度为一层的目录
        for(auto &a : fs::directory_iterator(_name)) {
          if(fs::is_directory(a) == true) {
            continue;//如果当前文件是一个文件夹，则不处理，遍历下一个
          }
          //当前我们的目录遍历中，只获取普通文件信息，针对目录不做深度处理
          //string pathname = fs::path(a).filename().string();//纯文件名
          string pathname = fs::path(a).relative_path().string();//带有路径的文件名
          array->push_back(pathname);
        }
      } 
      bool Remove() {
        if (this->Exists() == false) {
          return true;
        }
        fs::remove_all(_name);//压缩后删除原文件
        return true;
      }
      //文件压缩
      bool Compress(const string &packname) {
        if (this->Exists() == false) {
          return false;
        }
        string body;
        if (this->Read(&body) == false) {
          cout << "Compress read failed!\n";
        }
        string packed = bundle::pack(bundle::LZIP, body);
        if (FileUtil(packname).Writer(packed) == false) {
          cout << "Compress write pack data failed!\n";
          return false;
        }
        fs::remove_all(_name);//压缩后删除原文件
        return true;
      }
      //文件解压缩
      bool UnCompress(const string &filename) {
        if (this->Exists() == false) {
          return false;
        }
        string body;
        if (this->Read(&body) == false) {
          cout << "UnCompress read pack data failed!\n";
        }
        string unpack_data = bundle::unpack(body);
        if (FileUtil(filename).Writer(unpack_data) == false) {
          cout << "UnCompress write file data failed!\n";
          return false;
        }
        fs::remove_all(_name);//解压后删除压缩包
        return true;
      }
  };

  class JsonUtil{
    public:
      //序列化
      static bool Serialize(Json::Value &val, string *body) {
        Json::StreamWriterBuilder swb;                                         
        Json::StreamWriter *sw = swb.newStreamWriter(); //new一个StreamWriter对象
        stringstream ss;                                                       
        int ret = sw->write(val, &ss);//实现序列化                             
        if (ret != 0) {                                                        
          cout << "seriallize failed!\n";                                      
          delete sw;                                                           
          return false;
        } 
        *body = ss.str();
        delete sw;
        return true;
      }
      //反序列化
      static bool UnSerialize(string &body, Json::Value *val) {
        Json::CharReaderBuilder crb;
        Json::CharReader *cr = crb.newCharReader();
        string err;
        //pars(字符串首地址，字符串末尾地址，Json::Value对象指针，错误信息获取)
        bool ret = cr->parse(body.c_str(), body.c_str() + body.size(), val, &err);
        if (ret == false) {
          cout << "UnSerialize failed:" << err << endl;
          delete cr;
          return false;
        }
        delete cr;
        return true;
      }
  };

}

#endif
