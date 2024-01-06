//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_QUERY_WIDGET_H
#define SSP_TOOLS_QUERY_WIDGET_H

#include <gtkmm.h>

class QueryWidget : public Gtk::Window {
public:
    QueryWidget();
    ~QueryWidget() override;

private:
    void Init();

private:
    Gtk::Button button_;
    std::unique_ptr<Gtk::MessageDialog> diagMessageBox_ { nullptr };
};


#endif //SSP_TOOLS_QUERY_WIDGET_H
