# Project 19 Design Document 

11612328 程蕴玉  

11811132 郝欣晨

## Background

In Linux operating system, it is important to know how to check the resource usage of the system including memory, CPU and hard disk. This can help us better understand the operation of each program. If an application or program occupies too many resources, it will affect the execution of the system. We should fix this problem and optimize it. In this case, a favorable tool which can statistic the memory usage in real-time will be needed.

Linux already has many commends that can show the usage of resources such as free, top and vmstat, which can show different types of recources. They can be approriate reference object of our design.

//free, top, vmstat 等

## Description

This project is to develop a tool for memory usage on the Linux platform. It can display the memory usage of the process you want to observe and possible memory leaks in real time, which is similar to the task manager of Windows. User can single invoke the tool with the arguments(if any) in the terminal and then it will quickly inform user the statistic they want to know. User can also keep runnning the tool so that the tool will update memory usage in real time.

To further describe the function, the tool can directly show the resources usage, including the processes ID, the name, the CPU percentage, the memory percentage, and some other information. If the user does not specify the process ID, the tool will show the memory usage of the entire system and sort the statistic in real time. 

If the user specifies the process ID, it will display the detailed resource usage of the process and the threads belonging to the process. In addition, the memory allocation and release of the specific process will be detected. We also expect to realize the additional feature to judge whether there is leak situation in the process according to the memory usage information.



## Implementation

We decide to write a C file. 

The function of the file.


1. read_args()
2. show_all_process()
3. sort_all_process()
4. show_single_process(int pid)
5. show_threads(int pid)
6. show_allocation(int pid)
7. show_release(int pid)
8. detect_leak(int pid)



## Expected goals

- Statistic the memory usage of the entire system and sort it.
- Statistic the memory usage of the entire system and sort it in real time.
- Statistic the memory usage of a specific process and the threads belonging to it.
- Detect the memory allocation and release for a specific process.
- (Optional) Analyze whether there is memory leaks in the specific process.

## Division of Labor

| 程蕴玉                                             | 郝欣晨                                 |
| -------------------------------------------------- | -------------------------------------- |
| Flow control and function of handling all process. | Function of handling a single process. |

