#ifndef __MY_DATA__
#define __MY_DATA__
#include "util.hpp"
#include <iostream>
#include <unordered_map>
using namespace std;

namespace cloud {

	class DataManager {
	private:
		string _backup_file = "./backup.dat";
		unordered_map<string, string> _backup_map;
	public:
		static int Split(const string& body, const string& sep, vector<string>* arry) {
			int count = 0;
			size_t idx = 0, pos = 0;
			while (idx < body.size()) {
				pos = body.find(sep, idx);
				if (pos == string::npos) {//找不到分隔符
					break;
				}
				if (pos == idx) {
					idx = pos + sep.size(); 
					continue;
				}
				string val = body.substr(idx, pos - idx); 
				arry->push_back(val);
				idx = pos + sep.size();//让下次的检索间隔符起始位置向后偏移
				count++;
			}
			if (idx < body.size()) {
				string val = body.substr(idx);//从idx位置截断到末尾
				arry->push_back(val);
				count++;
			}
			return count;
		}
	public:
		DataManager() {
			InitLoad();
		}
		bool Storage() {
			stringstream body;
			for (auto it = _backup_map.begin(); it != _backup_map.end(); ++it) {
				body << it->first << "=" << it->second << "\n";
			}
			FileUtil(_backup_file).Writer(body.str());
			return true;
		}
		bool InitLoad() {
			string body;
			if (FileUtil(_backup_file).Read(&body) == false) {
				return false;
			}
			vector<string> arry;
			int count = Split(body, "\n", &arry);
			for (int i = 0; i < (int)arry.size(); ++i) {
				size_t pos = arry[i].find("=");
				if (pos == string::npos) {
					continue;
				}
				string key = arry[i].substr(0, pos);
				string val = arry[i].substr(pos + 1);
				_backup_map[key] = val;
			}
			return true;
		}
		string FileEtag(const string& pathname) {
			size_t fsize = FileUtil(pathname).Size();
			time_t mtime = FileUtil(pathname).MTime();
			stringstream ss;
			ss << fsize << "-" << mtime;
			return ss.str();
		}
		bool Insert(const string& pathname) {
			string etag = FileEtag(pathname);
			_backup_map[pathname] = etag;
			Storage();
			return true;
		}
		bool Update(const string& pathname) {
			string etag = FileEtag(pathname);
			_backup_map[pathname] = etag;
			Storage();
			return true;
		}
		bool SelectAll(vector<pair<string, string>>* infos) {
			for (auto it = _backup_map.begin(); it != _backup_map.end(); ++it) {
				pair<string, string> info;
				info.first = it->first;
				info.second = it->second;
				infos->push_back(info);
			}
			return true;
		}
		bool SelectOne(const string& filename, string* etag) {
			auto it = _backup_map.find(filename);
			if (it == _backup_map.end()) {
				return false;
			}
			*etag = it->second;
			return true;
		}
		bool Delete(const string& filename) {

			auto it = _backup_map.find(filename);
			if (it == _backup_map.end()) {
				return false;
			}
			_backup_map.erase(it);
			Storage();
			return true;
		}
	};
}


#endif
