#pragma once 
//构建索引模块和搜索模块
#include<string>
#include<vector>
#include<unordered_map>
#include"./cppjieba/include/cppjieba/Jieba.hpp"
#include "./cppjieba/include/cppjieba/limonp/Logging.hpp"
namespace searcher
{
  struct DocInfo
  {
    uint64_t doc_id;
    std::string title;
    std::string content;
    std::string url;
  };
  struct Weight
  {
    uint64_t doc_id;//uint_64_t 不区分系统，必须占64位
    int weight;//权重，为了后面进行排序做准备
    //当前我们采用词频计算权重
  };

  //类型重命名，创建一个“倒排拉链类型”
  typedef std::vector<Weight> InvertedList;

  //通过这个类来描述索引模块
  class Index
  {
    private:
      //知道id获取到对应的文档内容
      //使用vector下标来表示文档id
      std::vector<DocInfo> forward_index_;

      //知道某个词获取到对应的id列表
      //每个词对应的id可能有很多，所以需要用vector封装
      std::unordered_map<std::string,InvertedList>inverted_index_;//查找时间复杂度为O1，比map的logn强
      cppjieba::Jieba jieba_;
    public:
      //读取output文件，在内存中构建索引
      bool Build(const std::string& input_path);
      //查正排，给定id读取文档内容
      const DocInfo* GetDocInfo(uint64_t doc_id)const;
      //查倒排，给定词，找到这个词在那些文档中出现过
      const InvertedList* GetInvertedList(const std::string& key)const;
      void CutWord(const std::string& input,std::vector<std::string>* output);
      Index();  
    private:
      const DocInfo* BuildForward(const std::string& line);
      void BuildInverted(const DocInfo& doc_info);
  };
  //搜索模块
  class Searcher
  {
    private:
      Index* index_;
    public:
      //加载索引
      bool Init();
//通过特定的格式再result字符串中表示搜索结果
      bool Search(const std::string& query,std::string* result);
  };
}//end searcher
