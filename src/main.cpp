#include <gtkmm.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <thread>

#include "ConfigManager.h"
#include "DictEntry.h"

class MyWindow : public Gtk::Window
{
public:
    MyWindow() : config_(get_config_path())
    {
        // 窗口基本样式
        // set_icon_from_file("RimeDictHelper2.png");
        set_title("RimeDictHelper for Linux");
        set_default_size(400, 200);
        set_border_width(10);
        set_keep_above(true);

        // ====== 0. 主布局 ======
        auto* main_box_v = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

        // ====== 1. 菜单栏 ======
        // 文件菜单
        auto* item_file = Gtk::manage(new Gtk::MenuItem("文件"));
        auto* menu_file = Gtk::manage(new Gtk::Menu());
        item_file->set_submenu(*menu_file);

        auto* item_setworkspace = Gtk::manage(new Gtk::MenuItem("指定工作目录.."));
        auto* item_reloaddict = Gtk::manage(new Gtk::MenuItem("重新加载词库"));
        auto* item_quit = Gtk::manage(new Gtk::MenuItem("退出"));
        menu_file->append(*item_setworkspace);
        menu_file->append(*item_reloaddict);
        menu_file->append(*item_quit);

        // 帮助菜单
        auto* item_help = Gtk::manage(new Gtk::MenuItem("帮助"));
        auto* menu_help = Gtk::manage(new Gtk::Menu());
        item_help->set_submenu(*menu_help);

        auto* item_about = Gtk::manage(new Gtk::MenuItem("关于"));
        auto* item_website = Gtk::manage(new Gtk::MenuItem("主页"));
        menu_help->append(*item_about);
        menu_help->append(*item_website);

        menubar_.append(*item_file);
        menubar_.append(*item_help);
        main_box_v->pack_start(menubar_, Gtk::PACK_SHRINK);

        // ====== 2. 输入区 ======
        auto* edit_box_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));

        auto* label_word = Gtk::manage(new Gtk::Label("词条"));
        auto* label_code = Gtk::manage(new Gtk::Label("编码"));
        auto* label_weight = Gtk::manage(new Gtk::Label("权重"));

        edit_box_h->pack_start(*label_word, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(entry_text_, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(*label_code, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(entry_code_, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(*label_weight, Gtk::PACK_EXPAND_WIDGET);
        edit_box_h->pack_start(entry_weight_, Gtk::PACK_EXPAND_WIDGET);

        main_box_v->pack_start(*edit_box_h, Gtk::PACK_SHRINK);

        // 输入框信号绑定
        entry_code_.signal_changed().connect(sigc::mem_fun(*this, &MyWindow::on_entry_code_changed));
        entry_weight_.signal_changed().connect(sigc::mem_fun(*this, &MyWindow::on_entry_weight_changed));

        // ====== 3. 按钮区 ======
        auto* button_box_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
        btn_add_.set_label("添加");
        btn_deploy_.set_label("部署");
        btn_add_deploy_.set_label("添加并部署");

        button_box_h->set_homogeneous(true);
        button_box_h->pack_start(btn_add_, Gtk::PACK_EXPAND_WIDGET);
        button_box_h->pack_start(btn_deploy_, Gtk::PACK_EXPAND_WIDGET);
        button_box_h->pack_start(btn_add_deploy_, Gtk::PACK_EXPAND_WIDGET);
        main_box_v->pack_start(*button_box_h, Gtk::PACK_SHRINK);


        // ====== 4. 表格区 ======
        list_store_ = Gtk::ListStore::create(columns_);
        tree_view_.set_model(list_store_);

        // 1. 手动创建列对象，并绑定模型数据
        auto* col1 = Gtk::manage(new Gtk::TreeViewColumn("词条", columns_.col_text));
        auto* col2 = Gtk::manage(new Gtk::TreeViewColumn("编码", columns_.col_code));
        auto* col3 = Gtk::manage(new Gtk::TreeViewColumn("权重", columns_.col_weight));
        auto* col4 = Gtk::manage(new Gtk::TreeViewColumn("来源", columns_.col_source));
        auto* col5 = Gtk::manage(new Gtk::TreeViewColumn("行号", columns_.col_linenumber));

        // 2. 设置自动拉伸，让三列均匀铺满宽度
        // 2. 设置各列的比例 (词条:编码:权重:来源:行号 = 1:1.5:1:1:1)
        // 基础宽度设定为 100，编码列设为 150
        col1->set_min_width(100);
        col2->set_min_width(150); // 这列较宽
        col3->set_min_width(100);
        col4->set_min_width(100);
        col5->set_min_width(100);

        // 依然需要开启 expand 让它们能够分配窗口拉大时多出来的剩余空间
        col1->set_expand(true);
        col2->set_expand(true);
        col3->set_expand(true);
        col4->set_expand(true);
        col5->set_expand(true);

        // 3. 把列追加到表格视图
        tree_view_.append_column(*col1);
        tree_view_.append_column(*col2);
        tree_view_.append_column(*col3);
        tree_view_.append_column(*col4);
        tree_view_.append_column(*col5);

        auto* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
        scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scrolled->set_min_content_height(120);
        scrolled->add(tree_view_);

        // 关键：让表格撑满中间区域
        main_box_v->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);

        // ====== 5. 状态栏 ======
        main_box_v->pack_end(statusbar_, Gtk::PACK_SHRINK);
        add_events(Gdk::KEY_PRESS_MASK);
        // 绑定事件
        bind_events(item_setworkspace, item_reloaddict, item_quit, item_about, item_website);

        add(*main_box_v);
        show_all_children();

        init();
    }

    void bind_events(Gtk::MenuItem* item_setworkspace, Gtk::MenuItem* item_reloaddict,
                     Gtk::MenuItem* item_quit, Gtk::MenuItem* item_about, Gtk::MenuItem* item_website)
    {
        item_setworkspace->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::choose_workspace));
        item_reloaddict->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::load_dict));
        item_quit->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_quit));
        item_about->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_about));
        item_website->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::open_website));


        // 在输入区，绑定词条输入框的变化信号
        entry_text_.signal_changed().connect(sigc::mem_fun(*this, &MyWindow::on_entry_text_changed));
        // 在输入区，绑定回车键到添加按钮
        entry_text_.signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_btn_add_signal));
        entry_code_.signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_btn_add_signal));
        entry_weight_.signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_btn_add_signal));

        signal_key_press_event().connect(sigc::mem_fun(*this, &MyWindow::on_window_key_press));

        btn_add_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_btn_add_signal));
        btn_deploy_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_btn_deploy_clicked));
        btn_add_deploy_.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_btn_add_deploy_clicked));
    }

    void init()
    {
        // 读取保存的路径，如果没配置过，默认为空
        std::string saved_workspace = config_.get_value("workspace_path", "");
        if (!saved_workspace.empty())
        {
            // load_dict_from(saved_workspace);
            statusbar_.push("已加载工作目录: " + saved_workspace, 0);
            // 加载基本词库
            load_dict_file(saved_workspace + "/wubi86_jidian.dict.yaml", DictSource::BASE, db_);
            // 加载用户词库
            load_dict_file(saved_workspace + "/wubi86_jidian_user.dict.yaml", DictSource::USER, db_);

            statusbar_.push("已读取词库，共" + std::to_string(db_.single_char_codes.size()) + "个单字", 0);
            if (saved_workspace.find("rime4test") != std::string::npos)
            {
                set_title("RimeDictHelper for Linux [测试模式]");
            }
        }
        else
        {
            statusbar_.push("请先指定工作目录", 0);
        }
    }

