# OS Labs (WinAPI, Processes & Threads)

This repository contains laboratory works for the Operating Systems course.
The implementations are written in C++ using the Windows API (Win32).

---

## 📌 Overview

The project currently includes:

* **Lab #1 — Processes**
* **Lab #2 — Threads**

Each lab demonstrates a specific OS concept:

* process creation and management,
* thread creation and synchronization.

---

## 🧪 Lab #1 — Processes

### Topic

Process creation and inter-process interaction using WinAPI.

### Description

The lab consists of three programs:

* **Main** — управляющая программа
* **Creator** — создает бинарный файл сотрудников
* **Reporter** — формирует текстовый отчет

### Workflow

1. `Main`:

   * requests file name and number of records,
   * starts **Creator** using command line arguments,
   * waits for its completion (`WaitForSingleObject`),
   * displays the binary file content.

2. `Creator`:

   * creates a binary file,
   * writes employee records entered from console.

   ```cpp
   struct employee
   {
       int num;
       char name[10];
       double hours;
   };
   ```

3. `Main`:

   * requests report file name and hourly rate,
   * starts **Reporter**.

4. `Reporter`:

   * reads the binary file,
   * calculates salary (`hours * rate`),
   * generates a formatted text report.

5. `Main`:

   * waits for Reporter,
   * prints the report to console.

---

## 🧵 Lab #2 — Threads

### Topic

Thread creation and synchronization in a console application.

### Description

The program creates three threads:

* `main`
* `min_max`
* `average`

### Workflow

1. `main` thread:

   * reads an integer array from console,
   * creates worker threads,
   * waits for their completion,
   * replaces min and max elements with average value,
   * prints the result.

2. `min_max` thread:

   * finds minimum and maximum values,
   * prints them,
   * sleeps 7 ms after each comparison.

3. `average` thread:

   * computes arithmetic mean,
   * prints it,
   * sleeps 12 ms during summation.

### Synchronization

* `WaitForSingleObject(INFINITE)` is used to wait for thread completion.

---

## ⚙️ Build

The project uses **CMake**.

### Requirements

* Windows OS
* CMake ≥ 3.10
* C++ compiler (MSVC / MinGW)

### Build steps

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## ▶️ Run

Executables are built separately for each lab component.

Example (Lab #1):

```bash
main.exe
```

Example (Lab #2):

```bash
Win32ThreadsLab.exe
```

---

## 🛠️ Technologies

* C++
* WinAPI (CreateProcess, CreateThread, WaitForSingleObject, etc.)
* CMake

---

## 📚 Purpose

The repository demonstrates practical usage of:

* process management,
* thread synchronization,
* basic inter-process communication patterns.

---
