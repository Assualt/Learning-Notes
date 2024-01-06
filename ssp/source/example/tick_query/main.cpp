//
// Created by 侯鑫 on 2024/1/7.
//

#include "query_widget.h"
#include <gtkmm/application.h>

int main(int argc, char **argv)
{
    auto app = Gtk::Application::create("org.gtkmm.example");

    //Shows the window and returns when it is closed.
    return app->make_window_and_run<QueryWidget>(argc, argv);
}