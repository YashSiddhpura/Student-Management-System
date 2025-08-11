// student_terminal_full.c
// Compile:
//   gcc student_terminal_full.c -o student
// Run:
//   ./student

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "student.txt"
#define MAX_STUDENTS 2000

typedef struct
{
    int roll;
    char name[50];
    char section[10];
    float marks;
    char grade[6]; // e.g. "A+", "B"
} Student;

char current_role[16] = ""; // "admin" or "teacher"

/* ---------------- Utilities: File I/O ---------------- */

void calc_grade_from_marks(Student *s)
{
    if (s->marks >= 90) strcpy(s->grade, "A+");
    else if (s->marks >= 80) strcpy(s->grade, "A");
    else if (s->marks >= 70) strcpy(s->grade, "B+");
    else if (s->marks >= 60) strcpy(s->grade, "B");
    else if (s->marks >= 50) strcpy(s->grade, "C");
    else strcpy(s->grade, "F");
}

int load_students(Student arr[])
{
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) return 0;
    int cnt = 0;
    while (cnt < MAX_STUDENTS && fread(&arr[cnt], sizeof(Student), 1, fp) == 1) cnt++;
    fclose(fp);
    return cnt;
}

void save_all_students(Student arr[], int n)
{
    FILE *fp = fopen(FILE_NAME, "wb");
    if (!fp)
    {
        printf("Error: cannot write to %s\n", FILE_NAME);
        return;
    }
    fwrite(arr, sizeof(Student), n, fp);
    fclose(fp);
}

void append_student(const Student *s)
{
    FILE *fp = fopen(FILE_NAME, "ab");
    if (!fp)
    {
        // If file doesn't exist, try creating by saving single record
        fp = fopen(FILE_NAME, "wb");
        if (!fp)
        {
            printf("Error: cannot open file for writing\n");
            return;
        }
    }
    fwrite(s, sizeof(Student), 1, fp);
    fclose(fp);
}

/* ---------------- Input Helpers ---------------- */

// Read a whole line into buf (size bytes), strip newline. Returns 1 on success.
int read_line(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return 1;
}

// Read a number line and parse int. If blank line, return default_flag = 0 and don't change out_val.
int read_int_with_default(int *out_val, int allow_blank, int default_val)
{
    char buf[128];
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0')
    {
        if (allow_blank) return 0;
        else
        {
            *out_val = default_val;
            return 1;
        }
    }
    int val;
    if (sscanf(buf, "%d", &val) == 1)
    {
        *out_val = val;
        return 1;
    }
    return -1;
}

// Read float with default behavior similarly
int read_float_with_default(float *out_val, int allow_blank, float default_val)
{
    char buf[128];
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0')
    {
        if (allow_blank) return 0;
        else
        {
            *out_val = default_val;
            return 1;
        }
    }
    float val;
    if (sscanf(buf, "%f", &val) == 1)
    {
        *out_val = val;
        return 1;
    }
    return -1;
}

/* ---------------- Display helpers ---------------- */

void print_student_row(const Student *s)
{
    printf("| %-6d | %-20s | %-8s | %-7.2f | %-3s |\n",
           s->roll, s->name, s->section, s->marks, s->grade);
}

void print_table_header()
{
    printf("+--------+----------------------+----------+---------+-----+\n");
    printf("| Roll   | Name                 | Section  | Marks   | G   |\n");
    printf("+--------+----------------------+----------+---------+-----+\n");
}

/* ---------------- Authentication ---------------- */

int login_prompt()
{
    char user[64], pass[64];
    printf("=== Login ===\n");
    printf("Username: ");
    if (!read_line(user, sizeof(user))) return 0;
    printf("Password: ");
    if (!read_line(pass, sizeof(pass))) return 0;

    // valid combinations:
    // admin / admin123
    // teacher / teacher123
    if (strcmp(user, "admin") == 0 && strcmp(pass, "admin123") == 0)
    {
        strcpy(current_role, "admin");
        return 1;
    }
    else if (strcmp(user, "teacher") == 0 && strcmp(pass, "teacher123") == 0)
    {
        strcpy(current_role, "teacher");
        return 1;
    }
    else
    {
        printf("Invalid credentials. Try again.\n");
        return 0;
    }
}

/* ---------------- Core Features ---------------- */

void insert_record_terminal()
{
    Student s;
    char buf[128];

    printf("\n--- Insert Student ---\n");
    while (1)
    {
        printf("Roll (integer): ");
        if (!read_line(buf, sizeof(buf))) return;
        if (sscanf(buf, "%d", &s.roll) == 1) break;
        printf("Invalid roll. Try again.\n");
    }

    printf("Name: ");
    read_line(s.name, sizeof(s.name));
    if (s.name[0] == '\0') strcpy(s.name, "Unknown");

    printf("Section: ");
    read_line(s.section, sizeof(s.section));
    if (s.section[0] == '\0') strcpy(s.section, "-");

    while (1)
    {
        printf("Marks (0-100): ");
        if (!read_line(buf, sizeof(buf))) return;
        if (sscanf(buf, "%f", &s.marks) == 1 && s.marks >= 0 && s.marks <= 100) break;
        printf("Invalid marks. Enter a number between 0 and 100.\n");
    }

    printf("Grade (leave blank to auto-calc): ");
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0')
    {
        calc_grade_from_marks(&s);
    }
    else
    {
        strncpy(s.grade, buf, sizeof(s.grade)-1);
        s.grade[sizeof(s.grade)-1] = '\0';
    }

    append_student(&s);
    printf("Student inserted successfully.\n");
}

