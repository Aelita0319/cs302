# Project 2 Final Report

**Group 19**

11612328 程蕴玉  

11811132 郝欣晨

## Result Analysis

#### Expected Goals 

|                  | Single Call                                                  | Continuous Call                                              |
| ---------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| All processes    | Statistic the memory usage of the entire system and sort it. | Monitor the memory usage of the entire system and sort it.   |
| Specific process | 1. Statistic the memory usage of itself and the threads belonging to it.                          2. Detect the memory allocation and release. | 1. Monitor the memory usage of itself and the threads belonging to it. 2. Detect the memory allocation and release. |

#### Achieced Goals

- Our program is a c file used on the command line. Users can run the program directly, or add some other constraints on the command line. For instance, modifying the refresh time of real-time statistics memory usage, sorting all processes according to specified requirements.
- The results can be returned in a single time or continuously run to return the results in real time.

- For a single process with given pid, the amount of time of CPU will be showed on the first line.Then, the pid, its user and command will be displayed. After that are the state of it and the virtual memory size and resident set size of it. The last line is the number of threads it has.
- For a single process with given pid, our program can also display all the threads it owns separately. For the linux system, the thread is implemented by the process, so the thread id is also its process id. The program will display the tid, user and command and virtual memory usage of all processes belonging to this process group.
- For all the processes, our program will first display the CPU time and memory usage information. Then it will show the table of all the processes in a specific order. The list can be sorted by pid, which is the default order, virtual memory size and resident set size. The table will show the pid, user, state, threads number, virtual memory size, resident set size and command.

#### Difficulties

1. 可以加一条命令行方面的困难
2. To display the memory allocation, the prgram reads `/proc/meminfo` file. However, the `MemAvailable ` variable is not display before the Linux 3.14. In order to be consistent in differrent linux version, we finally choose only total memory, free memory and buffers three statistic of memory usage.
3. Our program find the vertual memory information in  `/proc/[pid]/stat` and `/proc/[pid]/status`. Nonetheless, the resident set size value is not accurate. It is number of pages the process has in real memory.  This is just the pages which count toward text, data, or stack space.  This does not include pages which have not been demand-loaded in, or which are swapped out.

## Implementation

Our function implementation refers to the `free.c` and `top.c` files. The main technique our program statistic the memory situation is to read the **proc** file system, including `/proc/meminfo`, `/proc/stat`, `/proc/[pid]/stat`, `/proc/[pid]/cmdline` and `/proc/[pid]/status`. Pack the required information of cpu, memory and process into three structures and read the **proc** file separately.

##### Memory Usage

This part reads the `/proc/meminfo` file and reads the first three lines, which are `MemTotal`, `MemFree` and `Buffers`.

##### Process Status

This part reads `/proc/stat`, `/proc/[pid]/stat`, `/proc/[pid]/cmdline` and `/proc/[pid]/status`. All the processes information is in them and the program accesses the status files of all pids by traversing. It reads the user name, state of the process, the command name, VmSize and VmRSS.

## Future Direction

##### More user-friendly information options

We provide not many command line functions for users to choose, and there is no way to switch at any time. We hope to allow users to switch to the desired display content and method at any time while the program is running.

##### More diverse process information display

Both the display of memory and cpu are very limited. There are still many datas in **proc** that can give users a more comprehensive statistic information. We hope to increase the types of statistical information provided to users by the program, and for information that cannot be accurately derived from the proc file, we can obtain it from other channels.

## Summary

可加一点你的收获

We have a deeper understanding of the use of `top` and `free` commands and have some understanding of how these files are used to view the memory usage. In addition, we have experience in reading **proc**, the process information pseudo-filesystem, which let us increase channels to acquire information of the operating status of the operating system.

As for the concept, we learn more about the virtual memory and the physical memory used by the process that has not been paged out. 

In the teamwork of this project, we adopted the separation of user input and function implementation. By providing interface, code writing of us can be performed synchronously.

## Division of labor

##### 程蕴玉

- Procedure flow chart. 
- User command line input decomposition

#####　郝欣晨

- Memory usage and process information statistic
- Output all statistics information
- Documentation and report