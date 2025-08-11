# Student-Management-System
I have developed a GUI based Student Management System using C language.

# Student Management System (GTK GUI)

## Overview

This is a **C language** Student Management System with a **GTK+ 3 GUI interface**.
It allows you to manage student records with features like **Add**, **Display**, **Search**, **Sort**, **Update**, **Delete**, and **Statistics**.

The application is designed to work similarly to the original terminal-based version, but with a **user-friendly GUI** built using GTK+.

## Features

* **Add Student** – Input student details and save them to file.
* **Display All Students** – View the list of all student records.
* **Search Student** – Find a student by Roll Number.
* **Sort Students** – Sort records by Roll Number or Name.
* **Update Student** – Edit an existing student's details.
* **Delete Student** – Remove a student record from the system.
* **Statistics** – View basic student data statistics (e.g., total students).
* **File Storage** – All data is stored in a file (`student.dat`) for persistence.

## Requirements

* GCC (GNU Compiler Collection)
* GTK+ 3 development libraries

For Windows (MinGW):

* Install GTK+ 3 for Windows
* Add `pkg-config` and GTK paths to environment variables


## Project Structure

student_management_system_gui.c   # Main C source code
student.dat                       # Data file (created after running)
README.md                         # This file

## Compilation & Running

Compile using:
```bash
gcc student_management_system_gui.c -o smsgui `pkg-config --cflags --libs gtk+-3.0`
```
Run:
```bash
./smsgui
```
On Windows (MinGW example):

```bash
gcc student_management_system_gui.c -o smsgui.exe `pkg-config --cflags --libs gtk+-3.0`
smsgui.exe
```

## Data Storage

* Data is stored in **`student.dat`** as binary records.
* Each time you add, update, or delete, changes are saved immediately.
* Ensure you have read/write permissions in the working directory.

## GUI Layout

The main window contains:

* A **Title Label**
* **Buttons** for all main operations:

  * Add Student
  * Display Students
  * Search Student
  * Sort Students
  * Update Student
  * Delete Student
  * Statistics
  * Quit