void display_all_records_terminal()
{
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    if (n == 0)
    {
        printf("No records found.\n");
        return;
    }
    print_table_header();
    for (int i = 0; i < n; ++i) print_student_row(&arr[i]);
    printf("+--------+----------------------+----------+---------+-----+\n");
    printf("Total records: %d\n", n);
}

void search_record_terminal()
{
    char buf[128];
    printf("\n--- Search by Roll ---\n");
    printf("Enter Roll: ");
    read_line(buf, sizeof(buf));
    int roll;
    if (sscanf(buf, "%d", &roll) != 1)
    {
        printf("Invalid input.\n");
        return;
    }
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    for (int i = 0; i < n; ++i)
    {
        if (arr[i].roll == roll)
        {
            print_table_header();
            print_student_row(&arr[i]);
            printf("+--------+----------------------+----------+---------+-----+\n");
            return;
        }
    }
    printf("Record not found.\n");
}

void update_record_terminal()
{
    if (strcmp(current_role, "admin") != 0)
    {
        printf("Permission denied. Only admin can update records.\n");
        return;
    }
    char buf[128];
    printf("\n--- Update Student (admin) ---\n");
    printf("Enter Roll to update: ");
    read_line(buf, sizeof(buf));
    int roll;
    if (sscanf(buf, "%d", &roll) != 1)
    {
        printf("Invalid roll.\n");
        return;
    }
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    int idx = -1;
    for (int i = 0; i < n; ++i) if (arr[i].roll == roll)
        {
            idx = i;
            break;
        }
    if (idx == -1)
    {
        printf("Record not found.\n");
        return;
    }

    printf("Leave field blank to keep current value.\n");
    printf("Current Name: %s\nNew Name: ", arr[idx].name);
    char line[128];
    read_line(line, sizeof(line));
    if (line[0] != '\0') strncpy(arr[idx].name, line, sizeof(arr[idx].name)-1);

    printf("Current Section: %s\nNew Section: ", arr[idx].section);
    read_line(line, sizeof(line));
    if (line[0] != '\0') strncpy(arr[idx].section, line, sizeof(arr[idx].section)-1);

    printf("Current Marks: %.2f\nNew Marks: ", arr[idx].marks);
    int r = read_float_with_default(&arr[idx].marks, 1, arr[idx].marks);
    if (r == -1)
    {
        printf("Invalid marks input. Update aborted.\n");
        return;
    }

    printf("Current Grade: %s\nNew Grade (leave blank to auto-recalc): ", arr[idx].grade);
    read_line(line, sizeof(line));
    if (line[0] == '\0') calc_grade_from_marks(&arr[idx]);
    else
    {
        strncpy(arr[idx].grade, line, sizeof(arr[idx].grade)-1);
        arr[idx].grade[sizeof(arr[idx].grade)-1] = '\0';
    }

    save_all_students(arr, n);
    printf("Record updated successfully.\n");
}

void delete_record_terminal()
{
    if (strcmp(current_role, "admin") != 0)
    {
        printf("Permission denied. Only admin can delete records.\n");
        return;
    }
    char buf[128];
    printf("\n--- Delete Student (admin) ---\n");
    printf("Enter Roll to delete: ");
    read_line(buf, sizeof(buf));
    int roll;
    if (sscanf(buf, "%d", &roll) != 1)
    {
        printf("Invalid roll.\n");
        return;
    }
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    int idx = -1;
    for (int i = 0; i < n; ++i) if (arr[i].roll == roll)
        {
            idx = i;
            break;
        }
    if (idx == -1)
    {
        printf("Record not found.\n");
        return;
    }
    // shift left
    for (int i = idx; i < n - 1; ++i) arr[i] = arr[i+1];
    save_all_students(arr, n - 1);
    printf("Record deleted successfully.\n");
}

/* Sorting helpers */
int cmp_roll_asc(const void *a, const void *b)
{
    Student *A = (Student*)a;
    Student *B = (Student*)b;
    return (A->roll - B->roll);
}
int cmp_marks_desc(const void *a, const void *b)
{
    Student *A = (Student*)a;
    Student *B = (Student*)b;
    if (A->marks < B->marks) return 1;
    if (A->marks > B->marks) return -1;
    return 0;
}

void sort_records_terminal()
{
    char buf[128];
    printf("\n--- Sort Records ---\n");
    printf("1. By Roll (ascending)\n2. By Marks (descending)\nChoose option: ");
    read_line(buf, sizeof(buf));
    int opt = atoi(buf);
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    if (n == 0)
    {
        printf("No records to sort.\n");
        return;
    }
    if (opt == 1) qsort(arr, n, sizeof(Student), cmp_roll_asc);
    else qsort(arr, n, sizeof(Student), cmp_marks_desc);
    print_table_header();
    for (int i = 0; i < n; ++i) print_student_row(&arr[i]);
    printf("+--------+----------------------+----------+---------+-----+\n");
}

