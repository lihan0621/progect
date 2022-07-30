#include "data.hpp"
#include "client.hpp"

void DataTest()
{
	/*
	cloud::DataManager* _data = new cloud::DataManager();
	cloud::FileUtil("./hello.txt").Writer("leihaoa~laotie~!!\n");
	cloud::FileUtil("./hi.txt").Writer("hello~xiangdang~!!\n");
	_data->Insert("./hello.txt");
	_data->Insert("./hi.txt");
	vector<pair<string, string>> arry;
	cout << "---------insert&all---------" << endl;
	_data->SelectAll(&arry);
	for (auto& a : arry) {
		cout << a.first << "<==>" << a.second << endl;
	}

	cout << "---------update---------" << endl;
	cloud::FileUtil("./hi.txt").Writer("shbcdisnibiweshcos~!!\n");
	_data->Update("hi.txt");

	string etag;
	_data->SelectOne("./hi.txt", &etag);
	cout << "hi.txt <==> " << etag << endl;

	cout << "---------delete---------" << endl;
	_data->Delete("./hello.txt");
	arry.clear();
	_data->SelectAll(&arry);
	for (auto& a : arry) {
		cout << a.first << "<==>" << a.second << endl;
	}
	*/
	cloud::DataManager* _data = new cloud::DataManager();
	vector<pair<string, string>> arry;
	_data->SelectAll(&arry);
	for (auto& a : arry) {
		cout << a.first << "<==>" << a.second << endl;
	}
	return;
}

int main()
{
	//DataTest();
	cloud::Client client("192.168.19.130", 9090);
	client.RunModule();
	return 0;
}