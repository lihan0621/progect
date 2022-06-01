#ifndef __MY_DATA__
#define __MY_DATA__
#include "util.hpp"
#include <iostream>
#include <unordered_map>
#include <jsoncpp/json/json.h>
using namespace std;

namespace cloud {
  typedef struct _FileInfo {
    string filename;  //文件名
    string url_path;  //下载链接路径
    string real_path; //实际存储路径
    size_t file_size; //文件大小
    time_t back_time; //备份时间
    bool pack_flag;   //压缩标志
    string pack_path; //压缩包路径名
  }FileInfo;

  class DataManager {
    private:
      string _back_dir = "./backup_dir/";//备份文件的实际存储路径
      string _pack_dir = "./pack_dir/";//压缩包存储路径
      string _download_prefix = "/download/";//下载链接的前缀路径
      string _pack_subfix = ".zip";//压缩包后缀名
      string _back_info_file = "./backup.dat";//存储备份信息的文件
      //用url作为key，是因为当前下载文件时，就会发送过来url
      unordered_map<string, FileInfo> _back_info;//<url, fileinfo>备份信息
    public:
      DataManager() {
        FileUtil(_back_dir).CreateDirectory();
        FileUtil(_pack_dir).CreateDirectory();
        if (FileUtil(_back_info_file).Exists()) {
          InitLoad();
        }
      }
      bool Storage() {
        Json::Value infos;
        vector<FileInfo> arry;
        this->SelectAll(&arry);
        for (auto &a : arry) {
          Json::Value info;
          info["filename"] = a.filename;
          info["url_path"] = a.url_path;
          info["real_path"] = a.real_path;
          info["file_size"] = (Json::UInt64)a.file_size;
          info["back_time"] = (Json::UInt64)a.back_time;
          info["pack_flag"] = a.pack_flag;
          info["pack_path"] = a.pack_path;
          infos.append(info);
        }
        string body;
        JsonUtil::Serialize(infos, &body);
        FileUtil(_back_info_file).Writer(body);
        return true;
      }
      //当模块运行起来后，第一时间将历史信息读取出来存储到hash表中
      bool InitLoad() {
        //1.读取文件中的历史备份信息
        string body;
        bool ret = FileUtil(_back_info_file).Read(&body);
        if (ret == false) {
          cout << "load history failed!\n";
          return false;
        }
        //2.对读取的信息进行反序列化
        Json::Value infos;
        ret = JsonUtil::UnSerialize(body, &infos);
        if (ret == false) {
          cout << "initload parse history falied!\n";
          return false;
        }
        //3.将反序列化得到的数据存储到hash表中
        int sz = infos.size();
        for (int i = 0; i < sz; ++i) {
          FileInfo info;
          info.filename = infos[i]["filename"].asString();
          info.url_path = infos[i]["url_path"].asString();
          info.real_path = infos[i]["real_path"].asString();
          info.file_size = infos[i]["file_size"].asInt64();
          info.back_time = infos[i]["back_time"].asInt64();
          info.pack_flag = infos[i]["pack_flag"].asBool();
          info.pack_path = infos[i]["pack_path"].asString();
          _back_info[info.url_path] = info;
        }
        return true; 
      }
      //数据的增删改查
      //增：输入一个文件名，在接口中获取各项信息，生成压缩包名称，填充结构体，压入hash表
      bool Insert(const string &pathname) {
        if (cloud::FileUtil(pathname).Exists() == false) {
          cout << "insert file is not exists!\n";
          return false;
        }
        // pathname = ./backup_dir/a.txt
        FileInfo info;
        info.filename = cloud::FileUtil(pathname).Name();//a.txt
        info.url_path = _download_prefix + info.filename;//  /download/a.txt 下载链接的资源路径
        info.real_path = pathname; //实际存储路径  ./backup_dir/a.txt
        info.file_size = cloud::FileUtil(pathname).Size(); //文件大小
        info.back_time = cloud::FileUtil(pathname).MTime();//最后一次修改时间就是备份时间
        info.pack_flag = false;//刚上传的文件都是非压缩状态
        info.pack_path = _pack_dir  + info.filename + _pack_subfix;//压缩包路径名 /.pack_dir/a.txt.zip
        _back_info[info.url_path] = info;//以url_path为key，info为value添加到map中；
        Storage();
        return true;
      }
      //改：当文件压缩存储了，要修改压缩标志，下载解压后也要修改压缩标志
      bool UpdateStatus(const string &pathname, bool status) {
        string url_path = _download_prefix + FileUtil(pathname).Name();
        auto it = _back_info.find(url_path);
        if (it == _back_info.end()) {
          cout << "file info is not exists!\n";
          return false;
        }
        it->second.pack_flag = status;
        return true;
      }
      //查：查询所有备份信息（前端展示界面需要-文件名，下载链接，大小，备份时间），
      //    查询单个文件信息（文件下载-获取文件时机备份路径）
      bool SelectAll(vector<FileInfo> *infos) {
        for (auto it = _back_info.begin(); it != _back_info.end(); ++it) {
          infos->push_back(it->second);
        }
        return true;
      }
      //通过key值也就是url_path获取备份信息
      bool SelectOne(const string &url_path, FileInfo *info) {
        auto it = _back_info.find(url_path);
        if (it == _back_info.end()) {
          cout << "file info is not exists!\n";
          return false;
        }
        *info = it->second;
        return true;
      }
      bool SelectOneByRealpath(const string &realpath, FileInfo *info) {
        for (auto it = _back_info.begin(); it != _back_info.end(); ++it) {
          if (it->second.real_path == realpath) {
            *info = it->second;//怕是一个浅拷贝
            return true;
          }
        }
        return true;
      }
      //删：基本备份的文件都不删除（防备功能扩展）
      bool DeleteOne(const string &url_path) {
        auto it = _back_info.find(url_path);
        if (it == _back_info.end()) {
          cout << "file info is not exists!\n";
          return false;
        }
        _back_info.erase(it);
        Storage();
        return true;
      }
  };
}


#endif
