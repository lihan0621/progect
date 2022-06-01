#ifndef __MY_HOT__
#define __MY_HOT__
#include "data.hpp"
#include <unistd.h>

extern cloud::DataManager *_data;//全局数据
using namespace std;
namespace cloud {
  class HotManager {
    private:
      time_t _hot_time = 30; //热点判断时长，应该是个可配置项，当前简化，先默认30s
      string _backup_dir = "./backup_dir/"; //要检测的原文件的存储路径
    public:
      HotManager() {
        FileUtil(_backup_dir).CreateDirectory();
      }
      bool IsHot(const string &filename) {
        time_t atime = FileUtil(filename).ATime();
        time_t ctime = time(NULL);
        if ((ctime - atime) > _hot_time) {
          return false;
        }
        return true;
      }
      bool RunModule() {
        while(1) {
          //1.遍历目录
          vector<string> arry;
          FileUtil(_backup_dir).ScanDirectory(&arry);
          //2.遍历信息
          for (auto &file : arry) {
            //3.获取指定文件时间属性，以当前系统时间，进行热点判断
            if (IsHot(file) == true) {
              continue;//热点文件暂时不处理
            }
            //获取当前文件的历史信息
            FileInfo info;
            bool ret = _data->SelectOneByRealpath(file, &info);
            if (ret == false) {
              //当前检测到的文件，没有历史备份信息，这可能是一个异常上传的文件，删除处理
              cout << "An exception file is deleted. Delete it!\n"; 
              FileUtil(file).Remove();
              continue;//异常文件删除后，处理下一个
              //对于检测到没有历史信息的文件，则新增信息，然后进行压缩存储
              //_data->Insert(file);
              //_data->SelectOneByRealpath(file, &info);
            }
            //4.非热点进行压缩存储
            FileUtil(file).Compress(info.pack_path);
            //5.压缩后则进行备份信息修改
            //info.pack_flag = true;
            _data->UpdateStatus(file, true);
            cout << info.real_path << "<--compress-->" << info.pack_path << endl;
          }
          usleep(1000);//避免空目录情况下，空遍历消耗CPU资源过高
        }
        return true;
      }
  };
}

#endif
