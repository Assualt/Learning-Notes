#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/application.h>
#include <iostream>

class HelloWorld : public Gtk::Window {
public:
  HelloWorld() :m_button("Hello World") {
    // Sets the margin around the button.
    m_button.set_margin(10);

    // When the button receives the "clicked" signal, it will call the
    // on_button_clicked() method defined below.
    m_button.signal_clicked().connect(&HelloWorld::ButtonOnClicked);

    // This packs the button into the Window (a container).
    set_child(m_button);

  }
  ~HelloWorld() override = default;

protected:
  //Signal handlers:
  static void ButtonOnClicked() {
    std::cout << "button clicked" << std::endl;
  }

  //Member widgets:
  Gtk::Button m_button;
};

int main(int argc, char* argv[])
{
  auto app = Gtk::Application::create("org.gtkmm.example");

  //Shows the window and returns when it is closed.
  return app->make_window_and_run<HelloWorld>(argc, argv);
}

