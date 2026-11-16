# hello.cpp — Beginner's Line-by-Line Explanation

This file is a simple FLTK-based notepad application called **AdiPad**. Below is a line-by-line breakdown of what each part does, written for someone new to C++ and GUI programming.

---

## 1. Header Inclusions (`lines 1–12`)

```cpp
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
```

| Line | Header | What it provides |
|------|--------|------------------|
| 1 | `<FL/Fl.H>` | The main FLTK library header — defines the `Fl` class, global functions, and core types. |
| 2 | `<FL/Fl_Window.H>` | Defines `Fl_Window`, the window/container widget. |
| 3 | `<FL/Fl_Text_Editor.H>` | Defines `Fl_Text_Editor` — the multi-line text editing area. |
| 4 | `<FL/Fl_Text_Buffer.H>` | Defines `Fl_Text_Buffer` — holds the text content that the editor displays. |
| 5 | `<FL/Fl_File_Chooser.H>` | Provides `fl_file_chooser()` — pops up a dialog to select/open files. |
| 6 | `<FL/fl_ask.H>` | Provides `fl_choice()` and `fl_input()` — simple popup dialogs for user questions/inputs. |
| 7 | `<FL/fl_message.H>` | Provides `fl_alert()` and `fl_message()` — pop-up message boxes. |
| 8 | `<FL/Fl_Menu_Bar.H>` | Defines `Fl_Menu_Bar` — the horizontal menu bar at the top. |
| 9 | `<cerrno>` | C++ wrapper for `errno` — holds error codes from system calls (e.g., file I/O errors). |
| 10 | `<cstring>` | C++ string utilities — `strcpy`, `strrchr`, `strlen`, etc. |
| 11 | `<cstdlib>` | C++ standard library — `exit()` is defined here. |

---

## 2. Forward Declarations (`lines 14–23`)

```cpp
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
```

These are **function prototypes** (declare functions before `main`). They are named `cb_*` — short for **callbacks**. In FLTK, a callback is a function that gets automatically called when something happens (e.g., user clicks "New", "Save", etc.). `static` means these functions are only visible within this file — they won't conflict with anything outside.

---

## 3. Global State (`lines 26–29`)

```cpp
static Fl_Text_Buffer *textbuf = nullptr;
static char filename[256] = "";
static int changed = 0;
static Fl_Text_Editor *global_editor = nullptr;
```

| Variable | Type | Purpose |
|----------|------|---------|
| `textbuf` | `Fl_Text_Buffer*` | Holds the text content. `nullptr` means "pointing to nothing" (safe default). |
| `filename` | `char[256]` | Stores the current file's path as a C-style string. Starts empty (`""`). |
| `changed` | `int` | Flag: `0` = file is saved and unchanged, `1` = unsaved changes (modified). |
| `global_editor` | `Fl_Text_Editor*` | Pointer to the text editor widget — needed so callbacks can interact with it. |

---

## 4. `set_title()` Function (`lines 32–45`)

```cpp
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
```

This function updates the window's title bar text.

- If `filename` is empty → title becomes `"Untitled"`.
- Otherwise, `strrchr(filename, '/')` finds the **last** `/` in the path (i.e., the folder separator). It then uses the part after that as the filename (e.g., `/home/user/file.txt` → `file.txt`).
- If there's no `/` in the path, it uses the whole string.
- If `changed` is `1`, it appends ` (modified)` to whatever the title currently is.

---

## 5. `cb_check_save()` Function (`lines 48–75`)

```cpp
static int cb_check_save() {
  if (!changed) return 1;  // Nothing to save — proceed immediately
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
    return !changed;  // Return 1 if no longer changed, 0 if still changed
  }
  return (r == 2) ? 1 : 0; // Discard = proceed, Cancel = abort
}
```

This is the **save-prompts-before-losing-work** logic.

1. If `changed` is `0` (file is clean), return `1` immediately — no prompt needed, proceed.
2. If there are unsaved changes, pop up a choice dialog with three buttons: **Cancel**, **Save**, **Discard**.
3. **User clicks Save** (`r == 1`):
   - If a filename exists → try to save it. On error, show an `fl_alert` with the system error (`strerror(errno)`).
   - If no filename (i.e., the file is "Untitled") → open a "Save File As" chooser, let the user pick a location, then save.
   - After saving, reset `changed = 0` and refresh the window title.
4. **User clicks Discard** (`r == 2`): return `1` to proceed (lose the changes).
5. **User clicks Cancel**: return `0` to abort the operation.

The function returns `1` means "OK to proceed," and `0` means "stop and stay."

---

## 6. `cb_new()` Function (`lines 78–87`)

```cpp
static void cb_new(Fl_Widget*, void*) {
  if (!cb_check_save()) return;  // Abort if user chose Cancel in save prompt
  if (textbuf) {
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
  }
  changed = 0;
  strcpy(filename, "");
  set_title(Fl::first_window());
}
```

- First, check if there are unsaved changes — if the user chose "Cancel", exit early.
- If there's an existing text buffer, select all its content and remove it (effectively clearing the editor).
- Reset `changed = 0` and clear `filename` to `""`.
- Refresh the window title to "Untitled".

---

## 7. `cb_open()` Function (`lines 89–105`)

```cpp
static void cb_open(Fl_Widget*, void*) {
  if (!cb_check_save()) return;  // Same save-prompt logic

  if (textbuf) delete textbuf;   // Delete old buffer if it exists
  textbuf = new Fl_Text_Buffer;  // Create a fresh buffer

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
```

- Prompt to save changes if needed (same as above).
- Delete the old buffer to free memory.
- Create a new `Fl_Text_Buffer` to hold the new file's content.
- Open a file chooser dialog. If the user selects a file:
  - Try to load it into the buffer.
  - On error, show an alert.
  - On success, store the file path in `filename` and update the window title.

