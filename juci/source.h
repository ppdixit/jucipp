#ifndef JUCI_SOURCE_H_
#define JUCI_SOURCE_H_
#include <iostream>
#include <unordered_map>
#include <vector>
#include "gtkmm.h"
#include "clangmm.h"
#include <thread>
#include <mutex>
#include <string>
#include <atomic>
#include "gtksourceviewmm.h"
#include "terminal.h"
#include "tooltips.h"
#include "selectiondialog.h"

namespace Source {
  class Config {
  public:
    bool legal_extension(std::string e) const ;
    unsigned tab_size;
    bool show_line_numbers, highlight_current_line;
    std::string tab, background, background_selected, background_tooltips, font;
    char tab_char=' ';
    std::vector<std::string> extensions;
    std::unordered_map<std::string, std::string> tags, types;
  };  // class Config

  class Location {
  public:
    Location(int line_number, int column_offset):
      line_number(line_number), column_offset(column_offset) {}
    int line_number;
    int column_offset;
  };

  class Range {
  public:
    Range(const Location &start, const Location &end, int kind):
      start(start), end(end), kind(kind) {}
    Location start;
    Location end;
    int kind;
  };

  class AutoCompleteChunk {
  public:
    explicit AutoCompleteChunk(const clang::CompletionChunk &clang_chunk) :
      chunk(clang_chunk.chunk()), kind(clang_chunk.kind()) { }
    std::string chunk;
    enum clang::CompletionChunkKind kind;
  };

  class AutoCompleteData {
  public:
    explicit AutoCompleteData(const std::vector<AutoCompleteChunk> &chunks) :
      chunks(chunks) { }
    std::vector<AutoCompleteChunk> chunks;
    std::string brief_comments;
  };

  class View : public Gsv::View {
  public:
    View(const std::string& file_path, const std::string& project_path);
    std::string get_line(size_t line_number);
    std::string get_line_before_insert();
    std::string file_path;
    std::string project_path;
    Gtk::TextIter search_start, search_end;
  protected:
    bool on_key_press(GdkEventKey* key);
  };  // class View
  
  class GenericView : public View {
  public:
    GenericView(const std::string& file_path, const std::string& project_path):
    View(file_path, project_path) {
      signal_key_press_event().connect(sigc::mem_fun(*this, &Source::GenericView::on_key_press), false);
    }
  private:
    bool on_key_press(GdkEventKey* key) {
      return Source::View::on_key_press(key);
    }
  };
  
  class ClangView : public View {
  public:
    ClangView(const std::string& file_path, const std::string& project_path, Terminal::Controller& terminal);
    ~ClangView();
  private:
    // inits the syntax highligthing on file open
    void init_syntax_highlighting(const std::map<std::string, std::string>
                                &buffers,
                                int start_offset,
                                int end_offset,
                                clang::Index *index);
    std::vector<Source::AutoCompleteData> get_autocomplete_suggestions(int line_number, int column, std::map<std::string, std::string>& buffer_map);
    SelectionDialog selection_dialog;
    int reparse(const std::map<std::string, std::string> &buffers);
    std::vector<Range> extract_tokens(int, int);
    void update_syntax(const std::vector<Range> &locations);
    void update_diagnostics();
    void update_types();
    Tooltips diagnostic_tooltips;
    Tooltips type_tooltips;
    bool clangview_on_motion_notify_event(GdkEventMotion* event);
    void clangview_on_mark_set(const Gtk::TextBuffer::iterator& iterator, const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);
    sigc::connection on_mark_set_timeout_connection;
    bool clangview_on_focus_out_event(GdkEventFocus* event);
    bool clangview_on_scroll_event(GdkEventScroll* event);
    
    static clang::Index clang_index;
    std::map<std::string, std::string> get_buffer_map() const;
    std::mutex parsing_mutex;

    std::unique_ptr<clang::TranslationUnit> clang_tu;
    std::unique_ptr<clang::Tokens> clang_tokens;
    bool clang_updated=false;
    void highlight_token(clang::Token *token,
                        std::vector<Range> *source_ranges,
                        int token_kind);
    void highlight_cursor(clang::Token *token,
                        std::vector<Range> *source_ranges);
    std::vector<std::string> get_compilation_commands();
    bool on_key_press(GdkEventKey* key);
    bool on_key_release(GdkEventKey* key);
    Terminal::Controller& terminal;
    std::shared_ptr<Terminal::InProgress> parsing_in_progress;
    
    Glib::Dispatcher autocomplete_done;
    std::function<void()> autocomplete_done_function;
    bool autocomplete_running=false;
    bool autocomplete_cancel=false;
    Glib::Dispatcher parse_done;
    Glib::Dispatcher parse_start;
    std::thread parse_thread;
    std::map<std::string, std::string> parse_thread_buffer_map;
    std::mutex parse_thread_buffer_map_mutex;
    std::atomic<bool> parse_thread_go;
    std::atomic<bool> parse_thread_mapped;
    std::atomic<bool> parse_thread_stop;
  };

  class Controller {
  public:
    Controller(const std::string& file_path, std::string project_path, Terminal::Controller& terminal);
    Glib::RefPtr<Gsv::Buffer> buffer();
    
    bool is_saved = true;
    
    std::unique_ptr<Source::View> view;
  };  // class Controller
}  // namespace Source
#endif  // JUCI_SOURCE_H_
