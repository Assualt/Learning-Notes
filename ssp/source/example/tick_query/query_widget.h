//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_QUERY_WIDGET_H
#define SSP_TOOLS_QUERY_WIDGET_H

#include <gtkmm.h>

class ExampleApplication : public Gtk::Application
{
protected:
    ExampleApplication();

public:
    static Glib::RefPtr<ExampleApplication> create();

protected:
    //Overrides of default signal handlers:
    void on_startup() override;
    void on_activate() override;

private:
    void create_window();

    void on_menu_file_new_generic();
    void on_menu_file_quit();
    void on_menu_help_about();

    Glib::RefPtr<Gtk::Builder> m_refBuilder;
};


#endif //SSP_TOOLS_QUERY_WIDGET_H