void top_n_terminal()
{
    char buf[128];
    printf("\n--- Top N Students by Marks ---\nEnter N: ");
    read_line(buf, sizeof(buf));
    int N = atoi(buf);
    if (N <= 0)
    {
        printf("Invalid N.\n");
        return;
    }
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    if (n == 0)
    {
        printf("No records.\n");
        return;
    }
    qsort(arr, n, sizeof(Student), cmp_marks_desc);
    printf("Top %d students:\n", N);
    print_table_header();
    for (int i = 0; i < N && i < n; ++i) print_student_row(&arr[i]);
    printf("+--------+----------------------+----------+---------+-----+\n");
}

void statistics_terminal()
{
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    if (n == 0)
    {
        printf("No records.\n");
        return;
    }
    float total = 0;
    float max = arr[0].marks, min = arr[0].marks;
    int grade_counts[6] = {0}; // A+,A,B+,B,C, F/others
    for (int i = 0; i < n; ++i)
    {
        float m = arr[i].marks;
        total += m;
        if (m > max) max = m;
        if (m < min) min = m;
        if (strcmp(arr[i].grade, "A+") == 0) grade_counts[0]++;
        else if (strcmp(arr[i].grade, "A") == 0) grade_counts[1]++;
        else if (strcmp(arr[i].grade, "B+") == 0) grade_counts[2]++;
        else if (strcmp(arr[i].grade, "B") == 0) grade_counts[3]++;
        else if (strcmp(arr[i].grade, "C") == 0) grade_counts[4]++;
        else grade_counts[5]++;
    }
    printf("\n--- Statistics ---\n");
    printf("Total students: %d\n", n);
    printf("Average marks: %.2f\n", total / n);
    printf("Max marks: %.2f\n", max);
    printf("Min marks: %.2f\n", min);
    printf("Grade distribution:\n");
    printf("A+: %d\nA: %d\nB+: %d\nB: %d\nC: %d\nF/others: %d\n",
           grade_counts[0], grade_counts[1], grade_counts[2],
           grade_counts[3], grade_counts[4], grade_counts[5]);
}

/* Count students */
void count_students_terminal()
{
    Student arr[MAX_STUDENTS];
    int n = load_students(arr);
    printf("Total students: %d\n", n);
}

/* ---------------- Main Menu & Flow ---------------- */

void show_main_menu()
{
    while (1)
    {
        printf("\n=== Student Management System (Terminal) ===\n");
        printf("Logged in as: %s\n", current_role);
        printf("1. Insert a record\n");
        printf("2. Display all records\n");
        printf("3. Search a record by Roll No\n");
        if (strcmp(current_role, "admin") == 0)
        {
            printf("4. Update a record (admin)\n");
            printf("5. Delete a record (admin)\n");
            printf("6. Sort records\n");
            printf("7. Top N students\n");
            printf("8. Statistics\n");
            printf("9. Count students\n");
            printf("0. Exit\n");
        }
        else     // teacher
        {
            printf("4. Sort records\n");
            printf("5. Top N students\n");
            printf("6. Statistics\n");
            printf("7. Count students\n");
            printf("0. Exit\n");
        }
        printf("Choose option: ");
        char buf[32];
        read_line(buf, sizeof(buf));
        int opt = atoi(buf);

        if (strcmp(current_role, "admin") == 0)
        {
            switch (opt)
            {
            case 1:
                insert_record_terminal();
                break;
            case 2:
                display_all_records_terminal();
                break;
            case 3:
                search_record_terminal();
                break;
            case 4:
                update_record_terminal();
                break;
            case 5:
                delete_record_terminal();
                break;
            case 6:
                sort_records_terminal();
                break;
            case 7:
                top_n_terminal();
                break;
            case 8:
                statistics_terminal();
                break;
            case 9:
                count_students_terminal();
                break;
            case 0:
                printf("Goodbye.\n");
                return;
            default:
                printf("Invalid choice.\n");
            }
        }
        else
        {
            switch (opt)
            {
            case 1:
                insert_record_terminal();
                break;
            case 2:
                display_all_records_terminal();
                break;
            case 3:
                search_record_terminal();
                break;
            case 4:
                sort_records_terminal();
                break;
            case 5:
                top_n_terminal();
                break;
            case 6:
                statistics_terminal();
                break;
            case 7:
                count_students_terminal();
                break;
            case 0:
                printf("Goodbye.\n");
                return;
            default:
                printf("Invalid choice.\n");
            }
        }
    }
}

int main()
{
    printf("Student Management System (Terminal)\n");
    // simple login prompt: allow 3 attempts
    int attempts = 0;
    while (attempts < 3)
    {
        if (login_prompt()) break;
        attempts++;
    }
    if (attempts == 3)
    {
        printf("Too many failed attempts. Exiting.\n");
        return 0;
    }
    show_main_menu();
    return 0;
}
