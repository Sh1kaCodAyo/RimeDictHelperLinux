#include <gtkmm.h>
#include <iostream>

class MyWindow : public Gtk::Window {
public:
    MyWindow() {
        set_title("RimeDictHelper forLinux");
        set_default_size(400, 200);
        set_border_width(10);

        // 布局容器
        Gtk::Box* vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
        add(*vbox);

        // 输入框
        Gtk::Label* label = Gtk::manage(new Gtk::Label("请输入词条:"));
        label->set_halign(Gtk::ALIGN_START);
        vbox->pack_start(*label, Gtk::PACK_SHRINK);

        entry.set_placeholder_text("例如：你好");
        vbox->pack_start(entry, Gtk::PACK_SHRINK);

        // 按钮
        button.set_label("生成编码");
        vbox->pack_start(button, Gtk::PACK_SHRINK);

        // 结果显示
        result_label.set_text("等待操作...");
        result_label.set_halign(Gtk::ALIGN_START);
        vbox->pack_start(result_label, Gtk::PACK_SHRINK);

        // 绑定按钮点击信号
        button.signal_clicked().connect(sigc::mem_fun(*this, &MyWindow::on_button_clicked));

        show_all_children();
    }

protected:
    void on_button_clicked() {
        // 获取输入框内容（gtkmm 3 返回 Glib::ustring，内部是 UTF-8）
        Glib::ustring text = entry.get_text();
        if (text.empty()) {
            result_label.set_text("输入不能为空！");
        } else {
            // 这里可以替换为你的五笔编码逻辑
            result_label.set_text("你输入了: " + text + " (编码逻辑待接入)");
        }
    }

private:
    Gtk::Entry entry;
    Gtk::Button button;
    Gtk::Label result_label;
};

int main(int argc, char* argv[]) {
    // 初始化 gtkmm
    auto app = Gtk::Application::create(argc, argv, "org.example.rimedicthelper");

    MyWindow window;

    // 运行主循环并显示窗口
    return app->run(window);
}