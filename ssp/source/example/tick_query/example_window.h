//
// Created by 侯鑫 on 2024/1/10.
//

#ifndef SSP_TOOLS_EXAMPLE_WINDOW_H
#define SSP_TOOLS_EXAMPLE_WINDOW_H

#include <gtkmm.h>

class ExampleWindow : public Gtk::ApplicationWindow
{
public:
    ExampleWindow();
    virtual ~ExampleWindow();

protected:
    //Signal handlers:
    void on_menu_others();

    void on_menu_choices(const Glib::ustring& parameter);
    void on_menu_choices_other(int parameter);
    void on_menu_toggle();

    //Child widgets:
    Gtk::Box m_Box;

    Glib::RefPtr<Gtk::Builder> m_refBuilder;

    //Two sets of choices:
    Glib::RefPtr<Gio::SimpleAction> m_refChoice;
    Glib::RefPtr<Gio::SimpleAction> m_refChoiceOther;

    Glib::RefPtr<Gio::SimpleAction> m_refToggle;
};


#endif //SSP_TOOLS_EXAMPLE_WINDOW_H
