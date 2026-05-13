# OS Labs (WinAPI, Processes & Threads)

This repository contains laboratory works for the Operating Systems course.
The implementations are written in C++ using the Windows API (Win32).

---

## 📌 Overview

The project currently includes:

* **Lab #1 — Processes**
* **Lab #2 — Threads**
* **Lab #3 — Thread Synchronization**
* **Lab #4 — Processes Synchronization**
* **Lab #5 — Named Pipes**
Each lab demonstrates a specific OS concept:

* process creation and management,
* thread creation and synchronization,
* thread coordination with WinAPI synchronization objects,
* inter-process synchronization,
* inter-process communication using Named Pipes.

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

## 🧵 Lab #3 — Thread Synchronization

### Topic

Thread synchronization using WinAPI synchronization primitives.

### Description

The application creates multiple marker threads working with a shared integer array.

Each thread:

* randomly selects array elements,
* marks free cells with its own thread ID,
* stops when it encounters an already marked element.

### Workflow

1. `main` thread:

   * creates a shared array,
   * starts marker threads,
   * waits until all threads become blocked,
   * displays the current array state,
   * requests a thread number to terminate,
   * continues remaining threads.

2. Marker thread:

   * randomly selects array positions,
   * locks access using `CRITICAL_SECTION`,
   * marks free elements,
   * reports blocking information,
   * waits for further commands.

3. After termination:

   * thread clears all elements marked by itself,
   * exits safely.

### Synchronization

The implementation uses:

* `CreateThread`
* `WaitForSingleObject`
* `CreateEvent`
* `SetEvent`
* `ResetEvent`
* `CRITICAL_SECTION`

### Features

* safe shared memory access,
* thread coordination using events,
* validation of user input,
* graceful thread termination.

---

## 🖥️ Lab #4 — Process Synchronization

### Topic

Inter-process synchronization using WinAPI objects.

### Description

The lab implements message exchange between processes using:

* shared binary file,
* mutex,
* semaphores.

The system consists of:

* `Receiver`
* multiple `Sender` processes.

### Workflow

1. `Receiver`:

   * creates a binary message file,
   * creates synchronization objects,
   * starts sender processes,
   * receives and displays messages.

2. `Sender`:

   * waits for free space in the file,
   * writes messages,
   * synchronizes access using mutex and semaphores.

### Synchronization Objects

The implementation uses:

* `CreateMutex`
* `CreateSemaphore`
* `WaitForSingleObject`
* `ReleaseSemaphore`
* `ReleaseMutex`

### Features

* FIFO-style message processing,
* synchronization between independent processes,
* bounded message queue,
* protection against race conditions,
* validation and exception handling.

---

## 🔌 Lab #5 — Named Pipes

### Topic

Inter-process communication using WinAPI Named Pipes.

### Description

The lab implements a client-server system for synchronized access to employee records.

The system consists of:

* `Server`
* multiple `Client` processes.

### Workflow

1. `Server`:

   * creates a binary employee file,
   * initializes synchronization objects,
   * creates Named Pipes,
   * starts client processes,
   * processes client requests.

2. `Client`:

   * connects to the server pipe,
   * requests read/write access,
   * modifies employee records,
   * releases synchronization locks.

### Synchronization

Each employee record supports:

* multiple simultaneous readers,
* exclusive writer access.

### IPC Technologies

The implementation uses:

* `CreateNamedPipe`
* `ConnectNamedPipe`
* `CreateFile`
* `ReadFile`
* `WriteFile`

### Features

* concurrent client processing,
* per-record synchronization,
* reader-writer access model,
* binary file persistence,
* thread-safe server architecture.

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
* WinAPI
  * CreateProcess
  * CreateThread
  * WaitForSingleObject
  * CreateMutex
  * CreateSemaphore
  * CreateNamedPipe
  * ReadFile / WriteFile
  * CRITICAL_SECTION
  * Events* WinAPI (CreateProcess, CreateThread, WaitForSingleObject, etc.)
* CMake

---

## 📚 Purpose

The repository demonstrates practical usage of:

* process management,
* multithreading,
* synchronization primitives,
* inter-process synchronization,
* shared resource protection,
* client-server IPC using Named Pipes,
* WinAPI-based systems programming.

---
