#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "student.txt"
#define MAX_STUDENTS 2000

typedef struct {
    int roll;
    char name[50];
    char section[10];
    float marks;
    char grade[6];
} Student;

/* ---------------- Backend utilities ---------------- */

void calc_grade_from_marks(Student *s) {
    if (s->marks >= 90) strcpy(s->grade, "A+");
    else if (s->marks >= 80) strcpy(s->grade, "A");
    else if (s->marks >= 70) strcpy(s->grade, "B+");
    else if (s->marks >= 60) strcpy(s->grade, "B");
    else if (s->marks >= 50) strcpy(s->grade, "C");
    else strcpy(s->grade, "F");
}

int load_students(Student arr[]) {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) return 0;
    int cnt = 0;
    while (cnt < MAX_STUDENTS && fread(&arr[cnt], sizeof(Student), 1, fp) == 1) cnt++;
    fclose(fp);
    return cnt;
}

void save_all_students(Student arr[], int n) {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (!fp) {
        fprintf(stderr, "Error: cannot write to %s\n", FILE_NAME);
        return;
    }
    fwrite(arr, sizeof(Student), n, fp);
    fclose(fp);
}

void append_student(const Student *s) {
    FILE *fp = fopen(FILE_NAME, "ab");
    if (!fp) {
        fp = fopen(FILE_NAME, "wb");
        if (!fp) { fprintf(stderr, "Error: cannot open file for writing\n"); return; }
    }
    fwrite(s, sizeof(Student), 1, fp);
    fclose(fp);
}

/* comparators */
static int cmp_roll_asc(const void *a, const void *b) {
    const Student *A = a, *B = b;
    return (A->roll - B->roll);
}
static int cmp_marks_desc(const void *a, const void *b) {
    const Student *A = a, *B = b;
    if (A->marks < B->marks) return 1;
    if (A->marks > B->marks) return -1;
    return 0;
}

/* ---------------- GTK helpers ---------------- */

enum { COL_ROLL, COL_NAME, COL_SECTION, COL_MARKS, COL_GRADE, N_COLUMNS };

static void show_message(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_error(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_students_list_window(GtkWindow *parent, const char *title, Student arr[], int n) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_default_size(GTK_WINDOW(win), 720, 420);
    gtk_window_set_title(GTK_WINDOW(win), title);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(win), scrolled);

    GtkListStore *store = gtk_list_store_new(N_COLUMNS,
        G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_STRING);

    GtkTreeIter iter;
    for (int i = 0; i < n; ++i) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COL_ROLL, arr[i].roll,
            COL_NAME, arr[i].name,
            COL_SECTION, arr[i].section,
            COL_MARKS, (gdouble)arr[i].marks,
            COL_GRADE, arr[i].grade,
            -1);
    }

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Roll", renderer, "text", COL_ROLL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, COL_ROLL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COL_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Section", renderer, "text", COL_SECTION, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Marks", renderer, "text", COL_MARKS, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Grade", renderer, "text", COL_GRADE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_widget_show_all(win);
}

/* ---------------- UI structs ---------------- */

typedef struct {
    GtkWidget *ent_roll;
    GtkWidget *ent_name;
    GtkWidget *ent_section;
    GtkWidget *ent_marks;
    GtkWidget *ent_grade;
    GtkWindow *parent;
} InsertData;

typedef struct {
    GtkWidget *ent_roll_readonly;
    GtkWidget *ent_name;
    GtkWidget *ent_section;
    GtkWidget *ent_marks;
    GtkWidget *ent_grade;
    GtkWindow *parent;
} UpdateData;

/* ---------------- Insert feature (with unique roll check) ---------------- */

