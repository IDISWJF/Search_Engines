#include"searcher.hpp"
#include<iostream>
#include<fstream>
#include"util.hpp"
#include<algorithm>
#include<jsoncpp/json/json.h>
namespace searcher
{
  ///////////索引模块实现////////////////
  const char* const DICT_PATH = "./jieba_dict/jieba.dict.utf8";
  const char* const HMM_PATH = "./jieba_dict/hmm_model.utf8";
  const char* const USER_DICT_PATH = "./jieba_dict/user.dict.utf8";
  const char* const IDF_PATH = "./jieba_dict/idf.utf8";
  const char* const STOP_WORD_PATH = "./jieba_dict/stop_words.utf8";

  Index::Index() : jieba_(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
  {

  }
  //先实现两简单的函数
  const DocInfo* Index::GetDocInfo(uint64_t doc_id)const
  {
    if(doc_id >= forward_index_.size() )
    {
      return nullptr;
    }
    return &forward_index_[doc_id];
    //最好返回指针，防止多次拷贝
    //不能用引用只因为防止上面无效指针的情况
  }
  const InvertedList* Index::GetInvertedList(const std::string& key)const 
  {
    // std::unordered_map<std::string,InvertedList>::const_iterator
    auto pos = inverted_index_.find(key);
    if(pos == inverted_index_.end())
    {
      //没找到
      return nullptr;
    }
    //unorder_map 迭代器指向的数据类型是<key,value>
    return &pos->second;  
  }

  bool Index::Build(const std::string& input_path)
  {
    std::cout<<"Index build Start!"<<std::endl;
    //1：按行读取文件内容（每一行对应一个文档）
    std::ifstream file(input_path.c_str());
    if(!file.is_open())
    {
      std::cout<<"input_path open failed! input_path="
        <<input_path<<std::endl;
      return false;
    }
    std::string line;//每次按行读取，不包含结尾‘\n’
    while(std::getline(file,line))
    {
      //2：构造DocInfo对象，更新正排索引数据
      //  对读到的一行文件进行解析，得到的DocInfo对象在插入vector
      const DocInfo* doc_info = BuildForward(line);
      //3：更新倒排索引数据
      BuildInverted(*doc_info);
      if(doc_info->doc_id % 300 == 0)
      {
        std::cout<<"build doc_id = "
          <<doc_info->doc_id<<std::endl;
      }
    }
    std::cout<<"Index BUild Finish"<<std::endl;
    file.close();
    return true;
  }

  const DocInfo* Index::BuildForward(const std::string& line)
  {
    //1：对这一行内容进行切分（\3）
    std::vector<std::string>tokens;//存放结果
    //切分方式：strtok破坏原字符串
    //stringstream 比较麻烦
    //借助boost进行切分
    StringUtil::Split(line,&tokens,"\3");
    if(tokens.size() != 3)
    {
      std::cout<<"tokens not ok"<<std::endl;
      return nullptr;
    }
    //2：构造一个DocInfo对象
    DocInfo doc_info;
    doc_info.doc_id = forward_index_.size();
    doc_info.title = tokens[0];
    doc_info.url = tokens[1];
    doc_info.content = tokens[2];
    //3：把这个对象插入到正排索引中
    forward_index_.push_back(doc_info);
    //return &doc_info;不能返回局部变量
    return &forward_index_.back();
  }
  void Index::BuildInverted(const DocInfo& doc_info)
  {
    //1：先对当前的doc_info 进行分词，对作为分词，对标题分词
    std::vector<std::string> title_tokens;
    CutWord(doc_info.title,&title_tokens);
    std::vector<std::string> content_tokens;
    CutWord(doc_info.content,&content_tokens);
    //2：对doc_info 中的标题和正文进行词频统计
    //    当前词在标题中和正文中出现在几次（分开统计是因为权重不同）
    struct WordCnt
    {
      int title_cnt;
      int content_cnt;
    };
    //用hash完成词频统计
    std::unordered_map<std::string,WordCnt> word_cnt;
    for(std::string word:title_tokens)
    {
      //hello HELLO 大小写算一个词出现两次
      boost::to_lower(word);//修改原来字符串，变为小写
      ++word_cnt[word].title_cnt;//存在++，不存在新建
    }
    for(std::string word : content_tokens)
    {
      boost::to_lower(word);
      ++word_cnt[word].content_cnt;
    }
    //3：遍历分词结果，在倒排索引中查找
    for(const auto& word_pair : word_cnt)
    {
      Weight weight;
      weight.doc_id = doc_info.doc_id;
      weight.weight = 10 * word_pair.second.title_cnt  + word_pair.second.content_cnt;
      weight.key = word_pair.first;//把这个词记录到Weight中方便后面使用
      //4：如果该分词结果在倒排中不存在，就构建新的键值对
      //5：如果该分词结果在倒排中存在，找到对应的值（vector）构建一个新的
      InvertedList& inverted_list = inverted_index_[word_pair.first];
      inverted_list.push_back(weight); 
      //    weight对象插入到vector中
    }
    return ;
  }
  void Index::CutWord(const std::string& input,std::vector<std::string>* output)
  {
    jieba_.CutForSearch(input,*output);
  }
  //////////////以下是搜索模块实现////////////////
  bool Searcher::Init(const std::string& input_path)
  {
    return index_->Build(input_path);
  }

  bool Searcher::Search(const std::string& query,std::string* json_result)
  {
    //1分词：对查询词进行分词
    std::vector<std::string>tokens;
    index_->CutWord(query,&tokens);
    //2触发：针对分词结果查倒排索引，找到那些文件是有相关性的
    std::vector<Weight> all_token_result;
    for(std::string word : tokens)
    {
      boost::to_lower(word);
      auto* inverted_list = index_->GetInvertedList(word);
      if(inverted_list == nullptr)
      {
        continue;//不能因为某个词在索引中不存在就影响其他查询
      }
      //此处进一步改进是考虑不同的分词结果对应相同文档id的情况
      //此时需要去重，和权重合并
      //实现思路类似于有序链表合并
      all_token_result.insert(all_token_result.end(),
          inverted_list->begin(),inverted_list->end());//把invertered_list结果尾插
    }
    //3:排序：把这些结果按照一定规则排序
    //sort第三个参数可以使用 仿函数/函数指针/lambda表达式
    //lambda表达式就是一个匿名函数
    //将匿名函数赋给变量，在使用变量时就调用匿名函数
    std::sort(all_token_result.begin(),all_token_result.end(),
        [](const Weight& w1,const Weight& w2){
        return w1.weight > w2.weight;
        });    
    //4构造结果：查正排，找到每个搜索结果的标题，正文，url 
    //预期构造成的结果形如
    //[
    //{
    //"title":"标题"
    //"desc" : "描述"
    //"url" : "url"
    //}
    //]
    Json::Value results;//表示所有的搜索结果JSON对象
    for(const auto& weight : all_token_result)
    {
      const auto* doc_info = index_->GetDocInfo(weight.doc_id);
      if(doc_info == nullptr)
      {
        continue;
      }
      //如何构造成JSON结构？有第三方库实现 jsoncpp
      Json::Value result;//表示一条搜索结果的JSON对象
      result["title"] = doc_info->title;
      result["url"] = doc_info->url;
      result["content"] = GetDesc(doc_info->content,weight.key);
      results.append(result); 
    }
    //借助jsoncpp能快速完成JSON对象和字符串的转换
    Json::FastWriter writer;
    *json_result = writer.write(results);
    return true;
  }
  std::string Searcher::GetDesc(const std::string& content,const std::string& key)
  {
    //描述是正文的一部分，描述词最好要包含查询词
    //1：先在正文中查找词位置
    size_t pos = content.find(key);
    if(pos == std::string::npos)
    {
      //该词在正文中不存在，只在标题中出现
      //此时直接从开头截取一段即可
      //100是随机选择的
      if(content.size() < 100)
      {
        return content;
      }
      else 
      {
        return content.substr(0,100) + "...";
      }
    }

    //2：往前截取一段，往后截取一段
    //已该词为基准，往前截取50个字节，往后截取50个字节
    size_t beg = pos < 50 ? 0 : pos - 50;
    if(beg + 100 >= content.size())
    {
      //beg之后长度不足100，就把剩余的内容算作描述
      return content.substr(beg);
    }
    else
    {
      return content.substr(beg,160) + "...";
    }

  }
}//end searcher 
