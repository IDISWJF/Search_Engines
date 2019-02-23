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
#include"./util.hpp"

//给全局变量前面加一个g_
const std::string g_input_path = "../Search_Engines";
const std::string g_output_path = "../Search_Engines/output";
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

//除了标签之外的都认为是正文
//转移：
//< &lt;
//> &gt;
bool ParseContent(const std::string& html,std::string* content)
{
  //一个一个字符读取
  //如果当前字符是 < 认为标签开始，接下来就是字符舍弃
  //直到遇到 > 认为标签结束，接下来的字符就恢复

  //这个变量为true意味着当前处理正文
  //为false意味着当前在处理标签
  bool is_content = true;
  for(auto c : html)
  {
    if(is_content)
    {//当前为正文状态
      if(c ==  '<')
      {
        //进入标签
        is_content = false;
      }
      else 
      {
        //当前字符就是普通的正文字符，需要加入结果中
        if(c == '\n')
          c = ' ';//此处把换行替换为空格，为了最终的行文本文件
        content->push_back(c);
      }
    }
    else 
    {
      //当前是标签状态
      if(c == '>')
        is_content = true;
    }
  }
  return true;
}

bool ParseTitle(const std::string& html,std::string* title)
{
  //从html中的title标签中提取标题
  //1：先查找<title>标签
  size_t beg = html.find("<title>");
  if(beg == std::string::npos)
  {
    std::cout<<"<title> not found!"<<std::endl;
    return false;
  }
  //2：再查找</title>标签
  size_t end = html.find("</title>");
  if(end == std::string::npos)
  {
    std::cout<<"</title not found!>"<<std::endl;
    return false;
  }
  //3：通过字符串提取子串的方式获取到title
  beg += std::string("<title>").size();
  if(beg > end)//没有标题也是一种可能所以是>
  {
    std::cout<<"beg end error !"<<std::endl;
    return false;
  }
  *title = html.substr(beg,end - beg);
  return true;
}

//boost文档有有个统一的前缀
//https://www.boost.org/doc/libs/1_59_0/
//URL后半部分可以通过该文档的路劲中解析出来
//文档的路径形如
//../Search_Engines/html/xpressive.html
//需要的后缀形式：
// html/xpressive.html
bool ParseUrl(const std::string& file_path,std::string* url)
{
  std::string prefix = "https://www.boost.org/doc/libs/1_59_0/";
  std::string tail = file_path.substr(g_input_path.size());
  *url = prefix +tail;
  return true;
}

bool ParseFile(const std::string& file_path ,DocInfo* doc_info)
{
  //1：打开文件，读取文件内容
  std::string html;
  bool ret =FileUtil:: Read(file_path,&html);
  if(!ret)
  {
    std::cout<<"Read file failed! file_path="
      <<file_path<<std::endl; 
    return false;
  }
  //2：解析标题
  ret = ParseTitle(html,&doc_info->title);
  if(!ret)
  {
    std::cout<<"ParseTitle failed! file_path="
    <<file_path<<std::endl;
    return false;
  }
  //3：解析正文，并且取出html标签
  ret = ParseContent(html,&doc_info->content);
  if(!ret)
  {
    std::cout<<"ParseContent failed! file_path="
      <<file_path<<std::endl;
    return false;
  }
  //4：取出url
  ret = ParseUrl(file_path,&doc_info->url);
    if(!ret)
    {
      std::cout<<"parseUrl failed file_path="
        <<file_path<<std::endl;
      return false;
    }
return true;
}

void WriteOutput(const DocInfo& doc_info, std::ofstream& file)
{
  //c++中的iostream 和 fstream 等这些对象都是禁止拷贝的，所以要传引用
  //最终的输出结果是一个行文本文件，每一行对应到一个html文档
  //也就是每一行对应一个 doc_info 
  //"\3"ascll码表中值为3的不可见字符
  //使用这些特殊字符进行分割
  std::string line = doc_info.title + "\3"
    + doc_info.url + "\3" +  doc_info.content + "\n";
  file.write(line.c_str(),line.size());
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
  //TODO验证 Enum是不是正确
 // for(const auto& file_path : file_list)
 // {
 // std::cout<<file_path<<std::endl;
 //}

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
  for(const auto& file_path : file_list)
  {
    DocInfo info;
    //输入的是当前要解析的文件路径
    //输出的是解析之后得到的DocInfo结构
    ret = ParseFile(file_path,&info);
    if(!ret)
    {
      std::cout<<"ParseFile failed!file_path="<<file_path
      <<std::endl;
      continue;
    }
    //3： 把分析结果按照一行的形式写入到输出文件中
    WriteOutput(info,output_file);
  }
  output_file.close();
  return 0;
}