static void on_insert_save_clicked(GtkButton *btn, gpointer user_data) {
    InsertData *d = (InsertData*)user_data;
    const char *sroll = gtk_entry_get_text(GTK_ENTRY(d->ent_roll));
    const char *sname = gtk_entry_get_text(GTK_ENTRY(d->ent_name));
    const char *ssection = gtk_entry_get_text(GTK_ENTRY(d->ent_section));
    const char *smarks = gtk_entry_get_text(GTK_ENTRY(d->ent_marks));
    const char *sgrade = gtk_entry_get_text(GTK_ENTRY(d->ent_grade));

    if (sroll[0] == '\0') { show_error(d->parent, "Input Error", "Roll is required."); return; }
    int roll = atoi(sroll);
    if (roll <= 0) { show_error(d->parent, "Input Error", "Roll must be a positive integer."); return; }
    if (smarks[0] == '\0') { show_error(d->parent, "Input Error", "Marks are required."); return; }
    float marks = atof(smarks);
    if (marks < 0 || marks > 100) { show_error(d->parent, "Input Error", "Marks must be between 0 and 100."); return; }

    /* unique roll check */
    Student tmp[MAX_STUDENTS]; int n = load_students(tmp);
    for (int i = 0; i < n; ++i) {
        if (tmp[i].roll == roll) { show_error(d->parent, "Duplicate", "Roll number already exists."); return; }
    }

    Student s;
    s.roll = roll;
    strncpy(s.name, sname[0] ? sname : "Unknown", sizeof(s.name)-1); s.name[sizeof(s.name)-1] = 0;
    strncpy(s.section, ssection[0] ? ssection : "-", sizeof(s.section)-1); s.section[sizeof(s.section)-1] = 0;
    s.marks = marks;
    if (sgrade[0] == '\0') calc_grade_from_marks(&s);
    else { strncpy(s.grade, sgrade, sizeof(s.grade)-1); s.grade[sizeof(s.grade)-1] = 0; }

    append_student(&s);
    show_message(d->parent, "Success", "Student inserted successfully.");
    gtk_widget_destroy(GTK_WIDGET(d->parent));
    g_free(d);
}

static void on_insert_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(win), parent);
    gtk_window_set_title(GTK_WINDOW(win), "Insert Student");
    gtk_window_set_default_size(GTK_WINDOW(win), 420, 260);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(win), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    GtkWidget *lbl_roll = gtk_label_new("Roll (integer):");
    GtkWidget *ent_roll = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ent_roll), "e.g. 101");

    GtkWidget *lbl_name = gtk_label_new("Name:");
    GtkWidget *ent_name = gtk_entry_new();

    GtkWidget *lbl_section = gtk_label_new("Section:");
    GtkWidget *ent_section = gtk_entry_new();

    GtkWidget *lbl_marks = gtk_label_new("Marks (0-100):");
    GtkWidget *ent_marks = gtk_entry_new();

    GtkWidget *lbl_grade = gtk_label_new("Grade (leave blank to auto-calc):");
    GtkWidget *ent_grade = gtk_entry_new();

    GtkWidget *btn_save = gtk_button_new_with_label("Save");
    GtkWidget *btn_cancel = gtk_button_new_with_label("Cancel");

    gtk_grid_attach(GTK_GRID(grid), lbl_roll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_roll, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_name, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_name, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_section, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_section, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_marks, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_marks, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_grade, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_grade, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_save, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_cancel, 1, 5, 1, 1);

    InsertData *d = g_malloc(sizeof(InsertData));
    d->ent_roll = ent_roll; d->ent_name = ent_name; d->ent_section = ent_section;
    d->ent_marks = ent_marks; d->ent_grade = ent_grade; d->parent = GTK_WINDOW(win);

    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(gtk_widget_destroy), win);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_insert_save_clicked), d);

    gtk_widget_show_all(win);
}

/* ---------------- Display all ---------------- */

static void on_display_all_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    if (n == 0) { show_message(parent, "No records", "No records found."); return; }
    show_students_list_window(parent, "All Students", arr, n);
}

/* ---------------- Search by roll ---------------- */

static void on_search_by_roll_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Search by Roll", parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Search", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6); gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    GtkWidget *lbl = gtk_label_new("Enter Roll:");
    GtkWidget *ent = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *sroll = gtk_entry_get_text(GTK_ENTRY(ent));
        int roll = atoi(sroll);
        if (roll <= 0) show_error(parent, "Input Error", "Invalid roll.");
        else {
            Student arr[MAX_STUDENTS]; int n = load_students(arr); int found = 0;
            for (int i = 0; i < n; ++i) {
                if (arr[i].roll == roll) { show_students_list_window(parent, "Search Result", &arr[i], 1); found = 1; break; }
            }
            if (!found) show_message(parent, "Not found", "Record not found.");
        }
    }
    gtk_widget_destroy(dialog);
}

/* ---------------- Delete ---------------- */

