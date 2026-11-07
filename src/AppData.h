//
// Created by ftwrjh on 2026/11/7.
//

#ifndef APP_DATA_H
#define APP_DATA_H

#include <string>
#include <unordered_map>

class AppData {
public:
    // 获取全局唯一实例
    static AppData& instance() {
        static AppData data;
        return data;
    }

    // 禁用拷贝（单例标准写法）
    AppData(const AppData&) = delete;
    AppData& operator=(const AppData&) = delete;

    // ===== 词库映射表 =====
    // 键: 汉字 (std::string 即 UTF-8)
    // 值: 编码 (如 "pbf")
    std::unordered_map<std::string, std::string> dict_map;

    // ===== 其他全局状态/缓存 =====
    // 比如当前加载的工作目录
    std::string current_workspace;

    // 比如词库是否已加载完毕的标志
    bool is_dict_loaded = false;

private:
    AppData() = default; // 私有构造
};

#endif // APP_DATA_H