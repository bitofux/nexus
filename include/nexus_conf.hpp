#ifndef _NEXUS_CONF_HPP_
#define _NEXUS_CONF_HPP_
#include <vector>
#include "../include/nexus_utils.h"


//声明一个单例类读取配置文件

class Config {
public:
  //C++11起局部静态对象的初始化是原子操作
  //且生命周期是第一次调用此函数时生成对象
  //在进程结束时调用一次析构函数
  static Config& getInstance();
  //加载配置文件
  bool Load(const char* config_file_name);
  //获取配置项中的名称获取对应的字符串
  const char* getString(const char* item_name);
  //获取配置项中的名称获取对应的数字
  int getInt(const char* item_name,const int number);
  //获取存储配置项列表的容器
  std::vector<config_item_t>& getItemList();
private:
  //有界拷贝
  void copyBounded(char* dst,size_t dstsz,const char* src,const char* file,size_t lineno,bool is_key);
  //清空容器中的配置项
  void clear();
private:
  //禁用拷贝 移动
  Config(const Config&) = delete;
  Config(Config&&) = delete;
  Config& operator=(const Config&) = delete;
  Config& operator=(Config&) = delete;
  Config();
  ~Config();
  std::vector<config_item_t> _config_item_list; 
};

#endif // !_NEXUS_CONF_HPP_
