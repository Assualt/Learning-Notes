//
// Created by 侯鑫 on 2024/1/7.
//

#include "query_widget.h"

QueryWidget::QueryWidget()
{
    Init();

}

QueryWidget::~QueryWidget() = default;

void QueryWidget::Init()
{
    set_title("火车查票订票系统V1.0");
    set_default_size(1200, 500);

    button_.set_label("this is a ");
    button_.set_margin(10);

    set_child(button_);
}