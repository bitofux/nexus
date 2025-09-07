#ifndef _NEXUS_UTILS_H_
#define _NEXUS_UTILS_H_

// 类型的定义

// 配置文件中每行的元素有两个,共同组成一个配置项
// 定义一个结构体用于保存配置项的内容
typedef struct config_item {
  char item_name[41];
  char item_content[401];
}config_item_t;

#endif
