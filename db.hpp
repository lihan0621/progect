#include <iostream>
#include <fstream>
#include <mutex>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>

using namespace std;
namespace vod_system {
#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASS "123456"
#define MYSQL_NAME "vod_system"
	static MYSQL *MysqlInit(){
		MYSQL *mysql = mysql_init(NULL);
		if(mysql == NULL) {
			cout << "mysql init failed!\n";
			return NULL;
		}
		if(mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USER, 
					MYSQL_PASS, MYSQL_NAME, 0, NULL, 0) == NULL) {
			cout << mysql_error(mysql) << endl;
			mysql_close(mysql);
			return NULL;
		}
		if(mysql_set_character_set(mysql, "utf8") != 0) {
			cout << mysql_error(mysql) << endl;
			mysql_close(mysql);
			return NULL;
		}
		return mysql;
	}
	static void MysqlRelease(MYSQL *mysql) {
		if(mysql != NULL) {
			mysql_close(mysql);
		}
		return;
	}
	static bool MysqlQuery(MYSQL *mysql, const string &sql){
		int ret = mysql_query(mysql, sql.c_str());
		if (ret != 0) {
			cout << sql << endl;
			cout << mysql_error(mysql) << endl;
			return false;
		}
		return true;
	}
	class TableVod{
		private:
			MYSQL *_mysql;
			mutex _mutex;
		public:
			TableVod(){
				//句柄初始化连接服务器
				_mysql = MysqlInit();
				if(_mysql == NULL) {
					exit(0);
				}
			}
			~TableVod(){
				//句柄销毁
				MysqlRelease(_mysql);
			}
			bool Insert(const Json::Value &video){
				const char *name = video["name"].asCString();
				const char *vdesc = video["vdesc"].asCString();
				const char *video_url = video["video_url"].asCString();
				const char *image_url = video["image_url"].asCString();
				char sql[4096] = {0};
#define VIDEO_INSERT "insert tb_video values(null, '%s', '%s', '%s', '%s', now());"
				sprintf(sql, VIDEO_INSERT, name, vdesc, video_url, image_url);
				return MysqlQuery(_mysql, sql);
			}
			bool Delete(int video_id) {
#define VIDEO_DELETE "delete from tb_video where id=%d;"
				char sql[4096] = {0};
				sprintf(sql, VIDEO_DELETE, video_id);
				return MysqlQuery(_mysql, sql);
			}
			bool Update(int video_id, const Json::Value &video) {
#define VIDEO_UPDATE "update tb_video set name='%s', vdesc='%s', where id=%d;"
				char sql[4096] = {0};
				sprintf(sql, VIDEO_UPDATE,
						video["name"].asCString(),
						video["vdesc"].asCString(),
						video_id);
				return MysqlQuery(_mysql, sql);
			}
			bool GetAll(Json::Value *video) {
#define VIDEO_GETALL "select * from tb_video;"
				_mutex.lock();
				bool ret = MysqlQuery(_mysql, VIDEO_GETALL);
				if(ret == false) {
					_mutex.unlock();
					return false;
				}
				MYSQL_RES *res = mysql_store_result(_mysql);
				_mutex.unlock();
				if(res == NULL) {
					cout << "store result failed!\n";
					return false;
				}
				int num = mysql_num_rows(res);
				for (int i=0; i<num; i++) {
					MYSQL_ROW row = mysql_fetch_row(res);
					Json::Value val;
					val["id"] = stoi(row[0]);
					val["name"] = row[1];
					val["vdesc"] = row[2];
					val["video_url"] = row[3];
					val["image_url"] = row[4];
					val["ctime"] = row[5];
					video->append(val); 
				}
				mysql_free_result(res);
				return true;
			}
			bool GetOne(int video_id, Json::Value *video) {
#define VIDEO_GETONE "select * from tb_video where id=%d;"
				char sql_str[4096] = {0};
				sprintf(sql_str, VIDEO_GETONE, video_id);
				_mutex.lock();
				bool ret = MysqlQuery(_mysql, sql_str);
				if (ret == false) {
					_mutex.unlock();
					return false;
				}
				MYSQL_RES *res = mysql_store_result(_mysql);
				_mutex.unlock();
				if (res == NULL) {
					cout << mysql_error(_mysql) << endl;
					return false;
				}
				int	num_row = mysql_num_rows(res);
				if (num_row != 1) {
					cout << "getone result error\n";
					mysql_free_result(res);
					return false;
				}
				MYSQL_ROW row = mysql_fetch_row(res);
				(*video)["id"] = video_id;
				(*video)["name"] = row[1];
				(*video)["vdesc"] = row[2];
				(*video)["video_url"] = row[3];
				(*video)["image_url"] = row[4];
				(*video)["ctime"] = row[5];
				mysql_free_result(res);
				return true;
			}
	};
	class util{
		public:
		static bool WriteFile(const string &name, const string &content)
		{
			ofstream of;
			of.open(name, ios::binary);
			if (!of.is_open()) {
				cout << "open file failed!\n";
				return false;
			}
			of.write(content.c_str(), content.size());
			if (!of.good()) {
				cout << "write file failed!\n";
				return false;
			}
			of.close();
			return true;
		}
	};
}
