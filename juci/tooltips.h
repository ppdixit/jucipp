#ifndef JUCI_TOOLTIPS_H_
#define JUCI_TOOLTIPS_H_
#include "gtkmm.h"
#include <string>
#include <list>

class Tooltip {
public:
  Tooltip(std::function<Glib::RefPtr<Gtk::TextBuffer>()> get_buffer, Gtk::TextView& text_view, Glib::RefPtr<Gtk::TextBuffer::Mark> start_mark, Glib::RefPtr<Gtk::TextBuffer::Mark> end_mark);
  ~Tooltip();
  
  void update();
  void adjust();
  
  Gdk::Rectangle activation_rectangle;
  std::unique_ptr<Gtk::Window> window;
private:
  std::function<Glib::RefPtr<Gtk::TextBuffer>()> get_buffer;
  std::unique_ptr<Gtk::TextView> tooltip_widget;
  Glib::RefPtr<Gtk::TextBuffer::Mark> start_mark;
  Glib::RefPtr<Gtk::TextBuffer::Mark> end_mark;
  Gtk::TextView& text_view;
  int tooltip_width, tooltip_height;
};

class Tooltips : public std::list<Tooltip> {
public:
  void init() {drawn_tooltips_rectangle=Gdk::Rectangle();}
  void show(const Gdk::Rectangle& rectangle);
  void show();
  void hide();
  
  static Gdk::Rectangle drawn_tooltips_rectangle;
};

#endif  // JUCI_TOOLTIPS_H_