static void on_delete_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Delete Student", parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Delete", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6); gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    GtkWidget *lbl = gtk_label_new("Enter Roll to delete:");
    GtkWidget *ent = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int roll = atoi(gtk_entry_get_text(GTK_ENTRY(ent)));
        if (roll <= 0) show_error(parent, "Input Error", "Invalid roll.");
        else {
            Student arr[MAX_STUDENTS]; int n = load_students(arr); int idx = -1;
            for (int i = 0; i < n; ++i) if (arr[i].roll == roll) { idx = i; break; }
            if (idx == -1) show_message(parent, "Not found", "Record not found.");
            else {
                GtkWidget *confirm = gtk_message_dialog_new(parent,
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                    "Are you sure you want to delete roll %d?", roll);
                gint r2 = gtk_dialog_run(GTK_DIALOG(confirm));
                gtk_widget_destroy(confirm);
                if (r2 == GTK_RESPONSE_YES) {
                    for (int i = idx; i < n - 1; ++i) arr[i] = arr[i+1];
                    save_all_students(arr, n - 1);
                    show_message(parent, "Deleted", "Record deleted successfully.");
                }
            }
        }
    }
    gtk_widget_destroy(dialog);
}

/* ---------------- Update ---------------- */

static void on_update_save_clicked(GtkButton *btn, gpointer user_data) {
    UpdateData *d = (UpdateData*)user_data;
    const char *sroll = gtk_entry_get_text(GTK_ENTRY(d->ent_roll_readonly));
    const char *sname = gtk_entry_get_text(GTK_ENTRY(d->ent_name));
    const char *ssection = gtk_entry_get_text(GTK_ENTRY(d->ent_section));
    const char *smarks = gtk_entry_get_text(GTK_ENTRY(d->ent_marks));
    const char *sgrade = gtk_entry_get_text(GTK_ENTRY(d->ent_grade));

    int roll = atoi(sroll);
    if (roll <= 0) { show_error(d->parent, "Input Error", "Invalid roll."); return; }
    if (smarks[0] == '\0') { show_error(d->parent, "Input Error", "Marks required."); return; }
    float marks = atof(smarks);
    if (marks < 0 || marks > 100) { show_error(d->parent, "Input Error", "Marks must be 0-100."); return; }

    Student arr[MAX_STUDENTS]; int n = load_students(arr); int idx = -1;
    for (int i = 0; i < n; ++i) if (arr[i].roll == roll) { idx = i; break; }
    if (idx == -1) { show_error(d->parent, "Not found", "Record not found when saving."); return; }

    if (sname[0] != '\0') strncpy(arr[idx].name, sname, sizeof(arr[idx].name)-1);
    if (ssection[0] != '\0') strncpy(arr[idx].section, ssection, sizeof(arr[idx].section)-1);
    arr[idx].marks = marks;
    if (sgrade[0] == '\0') calc_grade_from_marks(&arr[idx]);
    else { strncpy(arr[idx].grade, sgrade, sizeof(arr[idx].grade)-1); arr[idx].grade[sizeof(arr[idx].grade)-1] = 0; }

    save_all_students(arr, n);
    show_message(d->parent, "Success", "Record updated successfully.");
    gtk_widget_destroy(GTK_WIDGET(d->parent));
    g_free(d);
}

