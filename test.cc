//这个文件调用Index ，进行粗略验证
#include"searcher.hpp"
#include<iostream>
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
