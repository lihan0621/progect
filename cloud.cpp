//#include "util.hpp"
//#include "data.hpp"
#include "hot.hpp"
#include "server.hpp"
#include <thread>

void FileUtilTest()
{
  //cloud::FileUtil("./testdir/adir").CreateDirectory();
  //cloud::FileUtil("./testdir/a.txt").Writer("hello bit\n");
  //string body;
  //cloud::FileUtil("./testdir/a.txt").Read(&body);
  //cout << body << endl;
  //cout << cloud::FileUtil("./testdir/a.txt").Size() << endl;
  //cout << cloud::FileUtil("./testdir/a.txt").MTime() << endl;
  //cout << cloud::FileUtil("./testdir/a.txt").ATime() << endl;
  vector<string> array;
  cloud::FileUtil("./testdir").ScanDirectory(&array);
  for(auto& a : array) {
    cout << a << endl;
  }
}

void JsonTest()
{
  Json::Value val;
  val["姓名"] = "小明";
  val["性别"] = "男";
  val["年龄"] = 18;
  val["成绩"].append(77.5);
  val["成绩"].append(78.5);
  val["成绩"].append(79.5);

  string body;
  cloud::JsonUtil::Serialize(val, &body);
  cout << body << endl;

  Json::Value root;
  cloud::JsonUtil::UnSerialize(body, &root);
  cout << root["姓名"].asString() << endl;
  cout << root["性别"].asString() << endl;
  cout << root["年龄"].asInt() << endl;
  cout << root["成绩"][0].asFloat() << endl;
  cout << root["成绩"][1].asFloat() << endl;
  cout << root["成绩"][2].asFloat() << endl;
}

void CompressTest()
{
  cloud::FileUtil("./hello.txt").Compress("hello.zip");
  cloud::FileUtil("./hello.zip").UnCompress("bit.txt");
}

void DataTest() {
  cloud::DataManager data;
  vector<cloud::FileInfo> arry;
  data.SelectAll(&arry);
  for (auto &e : arry) {
    cout << e.filename << endl;
    cout << e.url_path << endl;
    cout << e.real_path << endl;
    cout << e.file_size << endl;
    cout << e.back_time << endl;
    cout << e.pack_flag << endl;
    cout << e.pack_path << endl;
  }
  /*
  data.Insert("./backup_dir/hello.txt");
  data.UpdateStatus("./backup_dir/hello.txt", true);
  vector<cloud::FileInfo> arry;
  data.SelectAll(&arry);
  for (auto &e : arry) {
    cout << e.filename << endl;
    cout << e.url_path << endl;
    cout << e.real_path << endl;
    cout << e.file_size << endl;
    cout << e.back_time << endl;
    cout << e.pack_flag << endl;
    cout << e.pack_path << endl;
  }
  cout << "--------------------------------" << endl;
  data.UpdateStatus("./backup_dir/hello.txt", false);
  cloud::FileInfo info;
  data.SelectOne("/download/hello.txt", &info);
  cout << info.filename << endl;
  cout << info.url_path << endl;
  cout << info.real_path << endl;
  cout << info.file_size << endl;
  cout << info.back_time << endl;
  cout << info.pack_flag << endl;
  cout << info.pack_path << endl;
  cout << "----------delete---------------" << endl;
  
  data.DeleteOne("/download/hello.txt");
  arry.clear();
  data.SelectAll(&arry);
  for (auto &e : arry) {
    cout << e.filename << endl;
    cout << e.url_path << endl;
    cout << e.real_path << endl;
    cout << e.file_size << endl;
    cout << e.back_time << endl;
    cout << e.pack_flag << endl;
    cout << e.pack_path << endl;
  }
  */
}

cloud::DataManager *_data;
void HotTest() {
  cloud::HotManager cloud;
  cloud.RunModule();
}

void ServerTest()
{
  cloud::Server srv;
  srv.RunModule();
}

int main()
{
  _data = new cloud::DataManager();
  //FileUtilTest();
  //JsonTest();
  //CompressTest();
  //DataTest();
  //HotTest();
  //ServerTest();
  thread hot_thread(HotTest);
  thread srv_thread(ServerTest);

  hot_thread.join();
  srv_thread.join();
  return 0;
}