static void on_update_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Find Student to Update", parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Find", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *lbl = gtk_label_new("Enter Roll to update:");
    GtkWidget *ent_roll = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent_roll, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int roll = atoi(gtk_entry_get_text(GTK_ENTRY(ent_roll)));
        if (roll <= 0) show_error(parent, "Input Error", "Invalid roll.");
        else {
            Student arr[MAX_STUDENTS]; int n = load_students(arr); int idx = -1;
            for (int i = 0; i < n; ++i) if (arr[i].roll == roll) { idx = i; break; }
            if (idx == -1) show_message(parent, "Not found", "Record not found.");
            else {
                GtkWidget *uwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
                gtk_window_set_transient_for(GTK_WINDOW(uwin), parent);
                gtk_window_set_title(GTK_WINDOW(uwin), "Update Student");
                gtk_window_set_default_size(GTK_WINDOW(uwin), 420, 260);
                GtkWidget *ugrid = gtk_grid_new();
                gtk_container_add(GTK_CONTAINER(uwin), ugrid);
                gtk_grid_set_row_spacing(GTK_GRID(ugrid), 6); gtk_grid_set_column_spacing(GTK_GRID(ugrid), 8);
                gtk_container_set_border_width(GTK_CONTAINER(ugrid), 10);

                GtkWidget *lbl_rollr = gtk_label_new("Roll (readonly):");
                GtkWidget *ent_rollr = gtk_entry_new();
                char tmp[32]; snprintf(tmp, sizeof(tmp), "%d", arr[idx].roll);
                gtk_entry_set_text(GTK_ENTRY(ent_rollr), tmp);
                gtk_editable_set_editable(GTK_EDITABLE(ent_rollr), FALSE);

                GtkWidget *lbl_name = gtk_label_new("Name:");
                GtkWidget *ent_name = gtk_entry_new();
                gtk_entry_set_text(GTK_ENTRY(ent_name), arr[idx].name);

                GtkWidget *lbl_section = gtk_label_new("Section:");
                GtkWidget *ent_section = gtk_entry_new();
                gtk_entry_set_text(GTK_ENTRY(ent_section), arr[idx].section);

                GtkWidget *lbl_marks = gtk_label_new("Marks (0-100):");
                GtkWidget *ent_marks = gtk_entry_new();
                char tmpm[32]; snprintf(tmpm, sizeof(tmpm), "%.2f", arr[idx].marks);
                gtk_entry_set_text(GTK_ENTRY(ent_marks), tmpm);

                GtkWidget *lbl_grade = gtk_label_new("Grade (leave blank to auto-calc):");
                GtkWidget *ent_grade = gtk_entry_new();
                gtk_entry_set_text(GTK_ENTRY(ent_grade), arr[idx].grade);

                GtkWidget *btn_save = gtk_button_new_with_label("Save");
                GtkWidget *btn_cancel = gtk_button_new_with_label("Cancel");

                gtk_grid_attach(GTK_GRID(ugrid), lbl_rollr, 0, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), ent_rollr, 1, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), lbl_name, 0, 1, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), ent_name, 1, 1, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), lbl_section, 0, 2, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), ent_section, 1, 2, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), lbl_marks, 0, 3, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), ent_marks, 1, 3, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), lbl_grade, 0, 4, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), ent_grade, 1, 4, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), btn_save, 0, 5, 1, 1);
                gtk_grid_attach(GTK_GRID(ugrid), btn_cancel, 1, 5, 1, 1);

                UpdateData *ud = g_malloc(sizeof(UpdateData));
                ud->ent_roll_readonly = ent_rollr; ud->ent_name = ent_name; ud->ent_section = ent_section;
                ud->ent_marks = ent_marks; ud->ent_grade = ent_grade; ud->parent = GTK_WINDOW(uwin);

                g_signal_connect(btn_cancel, "clicked", G_CALLBACK(gtk_widget_destroy), uwin);
                g_signal_connect(btn_save, "clicked", G_CALLBACK(on_update_save_clicked), ud);

                gtk_widget_show_all(uwin);
            }
        }
    }
    gtk_widget_destroy(dialog);
}

/* ---------- Sort ---------- */

static void on_sort_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Sort Records", parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "By Roll (asc)", 1, "By Marks (desc)", 2, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (resp == 1 || resp == 2) {
        Student arr[MAX_STUDENTS]; int n = load_students(arr);
        if (n == 0) { show_message(parent, "No records", "No records to sort."); return; }
        if (resp == 1) qsort(arr, n, sizeof(Student), cmp_roll_asc);
        else qsort(arr, n, sizeof(Student), cmp_marks_desc);
        show_students_list_window(parent, "Sorted Students", arr, n);
    }
}

/* ---------- Top N ---------- */

static void on_topn_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Top N Students", parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "Show", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *lbl = gtk_label_new("Enter N:");
    GtkWidget *ent = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ent, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int N = atoi(gtk_entry_get_text(GTK_ENTRY(ent)));
        if (N <= 0) show_error(parent, "Input Error", "Invalid N.");
        else {
            Student arr[MAX_STUDENTS]; int n = load_students(arr);
            if (n == 0) show_message(parent, "No records", "No records.");
            else {
                qsort(arr, n, sizeof(Student), cmp_marks_desc);
                show_students_list_window(parent, "Top Students", arr, (N < n) ? N : n);
            }
        }
    }
    gtk_widget_destroy(dialog);
}

/* ---------- Stats & Count ---------- */

