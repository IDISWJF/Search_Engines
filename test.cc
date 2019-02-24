//这个文件调用Index ，进行粗略验证
#include"searcher.hpp"
#include<iostream>
#if 0
int main()
{
  searcher::Index index;
  bool ret =  index.Build("./output");
  if(!ret)
  {
    std::cout<<"Build failes"<<std::endl;
    return 1;
  }
  auto* inverted_list = index.GetInvertedList("filesystem");
  if(inverted_list == nullptr)
  {
    std::cout<<"GetInvertedList failed!"<<std::endl;
    return 1;
  }
  for(auto weight : *inverted_list)
  {
    std::cout<<"id:"<<weight.doc_id<<"weight:"<<weight.weight<<std::endl;
    const auto* doc_info = index.GetDocInfo(weight.doc_id);
    std::cout<<"title: "<<doc_info->title<<std::endl;
    std::cout<<"url: "<<doc_info->url<<std::endl;
  }
  return 0;
}
#endif
int main()
{
//验证搜索模块是否正确
  searcher::Searcher searcher;
  bool ret = searcher.Init("output");
  if(!ret)
  {
return 1;
  }
  std::string query = "filesystem";
  std::string result ;
  searcher.Search(query,&result);
  std::cout<<"result:"<<result;
  return 0;
}
