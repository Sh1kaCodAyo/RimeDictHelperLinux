#include <gtkmm.h>
#include <iostream>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/cellrenderertext.h>
#include <algorithm>
#include <cctype>

class MyWindow : public Gtk::Window {
public:
    MyWindow() {
        // 窗口基本样式
        // set_icon_from_file("RimeDictHelper2.png");
        set_title("RimeDictHelper forLinux");
        set_default_size(400, 200);
        set_border_width(10);
        set_keep_above(true);

        // 0. 主布局
        Gtk::Box* mainbox_v = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

        // 1. 菜单栏
        // 文件菜单
        Gtk::MenuItem* item_file = Gtk::manage(new Gtk::MenuItem("文件"));
        Gtk::Menu* menu_file = Gtk::manage(new Gtk::Menu());
        item_file->set_submenu(*menu_file);

        Gtk::MenuItem* item_setworkspace = Gtk::manage(new Gtk::MenuItem("指定工作目录.."));
        menu_file->append(*item_setworkspace);
        Gtk::MenuItem* item_reloaddict = Gtk::manage(new Gtk::MenuItem("重新加载词库"));
        menu_file->append(*item_reloaddict);
        Gtk::MenuItem* item_quit = Gtk::manage(new Gtk::MenuItem("退出"));
        menu_file->append(*item_quit);

        // 帮助菜单
        Gtk::MenuItem* item_help = Gtk::manage(new Gtk::MenuItem("帮助"));
        Gtk::Menu* menu_help = Gtk::manage(new Gtk::Menu());
        item_help->set_submenu(*menu_help);

        Gtk::MenuItem* item_about = Gtk::manage(new Gtk::MenuItem("关于"));
        menu_help->append(*item_about);
        Gtk::MenuItem* item_website = Gtk::manage(new Gtk::MenuItem("主页"));
        menu_help->append(*item_website);

        menubar.append(*item_file);
        menubar.append(*item_help);
        mainbox_v->pack_start(menubar, Gtk::PACK_SHRINK);

        // 2. 输入区
        Gtk::Box* editbox_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));

        Gtk::Label* l1 = Gtk::manage(new Gtk::Label("词条"));
        Gtk::Label* l2 = Gtk::manage(new Gtk::Label("编码"));
        Gtk::Label* l3 = Gtk::manage(new Gtk::Label("权重"));

        editbox_h->pack_start(*l1, Gtk::PACK_EXPAND_WIDGET);
        editbox_h->pack_start(entry_text, Gtk::PACK_EXPAND_WIDGET);
        editbox_h->pack_start(*l2, Gtk::PACK_EXPAND_WIDGET);
        editbox_h->pack_start(entry_code, Gtk::PACK_EXPAND_WIDGET);
        editbox_h->pack_start(*l3, Gtk::PACK_EXPAND_WIDGET);
        editbox_h->pack_start(entry_weight, Gtk::PACK_EXPAND_WIDGET);

        // 编码栏：拦截输入
        entry_code.signal_insert_text().connect( sigc::mem_fun(*this, &MyWindow::on_entry_code_insert_text), false);
        entry_code.signal_changed().connect( sigc::mem_fun(*this, &MyWindow::on_entry_code_changed));

        // 权重栏：拦截输入
        entry_weight.signal_insert_text().connect( sigc::mem_fun(*this, &MyWindow::on_entry_weight_insert_text), false);
        entry_weight.signal_changed().connect( sigc::mem_fun(*this, &MyWindow::on_entry_weight_changed));

        mainbox_v->pack_start(*editbox_h, Gtk::PACK_SHRINK);

        // 3. 按钮区
        Gtk::Box* buttonbox_h = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
        button1.set_label("添加");
        button2.set_label("部署");
        button3.set_label("添加并部署");
        buttonbox_h->set_homogeneous(true);
        buttonbox_h->pack_start(button1, Gtk::PACK_EXPAND_WIDGET);
        buttonbox_h->pack_start(button2, Gtk::PACK_EXPAND_WIDGET);
        buttonbox_h->pack_start(button3, Gtk::PACK_EXPAND_WIDGET);
        mainbox_v->pack_start(*buttonbox_h, Gtk::PACK_SHRINK);

        // 4. 表格区
        list_store = Gtk::ListStore::create(columns);
        tree_view.set_model(list_store);
        // 1. 手动创建列对象，并绑定模型数据
        auto* col1 = Gtk::manage(new Gtk::TreeViewColumn("词条", columns.col_text));
        auto* col2 = Gtk::manage(new Gtk::TreeViewColumn("编码", columns.col_code));
        auto* col3 = Gtk::manage(new Gtk::TreeViewColumn("权重", columns.col_weight));
        auto* col4 = Gtk::manage(new Gtk::TreeViewColumn("来源", columns.col_source));
        auto* col5 = Gtk::manage(new Gtk::TreeViewColumn("行号", columns.col_linenumber));

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
        tree_view.append_column(*col1);
        tree_view.append_column(*col2);
        tree_view.append_column(*col3);
        tree_view.append_column(*col4);
        tree_view.append_column(*col5);

        // 用 ScrolledWindow 包裹，让表格可滚动，并占满剩余空间
        Gtk::ScrolledWindow* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
        scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scrolled->set_min_content_height(120); // 至少 120 像素高
        scrolled->add(tree_view);

        // 关键：用 PACK_EXPAND_WIDGET 让表格撑满中间区域
        mainbox_v->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);


        // 5. 状态栏
        mainbox_v->pack_end(statusbar, Gtk::PACK_SHRINK);

        //
        bind_event(item_setworkspace, item_reloaddict, item_quit, item_about, item_website);

        // 最后把 vbox 加到窗口
        add(*mainbox_v);

        show_all_children();
        statusbar.push("等待操作...", 0);   // context_id 用来分组管理
    }

    void bind_event(Gtk::MenuItem* item_setworkspace, Gtk::MenuItem* item_reloaddict, Gtk::MenuItem* item_quit, Gtk::MenuItem* item_about, Gtk::MenuItem* item_website)
    {
        item_setworkspace->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::choose_workspace));
        item_reloaddict->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::load_dict));
        item_quit->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_quit));
        item_about->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::on_menu_about));
        item_website->signal_activate().connect(sigc::mem_fun(*this, &MyWindow::open_website));
        button1.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_button_clicked));
    }


