/*************************************************************************
    > File Name: test.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 05 Jan 2022 02:27:12 PM CST
 ************************************************************************/

#include<iostream>
#include <boost/algorithm/string.hpp>
using namespace std;
int main()
{
	string str1 = "I love HaiCoder and i learn C++ from HaiCoder";
	cout << str1 << endl;
	string s = "zhangwenchao";
	boost::algorithm::replace_all(str1, "love", s);
	cout << str1 << endl;
	return 0;
}

