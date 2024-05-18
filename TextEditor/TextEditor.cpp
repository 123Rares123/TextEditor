#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <cstring>
#include <cerrno>

int changed = 0;
char filename[256] = "";
Fl_Text_Buffer* textbuf;
int loading = 0;
char title[256] = "Untitled";


extern Fl_Menu_Item menuitems[];
void set_title(Fl_Double_Window* w) {
    
}

void set_title(Fl_Double_Window* w);
void save_file(const char* newfile);
void saveas_cb(void);
void load_file(const char* newfile, int ipos);
void find2_cb(Fl_Widget* w, void* v);
int check_save(void);




class EditorWindow : public Fl_Double_Window {
public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow();

    Fl_Window* replace_dlg;
    Fl_Input* replace_find;
    Fl_Input* replace_with;
    Fl_Button* replace_all;
    Fl_Return_Button* replace_next;
    Fl_Button* replace_cancel;
    Fl_Text_Editor* editor;

    char search[256];
};

void changed_cb(int, int nInserted, int nDeleted, int, const char*, void* v) {
    if ((nInserted || nDeleted) && !loading) changed = 1;
    EditorWindow* w = (EditorWindow*)v;
    set_title(w);
    if (loading) w->editor->show_insert_position();
}

void copy_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_copy(0, e->editor);
}

void cut_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_cut(0, e->editor);
}

void delete_cb(Fl_Widget*, void* v) {
    textbuf->remove_selection();
}

void find_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* val;

    val = fl_input("Search String:", e->search);
    if (val != NULL) {
       
        strcpy_s(e->search, sizeof(e->search), val);
        find2_cb(w, v);
    }
}

void find2_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    if (e->search[0] == '\0') {
        
        find_cb(w, v);
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, e->search, &pos);
    if (found) {
        
        textbuf->select(pos, pos + strlen(e->search));
        e->editor->insert_position(pos + strlen(e->search));
        e->editor->show_insert_position();
    }
    else {
        fl_alert("No occurrences of \'%s\' found!", e->search);
    }
}

void insert_cb(Fl_Widget*, void*) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Insert File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if (chooser.show() == 0) {
        load_file(chooser.filename(), textbuf->length());
    }
}

void new_cb(Fl_Widget*, void*) {
    if (!check_save()) return;

    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    changed = 0;
    textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*) {
    if (!check_save()) return;

    Fl_Native_File_Chooser chooser;
    chooser.title("Open File");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if (chooser.show() == 0) {
        load_file(chooser.filename(), -1);
    }
}

void paste_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_paste(0, e->editor);
}

void quit_cb(Fl_Widget*, void*) {
    if (changed && !check_save())
        return;

    exit(0);
}

void replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->show();
}

void replace2_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();

    if (find[0] == '\0') {
       
        e->replace_dlg->show();
        return;
    }

    e->replace_dlg->hide();

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, find, &pos);

    if (found) {
        
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
        textbuf->select(pos, pos + strlen(replace));
        e->editor->insert_position(pos + strlen(replace));
        e->editor->show_insert_position();
    }
    else {
        fl_alert("No occurrences of \'%s\' found!", find);
    }
}

void replall_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();

    if (find[0] == '\0') {
        
        e->replace_dlg->show();
        return;
    }

    e->replace_dlg->hide();

    e->editor->insert_position(0);
    int times = 0;

   
    for (int found = 1; found;) {
        int pos = e->editor->insert_position();
        found = textbuf->search_forward(pos, find, &pos);

        if (found) {
            
            textbuf->select(pos, pos + strlen(find));
            textbuf->remove_selection();
            textbuf->insert(pos, replace);
            e->editor->insert_position(pos + strlen(replace));
            e->editor->show_insert_position();
            times++;
        }
    }

    if (times) {
        fl_message("Replaced %d occurrences.", times);
    }
    else {
        fl_alert("No occurrences of \'%s\' found!", find);
    }
}

void replcan_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->hide();
}

void save_cb(void) {
    if (filename[0] == '\0') {
        
        saveas_cb();
        return;
    }
    else {
        save_file(filename);
    }
}

void saveas_cb(void) {
    Fl_Native_File_Chooser chooser;
    chooser.title("Save File As");
    chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    if (chooser.show() == 0) {
        save_file(chooser.filename());
    }
}

int check_save(void) {
    if (!changed) return 1;

    int r = fl_choice("The current file has not been saved.\n"
        "Would you like to save it now?",
        "Cancel", "Save", "Discard");

    if (r == 1) {
        save_cb(); 
        return !changed;
    }

    return (r == 2) ? 1 : 0;
}

void load_file(const char* newfile, int ipos) {
    loading = 1;
    int insert = (ipos != -1);
    changed = insert;
    if (!insert) strcpy_s(filename, sizeof(filename), "");
    int r;
    if (!insert) {
        r = textbuf->loadfile(newfile);
    }
    else {
        r = textbuf->insertfile(newfile, ipos);
    }
    if (r) {
        char err_buffer[256];
        strerror_s(err_buffer, sizeof(err_buffer), errno);
        fl_alert("Error reading from file \'%s\':\n%s.", newfile, err_buffer);
    }
    else {
        if (!insert) strcpy_s(filename, sizeof(filename), newfile);
    }
    loading = 0;
    textbuf->call_modify_callbacks();
}

