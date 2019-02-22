//一：数据处理模块
//把boost文档中涉及到的html进行处理：
//1.去标签
//2.把文件进行合并
//  把文档中涉及到的n个HTML的内容进行合并成一个行文本文件
//  生成的结果是一个大文件，里面包含很多行，每一行对应boost
//  文档中的一个html，这么做的目的是为了让后面的索引模块
//  处理起更方便
//3.对文档的结构进行分析，提取出文档的标题，正文，目标url

#include<iostream>
#include<vector>
#include<string>
#include<fstream>
//为了进行目录的遍历和枚举所用到的模块
#include<boost/filesystem/path.hpp>
#include<boost/filesystem/operations.hpp>
//给全局变量前面加一个g_
const std::string g_input_path = "../";
const std::string g_output_path = "../";
//doc 指的是文档，也就是带搜索的html
struct DocInfo
{
  std::string title;
  std::string content;
  std::string url;
};

bool EnumFile(const std::string& input_path,std::vector<std::string>* file_list)
{
  namespace fs = boost::filesystem;
  //input_path 是一个字符串，根据这个字符串构造出一个path对象
  fs::path root_path(input_path);
  if(!fs::exists(root_path))
  {
    std::cout<<"input_path not exist! input_path="
      <<input_path<<std::endl;
    return false;
  }
  //boost 递归遍历目录，借助一个特殊的迭代器即可
  //下面构造一个未初始化的迭代器作为遍历结束标志
  fs::recursive_directory_iterator end_iter;
  for(fs::recursive_directory_iterator iter(root_path); iter!=end_iter;++iter)
  {
    //1: 此处应该剔除目录
    if(!fs::is_regular_file(*iter))//判断是不是普通文件
    {
continue;
 }
    //2: 根据扩展名只保留html
    if(iter->path().extension() != ".html")//判断扩展名是不是html
    {
continue;
    }
    file_list->push_back(iter->path().string());
  }
  return true;
}


int main()
{
  //1：枚举出输入路径中所有的html文档的路径
  //vector中的每个元素就是一个文件路径
  ///home/wjf/Search_Engines/htmltypeof.html
  //...
  std::vector<std::string> file_list;
  bool ret = EnumFile(g_input_path,&file_list);
  if(!ret)
  {//如果搜索名称不存在就会失败
    std::cout<<"EnumFile failed!"<<std::endl;
    return 1;
  }
  //TOMO验证 Enum是不是正确
  for(const auto& file_path : file_list)
  {
    std::cout<<file_path<<std::endl;
  }
#if 0
  std::ofstream output_file(g_output_path.c_str());//ofstream从内存到硬盘，只能接受c风格字符串
  if(!output_file.is_open())
  {
    std::cout<<"open output_file failed! g_output_path="
      <<g_output_path<<std::endl;
    return 1;
  }
  //2: 依次处理每个枚举出的路径，对该文件进行分析，
  //   分析出文件的标题/正文/url，并且进行去标签
  //for(size_t i = 0;i<file_list.sze();++i)
  //c++11 提出的一种 range based for
  //for(const std::string& path:file_list)
  for(const auto& path : file_list)
  {
    DocInfo info;
    //输入的是当前要解析的文件路径
    //输出的是解析之后得到的DocInfo结构
    ret = ParseFile(file_path,&info);
    if(!ret)
    {
      std::cout<<"ParseFile failed!file_path="<<file_path;
      <<std::endl;
      continue;
    }
    //3： 把分析结果按照一行的形式写入到输出文件中
    WriteOutput(info,output_file);
  }
  output_file.close();
#endif
  return 0;
}


