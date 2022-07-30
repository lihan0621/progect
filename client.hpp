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
			//��Ҫ���ݣ�1.û����ʷ��Ϣ��2.����ʷ��Ϣ�����Ǳ��޸Ĺ�����ʶ�Ƿ�һ�£�
			string old_etag;
			if (_data->SelectOne(filename, &old_etag) == false) {
				return true;//û����ʷ������Ϣ
			}
			string new_etag = FileEtag(filename);
			time_t mtime = FileUtil(filename).MTime();
			time_t ctime = time(NULL);
			//��ֹ�ļ����ڳ����޸�״̬������ж����һ���޸�ʱ�䣬�뵱ǰʱ�����Ƿ񳬹�3��
			if (new_etag != old_etag && (ctime - mtime) > 3) {
				return true;//��ǰ��ʶ����ʷ��ʶ��ͬ����ζ���ļ����޸Ĺ�
			}
			return false;
		}
		bool Upload(const string& filename) {
			httplib::Client client(_srv_ip, _srv_port);
			httplib::MultipartFormDataItems items;
			httplib::MultipartFormData item;
			item.name = "file";//�������Ʊ�ʶ
			item.filename = FileUtil(filename).Name();//�ļ���
			FileUtil(filename).Read(&item.content); //�ļ�����
			item.content_type = "application/octet-stream";//�ļ����ݸ�ʽ---��������
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
			//1.��ʼ������ʼ�����ݹ�����󣬴������Ŀ¼
			FileUtil(_backup_dir).MCreateDirectory();
			_data = new DataManager();
			while (1) {
				//2.����Ŀ¼����ȡĿ¼�������ļ�
				vector<string> arry;
				FileUtil(_backup_dir).ScanDirectory(&arry);
				//3.������ʷ������Ϣ�жϣ���ǰ�ļ��Ƿ���Ҫ����
				for (auto& a : arry) {
					if (IsNeedBackup(a) == false) {
						continue;
					}
					cout << a << "need backup!\n";
					//4.�����Ҫ�򱸷��ļ�
					bool ret = Upload(a);
					//5.��ӱ�����Ϣ
					_data->Insert(a);
					cout << a << "backup success!\n";
				}
				Sleep(10);
			}
		}
	};
}
