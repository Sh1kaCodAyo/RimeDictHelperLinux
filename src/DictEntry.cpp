//
// Created by ftwrjh on 2026/11/7.
//
#include "DictEntry.h"

#include <iostream>
#include <fstream>
#include <sstream>

// 添加一条词条到内存
// void DictDatabase::add_entry(const DictEntry& entry) {
//     // 如果是单字，且单字表里没有，则记录全码
//     // 注意：这里的中文长度在 UTF-8 下占 3 个字节，所以单字 length() 应该是 3
//     if (entry.text.length() == 3 && single_char_codes.find(entry.text) == single_char_codes.end()) {
//         single_char_codes[entry.text] = entry.code;
//     }
//
//     // 将其加入对应的编码列表
//     code_to_entries[entry.code].push_back(entry);
// }

void DictDatabase::add_entry(const DictEntry& entry) {
    // 1. 判断是否是单字（或 Emoji 等单字符）
    // 既然我们可能输入 Emoji，就不能死板地要求 length() == 3。
    // 一个 UTF-8 字符的长度在 1 到 4 字节之间，只要 <= 4，我们就把它当“单字/单符号”处理
    if (entry.text.length() <= 4) {
        // 查找该字是否已经存在于映射表中
        auto it = single_char_codes.find(entry.text);

        if (it == single_char_codes.end()) {
            // 第一次遇到这个字，直接记录
            single_char_codes[entry.text] = entry.code;
        } else {
            // 如果已经存在，比对编码长度，保留最长的那个（全码）
            if (entry.code.length() > it->second.length()) {
                it->second = entry.code;
            }
        }
    }

    // 2. 将其加入对应的编码列表（重码表）
    code_to_entries[entry.code].push_back(entry);
}

// 获取某个编码下的所有重码词条
const std::vector<DictEntry>* DictDatabase::get_entries_by_code(const std::string& code) {
    auto it = code_to_entries.find(code);
    if (it != code_to_entries.end()) {
        return &(it->second);
    }
    return nullptr;
}
// 将解析逻辑独立成一个函数
bool load_dict_file(const std::string& filepath, DictSource source_type, DictDatabase& db) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filepath << std::endl;
        return false;
    }

    std::string line;
    int line_number = 0;
    bool parsing_entries = false; // 状态标志：是否已经越过了 ...

    while (std::getline(file, line)) {
        line_number++;

        // 1. 处理 Windows 换行符遗留的 '\r' (防止读取出现诡异字符)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // 2. 检查是否到了分隔符
        if (!parsing_entries) {
            if (line == "...") {
                parsing_entries = true; // 遇到三个点，切换状态，下面开始解析词条
            }
            continue; // 头部信息直接跳过
        }

        // 3. 跳过正文中的空行（或者注释行）
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 4. 解析正文：词条 \t 编码 \t 权重
        std::stringstream ss(line);
        std::string text, code, weight_str;

        // std::getline 的第三个参数是分隔符，这里用 '\t'
        if (std::getline(ss, text, '\t') &&
            std::getline(ss, code, '\t') &&
            std::getline(ss, weight_str, '\t')) {

            int weight = 0;
            try {
                weight = std::stoi(weight_str);
            } catch (...) {
                weight = 20; // 如果解析权重失败（比如某些行没写权重），默认给个 20
            }

            // 5. 构建对象并加入内存数据库
            DictEntry entry(text, code, weight, source_type, line_number);
            db.add_entry(entry);
            }
    }

    std::cout << "成功加载 " << filepath << "，总计 " << db.code_to_entries.size() << " 个编码" << std::endl;
    return true;
}
// 删除文件指定行
bool DictDatabase::delete_line_from_file(const std::string& filepath, int target_line) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::vector<std::string> lines;
    std::string line;
    int current_line = 1;

    while (std::getline(in, line)) {
        if (current_line != target_line) {
            lines.push_back(line);
        }
        current_line++;
    }
    in.close();

    std::ofstream out(filepath);
    if (!out.is_open()) return false;
    for (const auto& l : lines) {
        out << l << "\n";
    }
    return true;
}

// 修改文件指定行的权重
bool DictDatabase::update_weight_in_file(const std::string& filepath, int target_line, int new_weight) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::vector<std::string> lines;
    std::string line;
    int current_line = 1;

    while (std::getline(in, line)) {
        if (current_line == target_line) {
            // 找到目标行，进行分割替换
            std::stringstream ss(line);
            std::string text, code, old_weight;
            std::getline(ss, text, '\t');
            std::getline(ss, code, '\t');

            // 重新拼接新行
            line = text + "\t" + code + "\t" + std::to_string(new_weight);
        }
        lines.push_back(line);
        current_line++;
    }
    in.close();

    std::ofstream out(filepath);
    if (!out.is_open()) return false;
    for (const auto& l : lines) {
        out << l << "\n";
    }
    return true;
}
