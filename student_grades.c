/*
 * student_grades.c
 * CLI app for managing student grades: add, remove, search,
 * sort by average, statistics, and save/load from file.
 *
 * Compile: gcc student_grades.c -o student_grades
 * Run:     ./student_grades
 *
 * Author: Barbaros Nicolin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS  50
#define MAX_NAME      60
#define MAX_SUBJECTS  10
#define DATA_FILE     "grades.dat"

typedef struct {
    char  name[MAX_NAME];
    float grades[MAX_SUBJECTS];
    int   num_grades;
    float average;
} Student;

/* ── Helpers ──────────────────────────────────────────────── */

void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void calculate_average(Student *s) {
    if (s->num_grades == 0) { s->average = 0.0f; return; }
    float sum = 0.0f;
    for (int i = 0; i < s->num_grades; i++)
        sum += s->grades[i];
    s->average = sum / s->num_grades;
}

/* ── Display ──────────────────────────────────────────────── */

void print_table(Student students[], int n) {
    if (n == 0) { printf("\n  No students recorded.\n"); return; }

    printf("\n  %-4s %-28s %-28s %s\n", "No.", "Name", "Grades", "Average");
    printf("  %s\n", "------------------------------------------------------------------");

    for (int i = 0; i < n; i++) {
        printf("  %-4d %-28s ", i + 1, students[i].name);
        for (int j = 0; j < students[i].num_grades; j++)
            printf("%.1f ", students[i].grades[j]);
        /* Align average column */
        int pad = MAX_SUBJECTS - students[i].num_grades;
        for (int j = 0; j < pad; j++) printf("     ");
        printf(" %.2f\n", students[i].average);
    }
    printf("\n");
}

/* ── Sort (bubble, descending by average) ─────────────────── */

void sort_by_average(Student students[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (students[j].average < students[j + 1].average) {
                Student tmp      = students[j];
                students[j]      = students[j + 1];
                students[j + 1]  = tmp;
            }
        }
    }
}

/* ── CRUD ─────────────────────────────────────────────────── */

void add_student(Student students[], int *n) {
    if (*n >= MAX_STUDENTS) {
        printf("  Maximum capacity reached (%d students).\n", MAX_STUDENTS);
        return;
    }

    Student s;
    printf("  Name: ");
    fgets(s.name, MAX_NAME, stdin);
    s.name[strcspn(s.name, "\n")] = '\0';

    if (strlen(s.name) == 0) { printf("  Name cannot be empty.\n"); return; }

    printf("  Number of grades to enter (1-%d): ", MAX_SUBJECTS);
    if (scanf("%d", &s.num_grades) != 1 || s.num_grades < 1 || s.num_grades > MAX_SUBJECTS) {
        printf("  Invalid number.\n"); clear_input(); return;
    }

    for (int i = 0; i < s.num_grades; i++) {
        do {
            printf("  Grade %d (1–10): ", i + 1);
            scanf("%f", &s.grades[i]);
        } while (s.grades[i] < 1.0f || s.grades[i] > 10.0f);
    }
    clear_input();

    calculate_average(&s);
    students[(*n)++] = s;
    printf("  Added '%s' — average: %.2f\n", s.name, s.average);
}

void remove_student(Student students[], int *n) {
    if (*n == 0) { printf("  No students to remove.\n"); return; }

    print_table(students, *n);
    int idx;
    printf("  Enter student number to remove: ");
    scanf("%d", &idx);
    clear_input();
    idx--;

    if (idx < 0 || idx >= *n) { printf("  Invalid number.\n"); return; }

    printf("  Removed: %s\n", students[idx].name);
    for (int i = idx; i < *n - 1; i++)
        students[i] = students[i + 1];
    (*n)--;
}

