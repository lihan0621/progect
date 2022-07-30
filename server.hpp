#ifndef __MY_SERVER__
#define __MY_SERVER__
#include "data.hpp"
#include "httplib.h"
#include <sstream>
using namespace std;

extern cloud::DataManager *_data;
namespace cloud {
  class Server {
    private:
      int _srv_port = 9090;//服务器的绑定监听端口
      string _url_prefix = "/download/";
      string _backup_dir = "./backup_dir/";//上传文件的备份存储路径
      httplib::Server _srv;
    private:
      static void Upload(const httplib::Request &req, httplib::Response &rsp) {
        string _backup_dir = "./backup_dir/";//上传文件的备份存储路径
        //判断有没有对应标识的文件上传区域数据
        if (req.has_file("file") == false) {//判断有没有name字段值是file的标识的区域
          cout << "Upload file data format error!\n";
          rsp.status = 400;
          return ;
        }
        //获取解析后的区域数据
        httplib::MultipartFormData data = req.get_file_value("file");
        //cout << data.filename << endl;//如果是文件上传则保存文件名称
        //cout << data.content << endl;//区域正文数据，如果是文件上传则是文件内容数据
        //组织文件的实际存储路径名
        string  realpath = _backup_dir + data.filename;
        //向文件中写入数据，实际上就是把文件备份起来了
        if (FileUtil(realpath).Writer(data.content) == false) {
          cout << "back file failed!\n";
          rsp.status = 500;
          return ;
        }
        //新增备份信息
        if (_data->Insert(realpath) == false) {
          cout << "insert back info failed!\n";
          rsp.status = 500;
          return ;
        }
        rsp.status = 200;
        cout << "new backup :" << realpath << endl;
        return ;
      }
      static string StrTime(time_t t) {
        return asctime(localtime(&t));
      }
      static void List(const httplib::Request &req, httplib::Response &rsp) {
        //获取所有历史备份信息，并且根据这些信息组织出来一个html页面，作为响应正文
        vector<FileInfo> arry;
        if (_data->SelectAll(&arry) == false) {
          cout << "select all back info failed!\n";
          rsp.status = 500;
          return ;
        }
        stringstream ss;
        ss << "<html>";
        ss << "<head>";
        ss << "<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
        ss << "<title>Download</title>";
        ss << "</head>";
        ss << "<body>";
        ss << "<h1>Download</h1>";
        ss << "<table>";
        for (auto &a : arry) {
          //组织每一行的页面标签
          ss << "<tr>";
          //<td><a href="/download/test.txt">test.txt</a></td>
          ss << "<td><a href='" << a.url_path << "'>" << a.filename << "</a></td>";
          //<td align="right"> 2021-12-29 10:10:10 </td>
          ss << "<td align='right'>" << StrTime(a.back_time) << "</td>";
          ss << "<td align='right'>" << a.file_size / 1024 << " KB </td>";
          ss << "</tr>";
        }
        ss << "</table>";
        ss << "</body>";
        ss << "</html>";
        rsp.set_content(ss.str(), "text/html");
        rsp.status = 200;
        return ;
      }
      static string StrETag(const string &filename) {
        //etag是一个文件的唯一标识，当文件被修改则会发生改变
        //这里etag不用内容计算：文件大小-文件最后修改时间
        time_t mtime = FileUtil(filename).MTime();
        size_t fsize = FileUtil(filename).Size();
        stringstream ss;
        ss << fsize << "-" << mtime;
        return ss.str();
      }
      static void Download(const httplib::Request &req, httplib::Response &rsp) {
        FileInfo info;
        if (_data->SelectOne(req.path, &info) == false) {
          cout << "select one back info failed!\n";
          rsp.status = 404;
          return ;
        }
        //如果文件已经被压缩了，则要先解压缩，然后再去原文件读取数据
        if (info.pack_flag == true) {
          FileUtil(info.pack_path).UnCompress(info.real_path);
        }
        if (req.has_header("If-Range")) {
          //using Range = std::pair<ssize_t, ssize_t>;
          //using Ranges = std::vector<Range>;
          string old_etag = req.get_header_value("If-Range");
          string cur_etag = StrETag(info.real_path);
          if (old_etag == cur_etag) {//文件没有改变可以断点续传
            //如果我们自己处理进行字符串解析得到起始和结束位置就行
            //size_t start = req.Ranges[0].first;//但是httplib已经替我们解析了
            //size_t end = req.Ranges[0].second;//如果没有end数字，则表示文件末尾
            //httplib会将second设置为-1，这时候从文件start位置开始读取end-start+1长度，如果end是-1，则是文件长度-start长度
            //因为假设1000长度的文件，请求900-999，则返回包含900和999在内总共100长度的数据
            //而如果请求900-，1000长度末尾位置其实就是999，直接长度减去900就可以了
            //
            //httplib已经替我们完成了断点续传的功能，我们只需要将文件的所有数据放到body中，
            //然后设置响应状态码206，httplib检测到的响应状态码是206，就会从body中截取指定区间的数据进行响应
            FileUtil(info.real_path).Read(&rsp.body);
            rsp.set_header("Content-Type", "application/octet-stream");//设置正文类型为二进制流
            rsp.set_header("Accept-Ranges", "bytes");//告诉客户端我支持断点续传
            //rsp.set_header("Content-Range", "bytes start-end/fsize");//httplib会自动设置
            rsp.set_header("ETag",cur_etag);
            rsp.status = 206;
            return ;
          }
        }
        FileUtil(info.real_path).Read(&rsp.body);
        rsp.set_header("Content-Type", "application/octet-stream");//设置正文类型为二进制流
        rsp.set_header("Accept-Ranges", "bytes");//告诉客户端我支持断点续传
        rsp.set_header("ETag",StrETag(info.real_path)); 
        rsp.status = 200;
        return ;
      }
    public:
      Server() {
        FileUtil(_backup_dir).CreateDirectory();

      }
      bool RunModule() {
        //搭建http服务器
        //建立请求-处理函数映射关系
        //Post(请求的资源路径，对应的业务处理回调函数)；
        _srv.Post("/upload", Upload);
        _srv.Get("/list", List);//这是一个展示页面的请求
        string regex_download_path = _url_prefix + "(.*)";
        _srv.Get(regex_download_path, Download);
        //启动服务器
        _srv.listen("0.0.0.0", _srv_port);
        return true;
      }
  };
}


#endif
