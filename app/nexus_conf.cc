#include <cctype>
#include <cstddef>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../include/nexus_conf.hpp"

// 无参构造函数实现
Config::Config() {
    // 预分配空间
    _config_item_list.reserve(20);
}

// 析构函数实现
Config::~Config() { clear(); }

// 清空容器中配置项
void Config::clear() { _config_item_list.clear(); }
// 获取单例对象
Config& Config::getInstance() {
    static Config inst;
    return inst;
}

// 获取存储配置项列表的容器
std::vector<config_item_t>& Config::getItemList() { return _config_item_list; }

// 有界拷贝
void Config::copyBounded(
    char* dst, size_t dst_length, const char* src, const char* file, size_t lineno, bool is_key) {
    if (dst_length == 0) return;
    // 获取原字符串的大小(除了'\0')
    size_t src_length = std::strlen(src);
    // 如果src_length大于dst_length
    if (src_length > dst_length) {
        // 只拷贝dst_length - 1的字符,留一个补'\0'
        std::memcpy(dst, src, dst_length - 1);
        dst[dst_length - 1] = '\0';
        std::fprintf(stderr, "%s:%zu: %s, truncated to %zu bytes\n", file, lineno,
                     (is_key ? "key" : "value"), dst_length - 1);
    } else {  // 如果src_length小于dst_length,则直接连src
              // 中的'\0'字符一起拷贝
        std::memcpy(dst, src, src_length + 1);
    }
}

// 加载配置文件: 传入配置文件所在的路径(包含配置文件名称)
// 逐行读取文本,解析出key = value
// 将其存入到数据成员_config_item_list中
bool Config::Load(const char* config_file_name) {
    // 1.先清空上次存储的配置项
    clear();
    // 2.二进制只读方式打开文件
    FILE* fp = fopen(config_file_name, "rb");
    if (!fp) {
        std::perror("fopen");
        return false;
    }
    // 3.用户态缓冲区:用于读取存储数据和写入提供数据
    // lineno是行数,可以在警告时提醒第几行出现问题
    char line[512];
    size_t lineno = 0;
    // 4.逐行读取配置文件
    while (std::fgets(line, sizeof(line), fp)) {
        // 4.1 读取一行,行数+1
        ++lineno;

        // 4.2 检查超长行,一行被截断
        // 4.2.1 获取当前读取行的字符数(除了'\0')
        size_t L = std::strlen(line);
        // 4.2.2 判断这个行的末尾元素是否为'\n'
        // 如果不是代表文件中的一行没有被读取完毕且用户态
        // 缓冲区line被读满了 L == sizeof(line) - 1
        // 那么truncated就是true,代表此行被截断
        bool truncated = (L > 0 && line[L - 1] != '\n' && L == sizeof(line) - 1);
        // 4.2.3 判断是否为true,若为true,将超长行余下的字符
        // 全部丢掉,只保留当前读取到的511个字符
        if (truncated) {
            // 循环从当前文件中读取超长行剩余的字符,直到遇到EOF或者'\n'
            int c;
            while ((c = std::fgetc(fp)) != EOF && c != '\n') {
            }
        }

        // 4.3 去掉行尾的\r\n
        // strcspn函数是以第二个参数为基准,从line中下标为0
        // 的字符开始遍历,遇到\r或者\n字符时停止,并返回\r或
        //\n在line中的下标,如果没有\r或\n,返回line的长度
        // 随后将其设置为'\0'
        line[std::strcspn(line, "\r\n")] == '\0';

        // 4.4 去掉首尾空白
        // 4.4.1 去掉前面的空白:可以让"   key = value"也可读取
        char* p = line;
        // 当p未指向末尾且当前p指向的字符是一个空白字符
        //' ','\t' tab键盘,'\n'linux/unix换行,'\r','\v','\f'
        while (*p != '\0' && std::isspace((unsigned int)*p)) {
            ++p;  // 当前p指向的是前面无空白字符的字符串
        }
        // 4.4.2 去掉后面的空白结合上述的p指针
        // 计算去掉前面空白字符p指向的新字符串的长度
        size_t n = std::strlen(p);
        // 从末尾开始遍历确认是否是空白字符
        while (n && std::isspace((unsigned int)p[n - 1])) {
            // 如果是将其所在位置赋值为'\0'字符
            p[--n] = '\0';
        }

        // 4.5 到这里 p指向的字符串内容前后无空白字符
        // 跳过不用处理的行
        // 如果p指向的第一个字符还是'\0',那么此行是一个全空白
        if (*p == '\0') continue;
        if (*p == '[') continue;               // 段名[socket]
        if (*p == '#' || *p == ';') continue;  // 跳过以#和;开头的行

        // 4.6 此时p是一个带有有效数据的字符串,我们需要以
        //'='字符为界进行分割左右两边字符串
        // 如果这一行带有'=',equals指向以'='开头的字符串
        char* equals = std::strchr(p, '=');
        if (!equals) {
            std::fprintf(stderr, "%s:%zu: no '=';\n", config_file_name, lineno);
        }

        // 4.7 拆分 此时equals指向的字符串的第一个字符是'='
        // 将其赋值为'\0'便于拆分
        *equals = '\0';
        char* key = p;
        char* value = equals + 1;  // 跳过'\0'字符
        // 4.7.1 去掉key后面的空白,防止key "   "
        size_t nk = std::strlen(key);
        while (n && std::isspace((unsigned int)key[nk - 1])) {
            key[--nk] = '\0';
        }
        // 4.7.2 去掉value前面和后面的空白字符
        while (*value != '\0' && std::isspace((unsigned int)*value)) {
            ++value;
        }
        size_t nv = std::strlen(value);
        while (nv && std::isspace((unsigned int)value[nv - 1])) {
            value[--nv] = '\0';
        }

        // 4.8 检查是否为空键或者空值
        // 类似=123或者host=
        if (*key == '\0' || *value == '\0') {
            std::fprintf(stderr, "%s:%zu: empty key/value;\n", config_file_name, lineno);
            continue;
        }

        // 4.9 生成一个config_item_t类型的对象保存
        // key和value
        config_item_t item{};
        // 有界拷贝数据
        // 拷贝key到item_name
        copyBounded(item.item_name, sizeof item.item_name, key, config_file_name, lineno, true);
        // 拷贝value到item.content
        copyBounded(item.item_content, sizeof item.item_content, value, config_file_name, lineno,
                    false);

        // 4.10 存入到配置项列表中
        _config_item_list.emplace_back(item);
    }

    std::fclose(fp);
    return true;
}

// 根据配置项的名称获取对应的字符串
const char* Config::getString(const char* item_name) {
    for (auto& ref : _config_item_list) {
        if (strcasecmp(item_name, ref.item_name) == 0) {
            return ref.item_content;
        }
    }

    return nullptr;
}

// 根据配置项的名称获取对应的数字
int Config::getInt(const char* item_name, const int number) {
    for (auto& ref : _config_item_list) {
        if (strcasecmp(item_name, ref.item_name) == 0) {
            return std::atoi(ref.item_content);
        }
    }
    return number;
}
