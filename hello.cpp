#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/fl_message.H>
#include <FL/Fl_Menu_Bar.H>
#include <cerrno>
#include <cstring>
#include <cstdlib>

// ---------- forward declarations ----------
static void cb_new(Fl_Widget*, void*);
static void cb_open(Fl_Widget*, void*);
static void cb_save(Fl_Widget*, void*);
static void cb_saveas(Fl_Widget*, void*);
static int  cb_check_save();
static void cb_cut(Fl_Widget*, void*);
static void cb_copy(Fl_Widget*, void*);
static void cb_paste(Fl_Widget*, void*);
static void cb_delete(Fl_Widget*, void*);
static void cb_find(Fl_Widget*, void*);

// ---------- global state ----------
static Fl_Text_Buffer *textbuf = nullptr;
static char filename[256] = "";
static int changed = 0;
static Fl_Text_Editor *global_editor = nullptr;

// ---------- set_title ----------
static void set_title(Fl_Window *w) {
  if (filename[0] == '\0') {
    w->label("Untitled");
  } else {
    const char *name = strrchr(filename, '/');
    name = (name) ? name + 1 : filename;
    w->label(name);
  }
  if (changed) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s (modified)", w->label());
    w->label(tmp);
  }
}

// ---------- check_save ----------
static int cb_check_save() {
  if (!changed) return 1;
  int r = fl_choice("The current file has not been saved.\n"
                      "Would you like to save it now?",
                      "Cancel", "Save", "Discard");
  if (r == 1) { // Save
    if (filename[0] != '\0') {
      if (textbuf->savefile(filename))
        fl_alert("Error writing to file '%s':\n%s", filename, strerror(errno));
      else {
        changed = 0;
        set_title(Fl::first_window());
      }
    } else {
      char *newfile = fl_file_chooser("Save File As?", "*", filename);
      if (newfile != nullptr) {
        if (textbuf->savefile(newfile))
          fl_alert("Error writing to file '%s':\n%s", newfile, strerror(errno));
        else {
          strcpy(filename, newfile);
          set_title(Fl::first_window());
        }
      }
    }
    return !changed;
  }
  return (r == 2) ? 1 : 0; // Discard = proceed, Cancel = abort
}

// ---------- file callbacks ----------
static void cb_new(Fl_Widget*, void*) {
  if (!cb_check_save()) return;
  if (textbuf) {
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
  }
  changed = 0;
  strcpy(filename, "");
  set_title(Fl::first_window());
}

static void cb_open(Fl_Widget*, void*) {
  if (!cb_check_save()) return;

  if (textbuf) delete textbuf;
  textbuf = new Fl_Text_Buffer;

  char *newfile = fl_file_chooser("Open File?", "*", filename);
  if (newfile != nullptr) {
    int r = textbuf->loadfile(newfile);
    if (r)
      fl_alert("Error reading from file '%s':\n%s", newfile, strerror(errno));
    else {
      strcpy(filename, newfile);
      set_title(Fl::first_window());
    }
  }
}

static void cb_save(Fl_Widget*, void*) {
  if (filename[0] == '\0') {
    cb_saveas(nullptr, nullptr);
    return;
  }
  if (textbuf) {
    if (textbuf->savefile(filename))
      fl_alert("Error writing to file '%s':\n%s", filename, strerror(errno));
    else {
      changed = 0;
      set_title(Fl::first_window());
    }
  }
}

static void cb_saveas(Fl_Widget*, void*) {
  char *newfile = fl_file_chooser("Save File As?", "*", filename);
  if (newfile != nullptr) {
    if (textbuf->savefile(newfile))
      fl_alert("Error writing to file '%s':\n%s", newfile, strerror(errno));
    else {
      strcpy(filename, newfile);
      set_title(Fl::first_window());
    }
  }
}

// ---------- edit callbacks ----------
static void cb_cut(Fl_Widget*, void*) {
  if (global_editor) Fl_Text_Editor::kf_cut(0, global_editor);
}

static void cb_copy(Fl_Widget*, void*) {
  if (global_editor) Fl_Text_Editor::kf_copy(0, global_editor);
}

static void cb_paste(Fl_Widget*, void*) {
  if (global_editor) Fl_Text_Editor::kf_paste(0, global_editor);
}

static void cb_delete(Fl_Widget*, void*) {
  if (textbuf) textbuf->remove_selection();
}

// ---------- search callbacks ----------
static void cb_find(Fl_Widget*, void*) {
  if (!global_editor) return;

  const char *val = fl_input("Search String:", "");
  if (val != nullptr && val[0] != '\0') {
    int pos = global_editor->insert_position();
    int found = textbuf->search_forward(pos, val, &pos);
    if (found) {
      textbuf->select(pos, pos + strlen(val));
      global_editor->insert_position(pos + strlen(val));
      global_editor->show_insert_position();
    } else {
      fl_alert("No occurrences of '%s' found!", val);
    }
  }
}

// ---------- main ----------
int main(int argc, char **argv) {
  textbuf = new Fl_Text_Buffer;
  global_editor = nullptr;

  Fl_Window *window = new Fl_Window(800, 600, "AdiPad");

  global_editor = new Fl_Text_Editor(0, 0, 800, 600);
  global_editor->buffer(textbuf);

  // Menu bar
  Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 800, 25);

  menubar->add("(&New)",        FL_CTRL + 'n', cb_new);
  menubar->add("(&Open)...",   FL_CTRL + 'o', cb_open);
  menubar->add("(&Save)",      FL_CTRL + 's', cb_save);
  menubar->add("Save (&As)...", FL_CTRL + FL_SHIFT + 's', cb_saveas);
  menubar->add("E(&xit)",      FL_CTRL + 'q', [](Fl_Widget*, void*) { exit(0); });

  menubar->add("(&Edit)Cu(&t)",     FL_CTRL + 'x', cb_cut);
  menubar->add("(&Copy)",           FL_CTRL + 'c', cb_copy);
  menubar->add("(&Paste)",          FL_CTRL + 'v', cb_paste);
  menubar->add("(&Delete)",         0, cb_delete);

  menubar->add("(&Search)Find...",     FL_CTRL + 'f', cb_find);

  window->end();
  window->show(argc, argv);

  if (argc > 1) {
    if (textbuf) delete textbuf;
    textbuf = new Fl_Text_Buffer;
    int r = textbuf->loadfile(argv[1]);
    if (r)
      fl_alert("Error reading from file '%s':\n%s", argv[1], strerror(errno));
    else {
      strcpy(filename, argv[1]);
      set_title(Fl::first_window());
    }
  }

  return Fl::run();
}