protected:
    bool on_window_key_press(GdkEventKey* event)
    {
        // 判断是否按下了回车
        if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter)
        {
            // 判断是否同时按下了 Ctrl
            if ((event->state & GDK_CONTROL_MASK) != 0)
            {
                on_btn_add_deploy_clicked_par(true);
                return true; // 拦截 Ctrl+Enter，防止继续传递
            }
        }
        // 其他按键放行
        return false;
    }

    std::string generate_wubi_code(const Glib::ustring& text)
    {
        std::vector<std::string> chars_codes;

        // 1. 使用 Glib::ustring 的原生迭代器，它会自动处理 1~4 字节的 UTF-8 字符（包括 Emoji）
        for (auto it = text.begin(); it != text.end(); ++it)
        {
            // 将当前迭代器指向的单个完整字符转为 std::string
            Glib::ustring single_char_glib(1, *it);
            std::string single_char = single_char_glib;

            // 2. 去单字库中查找
            auto db_it = db_.single_char_codes.find(single_char);
            if (db_it != db_.single_char_codes.end())
            {
                chars_codes.push_back(db_it->second);
            }
            else
            {
                // 如果词库里没有（比如是个 Emoji，或者生僻字）
                // 为了不直接报错，我们可以给个“占位符”，或者直接打日志后跳过
                // 这里我们选择返回空，表示无法为这个词组生成完整的五笔编码
                // （你可以在状态栏里提示用户“包含生僻字/特殊符号，无法自动生成”）
                return "";
            }
        }

        if (chars_codes.empty()) return "";

        // 3. 正常的五笔词组编码拼接逻辑
        size_t count = chars_codes.size();

        if (count == 1)
        {
            return chars_codes[0];
        }
        else if (count == 2)
        {
            return chars_codes[0].substr(0, 2) + chars_codes[1].substr(0, 2);
        }
        else if (count == 3)
        {
            return chars_codes[0].substr(0, 1) + chars_codes[1].substr(0, 1) + chars_codes[2].substr(0, 2);
        }
        else
        {
            return chars_codes[0].substr(0, 1) +
                chars_codes[1].substr(0, 1) +
                chars_codes[2].substr(0, 1) +
                chars_codes.back().substr(0, 1);
        }
    }

    // 词条输入框内容变化时实时触发
    void on_entry_text_changed()
    {
        // 1. 获取当前输入的词条
        Glib::ustring text = entry_text_.get_text();

        // 2. 如果为空（比如用户全删了），那就把编码栏也清空，并直接返回
        if (text.empty())
        {
            entry_code_.set_text("");
            // 如果右侧列表也需要清空，可以在这里写逻辑
            return;
        }

        // 3. 调用你原来的五笔编码生成逻辑
        std::string generated_code = generate_wubi_code(text);

        // 4. 将生成的编码设置到编码输入框里
        // 注意：为了防止死循环（虽然你的代码限制了只能输入4位），可以直接设置
        entry_code_.set_text(generated_code);
        entry_weight_.set_text("20");

        // 5. 实时更新右侧重码列表（伪代码）
        // update_conflict_list(generated_code);
    }

    // 追加数据到文件末尾
    bool append_to_user_dict(const std::string& filepath, const std::string& text, const std::string& code, int weight)
    {
        // 1. 以“追加模式”打开文件 (std::ios::app)

        std::ofstream file(filepath, std::ios::app);

        if (!file.is_open())
        {
            std::cerr << "无法打开文件进行写入: " << filepath << std::endl;
            return false;
        }

        // 2. 拼接字符串：词条 \t 编码 \t 权重
        // Rime 词库的权重通常是一个整数，我们把它转成字符串
        file << text << "\t" << code << "\t" << weight << "\n";

        // 3. 关闭文件（C++ 的 ofstream 析构时会自动关闭，但显式调用是个好习惯）
        file.close();
        return true;
    }

    // bool on_entry_key_press(GdkEventKey* event)
    // {
    //     if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter)
    //     {
    //         // 如果按了 Ctrl+Enter
    //         if ((event->state & GDK_CONTROL_MASK) != 0)
    //         {
    //             on_btn_add_deploy_clicked_par(true);
    //         }
    //         else
    //         {
    //             // 普通回车
    //             // 这里直接调用添加，并忽略返回值（为了防止焦点失去）
    //             on_btn_add_signal();
    //         }
    //
    //         // 关键：如果你想在输入框为空时按回车不发声（不响叮咚声），返回 true。
    //         // 为了确保输入法或系统不吞掉按键，建议在这里强行将焦点还给 text 框
    //         entry_text_.grab_focus();
    //         return true;
    //     }
    //     return false;
    // }

    bool on_btn_add_clicked()
    {
        // 1. 获取输入框的内容
        std::string text = entry_text_.get_text();
        std::string code = entry_code_.get_text();

        // 权重需要稍微处理一下，因为输入框是字符串，我们转为 int
        std::string weight_str = entry_weight_.get_text();
        int weight = 20; // 默认权重 20
        if (!weight_str.empty())
        {
            try
            {
                weight = std::stoi(weight_str);
            }
            catch (...)
            {
                weight = 20; // 解析失败用默认值
            }
        }

        // 2. 校验必填项
        if (text.empty() || code.empty())
        {
            statusbar_.push("词条和编码不能为空！", 0);
            return false;
        }

        // 3. 获取当前配置的用户词库文件路径
        // 注意：这里需要确保你已经从 config_ 读取了 workspace 路径
        std::string workspace = config_.get_value("workspace_path");
        if (workspace.empty())
        {
            statusbar_.push("未设置工作目录，无法保存！", 0);
            return false;
        }

        // 用户词库通常以 _user 结尾，具体视你的词库配置而定
        std::string user_dict_path = workspace + "/wubi86_jidian_user.dict.yaml";

        // 4. 追加写入文件
        if (append_to_user_dict(user_dict_path, text, code, weight))
        {
            statusbar_.push("已添加词条: " + text, 0);

            // 5. 可选：写入成功后，立刻更新内存数据库和右侧列表
            // 这里可以先往右侧列表插一行，或者直接调用重新加载词库的逻辑
            // 最优解是直接构造 DictEntry 插入 db_，不过为了直观，这里先清空输入框
            entry_text_.set_text("");
            entry_code_.set_text("");

            // 把焦点移回词条输入框，方便下次快速输入
            entry_text_.grab_focus();
            return true;
        }
        else
        {
            statusbar_.push("写入文件失败，请检查权限或路径！", 0);
            return false;
        }
    }

    void on_btn_add_signal()
    {
        on_btn_add_clicked();
    }

    // 获取当前可执行文件所在的绝对路径
    std::string get_current_dir()
    {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count != -1)
        {
            std::string full_path(result, count);
            size_t pos = full_path.find_last_of('/');
            if (pos != std::string::npos)
            {
                return full_path.substr(0, pos);
            }
        }
        return ".";
    }

    void on_btn_deploy_clicked()
    {
        on_btn_deploy_clicked_par(false);
    }

    void on_btn_deploy_clicked_par(bool close_after_deploy = false)
    {
        if (is_deploying_) return; // 如果正在部署，直接返回，防止连击
        is_deploying_ = true; // 上锁


        std::string workspace = config_.get_value("workspace_path");
        if (workspace.empty())
        {
            statusbar_.push("未设置工作目录！", 0);
            return;
        }

        std::string exe_dir = get_current_dir();
        std::string script_path = exe_dir + "/deploy.sh";
        std::string cmd = "bash \"" + script_path + "\" \"" + workspace + "\"";

        statusbar_.push("正在后台部署与同步...", 0);

        std::thread([this, cmd, close_after_deploy]()
        {
            int result = std::system(cmd.c_str());

            // 切回主线程
            Glib::signal_idle().connect_once([this, result, close_after_deploy]()
            {
                if (result == 0)
                {
                    statusbar_.push("部署与同步成功！", 0);
                    load_dict();
                }
                else
                {
                    statusbar_.push("部署失败，退出码: " + std::to_string(result), 0);
                }

                // 【核心修复】如果是 Ctrl+Enter 触发的，不要立刻 hide
                // 使用 timeout 延迟 100 毫秒再关闭，给底层进程回收留出时间
                if (close_after_deploy)
                {
                    Glib::signal_timeout().connect_once([this]()
                    {
                        hide();
                    }, 100); // 100 毫秒足以让 system 调用彻底清理完毕
                }
            });
        }).detach();
    }

    void on_btn_add_deploy_clicked()
    {
        on_btn_add_deploy_clicked_par(false);
    }

    void on_btn_add_deploy_clicked_par(bool close_after_deploy = false)
    {
        // 只有添加成功（返回 true），才继续执行部署
        if (on_btn_add_clicked())
        {
            on_btn_deploy_clicked_par(close_after_deploy);
        }
    }

    // 修正编码栏
    void on_entry_code_changed()
    {
        Glib::ustring text = entry_code_.get_text();
        Glib::ustring filtered;

        // 只保留字母，最多4位
        for (char c : text)
        {
            if (std::isalpha(c))
            {
                filtered += c;
                if (filtered.length() >= 4) break;
            }
        }

        if (text != filtered)
        {
            int pos = entry_code_.get_position();
            entry_code_.set_text(filtered);
            entry_code_.set_position(std::min((int)filtered.length(), pos));
        }
    }

    // 修正权重栏
    void on_entry_weight_changed()
    {
        Glib::ustring text = entry_weight_.get_text();
        if (text.empty()) return;

        Glib::ustring filtered;
        for (char c : text)
        {
            if (std::isdigit(c))
            {
                filtered += c;
            }
        }

        if (filtered.empty())
        {
            entry_weight_.set_text("");
            return;
        }

        int weight = 1;
        try
        {
            weight = std::stoi(filtered);
        }
        catch (...)
        {
            weight = 1;
        }

        weight = std::clamp(weight, 1, 99); // 使用 std::clamp 更简洁

        Glib::ustring final_text = std::to_string(weight);
        if (text != final_text)
        {
            int pos = entry_weight_.get_position();
            entry_weight_.set_text(final_text);
            entry_weight_.set_position(std::min((int)final_text.length(), pos));
        }
    }

    void choose_workspace()
    {
        // 1. 创建文件选择对话框
        Gtk::FileChooserDialog dialog(*this, "请选择 Rime 工作目录");
        dialog.set_transient_for(*this); // 设置父窗口

        // 2. 添加按钮（取消和打开）
        dialog.add_button("_取消", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_选择", Gtk::RESPONSE_OK);

        // 3. 关键：设置为只能选择文件夹（不能选文件）
        dialog.set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

        // 4. 显示对话框，并阻塞等待用户操作
        const int result = dialog.run();

        // 5. 处理用户的选择结果
        if (result == Gtk::RESPONSE_OK)
        {
            // 获取用户选中的文件夹路径
            std::string folder_path = dialog.get_filename();


            // TODO: 这里你可以把 folder_path 存到全局变量或配置文件中
            // workspace_path_ = folder_path;
            // load_dict(); // 顺便重新加载词库
            //

            // 存储到配置文件！
            config_.set_value("workspace_path", folder_path);

            // 更新底部状态栏提示
            statusbar_.push("工作目录已保存并设定为: " + folder_path, 0);
        }
        else
        {
            statusbar_.push("已取消选择目录", 0);
        }
    }

    void load_dict()
    {
    }

    void on_menu_quit()
    {
        hide();
    }

    void on_menu_about()
    {
        Gtk::MessageDialog dialog(*this, "RimeDictHelper for Linux",
                                  false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
        dialog.set_secondary_text("用于 Rime 词典造词的辅助工具。\n©By Ftwrjh, 2026");
        dialog.run();
    }

    void open_website()
    {
        try
        {
            Gio::AppInfo::launch_default_for_uri("https://github.com/Sh1kaCodAyo/RimeDictHelper2");
        }
        catch (const Glib::Error& ex)
        {
            statusbar_.push("无法打开网页: " + ex.what(), 0);
        }
    }

private:
    Gtk::MenuBar menubar_;

    Gtk::Entry entry_text_;
    Gtk::Entry entry_code_;
    Gtk::Entry entry_weight_;

    Gtk::Button btn_add_;
    Gtk::Button btn_deploy_;
    Gtk::Button btn_add_deploy_;

    bool is_deploying_ = false;

    // 获取用户目录，并拼装配置路径
    std::string get_config_path()
    {
        const char* home = std::getenv("HOME");
        if (home)
        {
            // 路径大概是：/home/你的用户名/.config/rimedicthelper/config.ini
            return std::string(home) + "/.config/rimedicthelper/config.ini";
        }
        return "config.ini"; // 兜底
    }

    // 实例化配置管理器
    ConfigManager config_{get_config_path()};
    DictDatabase db_;

    // 表格数据模型
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(col_text);
            add(col_code);
            add(col_weight);
            add(col_source);
            add(col_linenumber);
        }

        Gtk::TreeModelColumn<Glib::ustring> col_text;
        Gtk::TreeModelColumn<Glib::ustring> col_code;
        Gtk::TreeModelColumn<int> col_weight;
        Gtk::TreeModelColumn<Glib::ustring> col_source;
        Gtk::TreeModelColumn<int> col_linenumber;
    };

    ModelColumns columns_;
    Glib::RefPtr<Gtk::ListStore> list_store_;
    Gtk::TreeView tree_view_;

    Gtk::Statusbar statusbar_;
};

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.example.rimedicthelper");
    MyWindow window;
    return app->run(window);
}