protected:
    void on_button_clicked() {
        Glib::ustring text = entry_text.get_text();
        if (text.empty()) {
            // result_label.set_text("输入不能为空！");
            statusbar.push("输入不能为空！", 0);   // context_id 用来分组管理
            return;
        }

        // 往模型追加一行
        auto row = *(list_store->append());
        row[columns.col_text]   = text;
        row[columns.col_code]   = entry_code.get_text();
        row[columns.col_weight] = 12;
        row[columns.col_source] = "基本";
        row[columns.col_linenumber] = 159;

        statusbar.push("已添加: ", 0);   // context_id 用来分组管理

        // 可选：滚动到最后一行
        // tree_view.scroll_to_row(list_store->children().end(), 0.0);
        // tree_view.scroll_to_row(Gtk::TreeModel::Path(list_store->children().end()), 0.0);
        tree_view.scroll_to_row(list_store->get_path(row), 0.0);
    }

        // ===== 编码栏限制 =====
    void on_entry_code_insert_text(const Glib::ustring& text, int* position) {
        // 1. 限制只能输入字母
        for (char c : text) {
            if (!std::isalpha(c)) {
                // 如果含有非字母字符，拦截这次输入
                // 我们通过连接到 signal_insert_text 并返回 void，
                // 然后使用 Gtk::Entry 的 stop_emission 来阻止默认行为
                // 但最简单的做法是使用 GObject 的方式拦截
            }
        }
        // 由于 gtkmm 的 signal_insert_text 较老，更好的做法是使用 signal_changed 纠错
    }
    void on_entry_weight_insert_text(const Glib::ustring& text, int* position) {
        // 1. 限制只能输入字母
        for (char c : text) {
            if (!std::isalpha(c)) {
                // 如果含有非字母字符，拦截这次输入
                // 我们通过连接到 signal_insert_text 并返回 void，
                // 然后使用 Gtk::Entry 的 stop_emission 来阻止默认行为
                // 但最简单的做法是使用 GObject 的方式拦截
            }
        }
        // 由于 gtkmm 的 signal_insert_text 较老，更好的做法是使用 signal_changed 纠错
    }

    // 由于直接拦截较复杂，这里采用“输入后立即修正”的策略，
    // 配合 max_length 属性，用户体验同样很好。

    // 修正编码栏
    void on_entry_code_changed() {
        Glib::ustring text = entry_code.get_text();
        Glib::ustring filtered;

        // 只保留字母，最多4位
        for (char c : text) {
            if (std::isalpha(c)) {
                filtered += c;
                if (filtered.length() >= 4) break;
            }
        }

        if (text != filtered) {
            // 恢复光标位置（尽量保持在末尾）
            int pos = entry_code.get_position();
            entry_code.set_text(filtered);
            entry_code.set_position(std::min((int)filtered.length(), pos));
        }
    }

    // ===== 权重栏限制 =====
    void on_entry_weight_changed() {
        Glib::ustring text = entry_weight.get_text();

        // 如果清空了，保持清空状态（或者你可以强制设为 "1"）
        if (text.empty()) return;

        // 过滤掉非数字字符
        Glib::ustring filtered;
        for (char c : text) {
            if (std::isdigit(c)) {
                filtered += c;
            }
        }

        if (filtered.empty()) {
            entry_weight.set_text("");
            return;
        }

        // 转为整数判断范围
        int weight = 0;
        try {
            weight = std::stoi(filtered);
        } catch (...) {
            weight = 1;
        }

        // 限制范围 1~99
        if (weight < 1) {
            weight = 1;
        } else if (weight > 99) {
            weight = 99;
        }

        // 如果过滤或限制后的结果与原文本不同，则更新文本框
        Glib::ustring final_text = std::to_string(weight);
        if (text != final_text) {
            int pos = entry_weight.get_position();
            entry_weight.set_text(final_text);
            entry_weight.set_position(std::min((int)final_text.length(), pos));
        }
    }

    void choose_workspace() {
        // hide();  // 关闭窗口
    }

    void load_dict() {
        // hide();  // 关闭窗口
    }

    void on_menu_quit() {
        hide();  // 关闭窗口
    }

    void on_menu_about() {
        Gtk::MessageDialog dialog(*this, "RimeDictHelper for Linux",
                                  false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
        dialog.set_secondary_text("用于 Rime 词典造词的辅助工具。\n©By Ftwrjh, 2026");
        dialog.run();
    }

    void open_website() {
        // Gtk::show_uri("", 1);`
    }

private:
    Gtk::MenuBar menubar;

    Gtk::Entry entry_text;
    Gtk::Entry entry_code;
    Gtk::Entry entry_weight;

    Gtk::Button button1;
    Gtk::Button button2;
    Gtk::Button button3;

    // 表格数据模型：每行 3 列（词条、编码、权重）
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
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

    ModelColumns columns;
    Glib::RefPtr<Gtk::ListStore> list_store;
    Gtk::TreeView tree_view;

    // Gtk::Label result_label;
    Gtk::Statusbar statusbar;
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.example.rimedicthelper");
    MyWindow window;
    return app->run(window);
}