static void on_statistics_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    Student arr[MAX_STUDENTS]; int n = load_students(arr);
    if (n == 0) { show_message(parent, "No records", "No records."); return; }
    float total = 0; float max = arr[0].marks, min = arr[0].marks; int grade_counts[6] = {0};
    for (int i = 0; i < n; ++i) {
        float m = arr[i].marks; total += m;
        if (m > max) max = m; if (m < min) min = m;
        if (strcmp(arr[i].grade, "A+")==0) grade_counts[0]++;
        else if (strcmp(arr[i].grade,"A")==0) grade_counts[1]++;
        else if (strcmp(arr[i].grade,"B+")==0) grade_counts[2]++;
        else if (strcmp(arr[i].grade,"B")==0) grade_counts[3]++;
        else if (strcmp(arr[i].grade,"C")==0) grade_counts[4]++;
        else grade_counts[5]++;
    }
    char buf[512];
    snprintf(buf, sizeof(buf),
        "Total students: %d\nAverage marks: %.2f\nMax marks: %.2f\nMin marks: %.2f\n\nGrade distribution:\nA+: %d\nA: %d\nB+: %d\nB: %d\nC: %d\nF/others: %d",
        n, total / n, max, min, grade_counts[0], grade_counts[1], grade_counts[2], grade_counts[3], grade_counts[4], grade_counts[5]);
    show_message(parent, "Statistics", buf);
}

static void on_count_clicked(GtkButton *b, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);
    Student arr[MAX_STUDENTS]; int n = load_students(arr);
    char buf[64]; snprintf(buf, sizeof(buf), "Total students: %d", n);
    show_message(parent, "Count", buf);
}

/* ---------- Main window (no login) ---------- */

static void build_main_window(void) {
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Student Management System - GUI");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 520, 380);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 12);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(main_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8); gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    GtkWidget *lbl_title = gtk_label_new("Student Management System");
    gtk_grid_attach(GTK_GRID(grid), lbl_title, 0, 0, 2, 1);

    int row = 1;
    GtkWidget *btn_insert = gtk_button_new_with_label("Insert a record");
    gtk_grid_attach(GTK_GRID(grid), btn_insert, 0, row, 1, 1);
    g_signal_connect(btn_insert, "clicked", G_CALLBACK(on_insert_clicked), main_window);

    GtkWidget *btn_display = gtk_button_new_with_label("Display all records");
    gtk_grid_attach(GTK_GRID(grid), btn_display, 1, row++, 1, 1);
    g_signal_connect(btn_display, "clicked", G_CALLBACK(on_display_all_clicked), main_window);

    GtkWidget *btn_search = gtk_button_new_with_label("Search by Roll");
    gtk_grid_attach(GTK_GRID(grid), btn_search, 0, row, 1, 1);
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_search_by_roll_clicked), main_window);

    GtkWidget *btn_sort = gtk_button_new_with_label("Sort records");
    gtk_grid_attach(GTK_GRID(grid), btn_sort, 1, row++, 1, 1);
    g_signal_connect(btn_sort, "clicked", G_CALLBACK(on_sort_clicked), main_window);

    GtkWidget *btn_topn = gtk_button_new_with_label("Top N students");
    gtk_grid_attach(GTK_GRID(grid), btn_topn, 0, row, 1, 1);
    g_signal_connect(btn_topn, "clicked", G_CALLBACK(on_topn_clicked), main_window);

    GtkWidget *btn_stats = gtk_button_new_with_label("Statistics");
    gtk_grid_attach(GTK_GRID(grid), btn_stats, 1, row++, 1, 1);
    g_signal_connect(btn_stats, "clicked", G_CALLBACK(on_statistics_clicked), main_window);

    GtkWidget *btn_count = gtk_button_new_with_label("Count students");
    gtk_grid_attach(GTK_GRID(grid), btn_count, 0, row, 1, 1);
    g_signal_connect(btn_count, "clicked", G_CALLBACK(on_count_clicked), main_window);

    GtkWidget *btn_update = gtk_button_new_with_label("Update a record");
    gtk_grid_attach(GTK_GRID(grid), btn_update, 1, row++, 1, 1);
    g_signal_connect(btn_update, "clicked", G_CALLBACK(on_update_clicked), main_window);

    GtkWidget *btn_delete = gtk_button_new_with_label("Delete a record");
    gtk_grid_attach(GTK_GRID(grid), btn_delete, 0, row, 1, 1);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_delete_clicked), main_window);

    GtkWidget *btn_exit = gtk_button_new_with_label("Exit");
    gtk_grid_attach(GTK_GRID(grid), btn_exit, 0, row+1, 2, 1);
    g_signal_connect_swapped(btn_exit, "clicked", G_CALLBACK(gtk_widget_destroy), main_window);
    g_signal_connect_swapped(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(main_window);
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    build_main_window();
    gtk_main();
    return 0;
}
