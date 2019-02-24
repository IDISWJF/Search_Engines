#pragma once 
#include<string>
#include<fstream>
#include<vector>
#include<boost/algorithm/string.hpp>
class FileUtil
{
  public:
    static bool Read(const std::string& file_path,std::string* content)
    {
      std::ifstream file(file_path.c_str());
      if(!file.is_open())
      {
        return false;
      }
      //按行读取
      std::string line;
      while(std::getline(file,line))
      {
        *content += line + "\n";//对content解引用得到string对象+=赋值
      }
      file.close();
      return true;
    }

    static bool write(const std::string& file_path,const std::string& content)
    {
      std::ofstream file(file_path.c_str());
      if(!file.is_open())
      {
        return false;
      }
      file.write(content.c_str(),content.size());//1缓冲区的起始地址2写入内容长度
        file.close();
      return true;
    }
};

class StringUtil
{
public:
  //基于boost的字符串切分实现
  //aaa\3bbb\3ccc => 3
  //aaa\3\3bbb    =>2
  static void Split(const std::string& input,std::vector<std::string>* output,const std::string& split_char)
  {
    boost::split(*output,input,boost::is_any_of(split_char),boost::token_compress_off);
}
};