void save_file(const char* newfile) {
    if (textbuf->savefile(newfile)) {
        char err_buffer[256];
        strerror_s(err_buffer, sizeof(err_buffer), errno);
        fl_alert("Error writing to file \'%s\':\n%s.", newfile, err_buffer);
    }
    else {
        strcpy_s(filename, sizeof(filename), newfile);
    }
    changed = 0;
    textbuf->call_modify_callbacks();
}

void set_title(EditorWindow* w) {
    if (filename[0] == '\0') {
        strcpy_s(title, sizeof(title), "Untitled");
    }
    else {
        const char* slash;
        slash = strrchr(filename, '/');
        if (slash) strcpy_s(title, sizeof(title), slash + 1);
        else strcpy_s(title, sizeof(title), filename);
    }
    if (changed) strcat_s(title, sizeof(title), " (modified)");
    w->label(title);
}

void view_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    EditorWindow* new_win = new EditorWindow(e->w(), e->h(), title);
    new_win->show();
}

void close_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    if (!check_save()) return;
    e->hide();
}

void undo_cb(Fl_Widget*, void*) {
    textbuf->undo();
    changed = 1;
    textbuf->call_modify_callbacks();
}

EditorWindow::EditorWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) {
    
    Fl_Menu_Bar* m = new Fl_Menu_Bar(0, 0, w, 30);
    m->copy(menuitems);
    editor = new Fl_Text_Editor(10, 40, w - 20, h - 50);
    textbuf = new Fl_Text_Buffer();
    editor->buffer(textbuf);
    textbuf->add_modify_callback(changed_cb, this);

    replace_dlg = new Fl_Window(300, 200, "Replace Dialog");

    replace_find = new Fl_Input(70, 10, 220, 25, "Find:");
    replace_with = new Fl_Input(70, 40, 220, 25, "Replace:");
    replace_next = new Fl_Return_Button(10, 100, 90, 25, "Replace");
    replace_all = new Fl_Button(105, 100, 90, 25, "Replace All");
    replace_cancel = new Fl_Button(200, 100, 90, 25, "Cancel");

    replace_next->callback((Fl_Callback*)replace2_cb, this);
    replace_all->callback((Fl_Callback*)replall_cb, this);
    replace_cancel->callback((Fl_Callback*)replcan_cb, this);

    replace_dlg->end();

    end(); 
}

EditorWindow::~EditorWindow() {
    
}


Fl_Menu_Item menuitems[] = {
    { "&File", 0, 0, 0, FL_SUBMENU },
        { "&New File", FL_CTRL + 'n', (Fl_Callback*)new_cb },
        { "&Open File...", FL_CTRL + 'o', (Fl_Callback*)open_cb },
        { "&Insert File...", FL_CTRL + 'i', (Fl_Callback*)insert_cb, 0, FL_MENU_DIVIDER },
        { "&Save File", FL_CTRL + 's', (Fl_Callback*)save_cb },
        { "Save File &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback*)saveas_cb, 0, FL_MENU_DIVIDER },
        { "New &View", FL_ALT + 'v', (Fl_Callback*)view_cb, 0 },
        { "&Close View", FL_CTRL + 'w', (Fl_Callback*)close_cb, 0, FL_MENU_DIVIDER },
        { "E&xit", FL_CTRL + 'q', (Fl_Callback*)quit_cb, 0 },
        { 0 },
    { "&Edit", 0, 0, 0, FL_SUBMENU },
        { "&Undo", FL_CTRL + 'z', (Fl_Callback*)undo_cb, 0, FL_MENU_DIVIDER },
        { "Cu&t", FL_CTRL + 'x', (Fl_Callback*)cut_cb },
        { "&Copy", FL_CTRL + 'c', (Fl_Callback*)copy_cb },
        { "&Paste", FL_CTRL + 'v', (Fl_Callback*)paste_cb },
        { "&Delete", 0, (Fl_Callback*)delete_cb },
        { 0 },
    { "&Search", 0, 0, 0, FL_SUBMENU },
        { "&Find...", FL_CTRL + 'f', (Fl_Callback*)find_cb },
        { "F&ind Again", FL_CTRL + 'g', (Fl_Callback*)find2_cb },
        { "&Replace...", FL_CTRL + 'r', (Fl_Callback*)replace_cb },
        { "Re&place Again", FL_CTRL + 't', (Fl_Callback*)replace2_cb },
        { 0 },
    { 0 }
};

int main(int argc, char** argv) {
    EditorWindow win(1080, 580, "Text Editor");
    textbuf = new Fl_Text_Buffer;

    win.show(argc, argv);

    if (argc > 1) load_file(argv[1], -1);

    return Fl::run();
}
