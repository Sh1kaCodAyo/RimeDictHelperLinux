//
// Created by ftwrjh on 2026/11/7.
//

#include <string>
#include <vector>
#include <unordered_map>

// 定义词条来源的强类型枚举
enum class DictSource
{
    BASE = 0, // 基本词库
    USER = 1 // 用户词库
};

// 词条结构
struct DictEntry
{
    std::string text;
    std::string code;
    int weight;
    DictSource source; // 使用枚举类型
    int line_number;

    DictEntry(std::string t, std::string c, int w, DictSource s, int line)
        : text(std::move(t)), code(std::move(c)), weight(w), source(s), line_number(line)
    {
    }
};

class DictDatabase
{
public:
    // ================= 数据结构 =================

    // 1. 单字 -> 全码 (比如 "我" -> "trnt")
    std::unordered_map<std::string, std::string> single_char_codes;

    // 2. 编码 -> 词条列表 (处理重码)
    // 使用 std::vector<DictEntry> 存储属于同一个编码的所有词
    std::unordered_map<std::string, std::vector<DictEntry>> code_to_entries;

    // ================= 零散配置 =================
    std::string workspace_path; // 工作目录
    // 如果还有其他全局状态，都可以加在这里，比如：
    // bool auto_sync_enabled = false;


    // 在头文件中只写声明
    void add_entry(const DictEntry& entry);
    const std::vector<DictEntry>* get_entries_by_code(const std::string& code);
};

extern bool load_dict_file(const std::string& filepath, DictSource source_type, DictDatabase& db);