void find_student(Student students[], int n) {
    char query[MAX_NAME];
    printf("  Search (partial name): ");
    fgets(query, MAX_NAME, stdin);
    query[strcspn(query, "\n")] = '\0';

    int found = 0;
    for (int i = 0; i < n; i++) {
        if (strstr(students[i].name, query) != NULL) {
            printf("  Found: %-28s average: %.2f\n",
                   students[i].name, students[i].average);
            found++;
        }
    }
    if (!found) printf("  No match for '%s'.\n", query);
}

/* ── Statistics ───────────────────────────────────────────── */

void print_statistics(Student students[], int n) {
    if (n == 0) { printf("  No data available.\n"); return; }

    float total = 0.0f;
    int   best = 0, worst = 0;

    for (int i = 0; i < n; i++) {
        total += students[i].average;
        if (students[i].average > students[best].average)  best  = i;
        if (students[i].average < students[worst].average) worst = i;
    }

    int passed = 0, failed = 0;
    for (int i = 0; i < n; i++) {
        if (students[i].average >= 5.0f) passed++; else failed++;
    }

    printf("\n  ── Class Statistics ──\n");
    printf("  Students       : %d\n",     n);
    printf("  Class average  : %.2f\n",   total / n);
    printf("  Highest avg    : %-28s %.2f\n", students[best].name,  students[best].average);
    printf("  Lowest avg     : %-28s %.2f\n", students[worst].name, students[worst].average);
    printf("  Passed (>=5.0) : %d / %d\n", passed, n);
    printf("  Failed (<5.0)  : %d / %d\n", failed, n);
    printf("\n");
}

/* ── File I/O ─────────────────────────────────────────────── */

int save_to_file(Student students[], int n) {
    FILE *f = fopen(DATA_FILE, "w");
    if (!f) { printf("  Error: cannot write to '%s'.\n", DATA_FILE); return 0; }

    fprintf(f, "%d\n", n);
    for (int i = 0; i < n; i++) {
        fprintf(f, "%s\n%d\n", students[i].name, students[i].num_grades);
        for (int j = 0; j < students[i].num_grades; j++)
            fprintf(f, "%.2f\n", students[i].grades[j]);
    }
    fclose(f);
    printf("  Saved %d student(s) to '%s'.\n", n, DATA_FILE);
    return 1;
}

int load_from_file(Student students[], int *n) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) return 0;   /* File doesn't exist yet — first run */

    fscanf(f, "%d\n", n);
    for (int i = 0; i < *n; i++) {
        fgets(students[i].name, MAX_NAME, f);
        students[i].name[strcspn(students[i].name, "\n")] = '\0';
        fscanf(f, "%d\n", &students[i].num_grades);
        for (int j = 0; j < students[i].num_grades; j++)
            fscanf(f, "%f\n", &students[i].grades[j]);
        calculate_average(&students[i]);
    }
    fclose(f);
    printf("  Loaded %d student(s) from '%s'.\n", *n, DATA_FILE);
    return 1;
}

/* ── Main ─────────────────────────────────────────────────── */

int main(void) {
    Student students[MAX_STUDENTS];
    int n = 0;

    load_from_file(students, &n);

    int choice;
    do {
        printf("\n=== Student Grade Manager ===  [%d student(s)]\n\n", n);
        printf("  1. Add student\n");
        printf("  2. Remove student\n");
        printf("  3. Show all (sorted by average)\n");
        printf("  4. Search by name\n");
        printf("  5. Class statistics\n");
        printf("  6. Save to file\n");
        printf("  0. Exit\n\nChoice: ");

        if (scanf("%d", &choice) != 1) { clear_input(); continue; }
        clear_input();

        switch (choice) {
            case 1: add_student(students, &n);                 break;
            case 2: remove_student(students, &n);              break;
            case 3: sort_by_average(students, n);
                    print_table(students, n);                  break;
            case 4: find_student(students, n);                 break;
            case 5: print_statistics(students, n);             break;
            case 6: save_to_file(students, n);                 break;
            case 0: save_to_file(students, n);
                    printf("Goodbye!\n");                      break;
            default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