---

## 8. `cb_save()` Function (`lines 107–120`)

```cpp
static void cb_save(Fl_Widget*, void*) {
  if (filename[0] == '\0') {
    cb_saveas(nullptr, nullptr);  // No filename → go to "Save As"
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
```

- If `filename` is empty (i.e., the user never saved before), jump to "Save As".
- Otherwise, save the current buffer to the existing `filename`.
- On error, show an alert. On success, reset `changed = 0` and refresh the title.

---

## 9. `cb_saveas()` Function (`lines 122–132`)

```cpp
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
```

- Pops up a "Save File As" chooser.
- If the user picks a location and name:
  - Save the buffer there.
  - On error, show an alert.
  - On success, update `filename` and refresh the title.

---

## 10. Edit Callback Functions (`lines 135–149`)

```cpp
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
```

These map keyboard shortcuts (or menu clicks) to cut/copy/paste/delete actions.

- `cb_cut`, `cb_copy`, `cb_paste`: Use `global_editor` (the `Fl_Text_Editor` widget) and its built-in cut/copy/paste functions. They check that `global_editor` is not null first.
- `cb_delete`: Simply removes the current selection from the text buffer.

---

## 11. `cb_find()` Function (`lines 152–167`)

```cpp
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
```

- Pops up an input box asking for a search string.
- If the user enters something:
  - Start searching from the current cursor position.
  - `search_forward` looks for the string in the buffer forward from that position.
  - If found: highlight (select) the found text and move the cursor to after it.
  - If not found: show an alert saying "No occurrences found!"

---

## 12. `main()` Function (`lines 170–211`)

```cpp
int main(int argc, char **argv) {
  textbuf = new Fl_Text_Buffer;        // Create the text buffer
  global_editor = nullptr;              // No editor yet

  Fl_Window *window = new Fl_Window(800, 600, "AdiPad");  // Create window (800×600, title "AdiPad")

  global_editor = new Fl_Text_Editor(0, 0, 800, 600);     // Create the editor widget (full window size)
  global_editor->buffer(textbuf);                           // Link the buffer to the editor

  // Menu bar setup
  Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0, 0, 800, 25);  // Bar at top, full width, 25px tall

  menubar->add("(&New)",        FL_CTRL + 'n', cb_new);       // Ctrl+N → new file
  menubar->add("(&Open)...",   FL_CTRL + 'o', cb_open);     // Ctrl+O → open file
  menubar->add("(&Save)",      FL_CTRL + 's', cb_save);     // Ctrl+S → save
  menubar->add("Save (&As)...", FL_CTRL + FL_SHIFT + 's', cb_saveas); // Ctrl+Shift+S → save as
  menubar->add("E(&xit)",      FL_CTRL + 'q', [](Fl_Widget*, void*) { exit(0); }); // Ctrl+Q → quit

  menubar->add("(&Edit)Cu(&t)",     FL_CTRL + 'x', cb_cut);       // Ctrl+X → cut
  menubar->add("(&Copy)",           FL_CTRL + 'c', cb_copy);      // Ctrl+C → copy
  menubar->add("(&Paste)",          FL_CTRL + 'v', cb_paste);     // Ctrl+V → paste
  menubar->add("(&Delete)",         0, cb_delete);              // No shortcut → delete

  menubar->add("(&Search)Find...",     FL_CTRL + 'f', cb_find);    // Ctrl+F → find

  window->end();           // End window group (finalize layout)
  window->show(argc, argv); // Show the window and enter FLTK event loop

  // If a file was passed as command-line argument...
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

  return Fl::run();  // Start FLTK's main event loop
}
```

Step by step:

1. **Create the buffer** — `textbuf` now holds the text.
2. **`global_editor` stays `nullptr`** for now — it will be set after the editor widget is created.
3. **Create the main window** — 800 pixels wide, 600 tall, titled "AdiPad".
4. **Create the editor widget** — fills the entire window (0,0 to 800,600). Then link `textbuf` to it so the editor displays whatever is in the buffer.
5. **Build the menu bar** — a bar 800 wide, 25 pixels tall, at the top of the window.
6. **Add menu items** — each line adds a menu entry. The syntax is:
   - `menubar->add("Menu Text", shortcut_key, callback_function)`
   - Shortcuts use FLTK constants: `FL_CTRL + 'n'` means Ctrl+N, etc.
   - The `(&X)` notation adds an underline shortcut letter (Alt+X).
7. **Finalize and show the window** — `window->end()` groups the widgets, `window->show()` displays everything and starts the FLTK event loop (waits for user clicks, keystrokes, etc.).
8. **Command-line argument handling** — if the user runs the program with a filename (e.g., `./hello.cpp`), it loads that file instead of starting empty.
   - Delete any existing buffer, create a new one, try to load the file.
   - On success, set `filename` and update the title.
9. **`Fl::run()`** — the main loop. FLTK takes over from here, listening for clicks, keystrokes, and calling the appropriate callback functions.

---

## Summary

AdiPad is a minimal notepad-style application built with the **FLTK** (Fast Light Toolkit) library. It demonstrates:

- **Widgets**: window, menu bar, text editor.
- **Callbacks**: functions triggered by menu clicks or keyboard shortcuts.
- **Buffer**: holds the text content separately from the widget displaying it.
- **File I/O**: load/save files with error handling.
- **User prompts**: confirm unsaved changes, choose files, show alerts.
- **Command-line**: can open a file directly if passed as an argument.

This is a great beginner example because it ties together C++ basics, memory management (via FLTK's objects), and a full GUI loop in under 250 lines.