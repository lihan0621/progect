#pragma once
#include "data.hpp"
#include "httplib.h"

namespace cloud {
	class Client {
	private:
		string _backup_dir = "./backup_dir";
		DataManager* _data = NULL;
		string _srv_ip;
		int _srv_port;
	private:
		string FileEtag(const string& pathname) {
			size_t fsize = FileUtil(pathname).Size();
			time_t mtime = FileUtil(pathname).MTime();
			stringstream ss;
			ss << fsize << "-" << mtime;
			return ss.str();
		}
		bool IsNeedBackup(const string& filename) {
			//需要备份：1.没有历史信息；2.有历史信息，但是被修改过（标识是否一致）
			string old_etag;
			if (_data->SelectOne(filename, &old_etag) == false) {
				return true;//没有历史备份信息
			}
			string new_etag = FileEtag(filename);
			time_t mtime = FileUtil(filename).MTime();
			time_t ctime = time(NULL);
			//防止文件处于持续修改状态，因此判断最后一次修改时间，与当前时间间隔是否超过3秒
			if (new_etag != old_etag && (ctime - mtime) > 3) {
				return true;//当前标识与历史标识不同，意味着文件被修改过
			}
			return false;
		}
		bool Upload(const string& filename) {
			httplib::Client client(_srv_ip, _srv_port);
			httplib::MultipartFormDataItems items;
			httplib::MultipartFormData item;
			item.name = "file";//区域名称标识
			item.filename = FileUtil(filename).Name();//文件名
			FileUtil(filename).Read(&item.content); //文件数据
			item.content_type = "application/octet-stream";//文件数据格式---二进制流
			items.push_back(item);
			auto res = client.Post("/upload", items);
			if (res && res->status != 200) {
				return false;
			}
			return true;
		}
	public:
		Client(const string srv_ip, int srv_port) :_srv_ip(srv_ip), _srv_port(srv_port) {}
		bool RunModule() {
			//1.初始化：初始化数据管理对象，创建监控目录
			FileUtil(_backup_dir).MCreateDirectory();
			_data = new DataManager();
			while (1) {
				//2.创建目录，获取目录下所有文件
				vector<string> arry;
				FileUtil(_backup_dir).ScanDirectory(&arry);
				//3.根据历史备份信息判断，当前文件是否需要备份
				for (auto& a : arry) {
					if (IsNeedBackup(a) == false) {
						continue;
					}
					cout << a << "need backup!\n";
					//4.如果需要则备份文件
					bool ret = Upload(a);
					//5.添加备份信息
					_data->Insert(a);
					cout << a << "backup success!\n";
				}
				Sleep(10);
			}
		}
	};
}
