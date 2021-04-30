# Project 19 Design Document 

11612328 程蕴玉  

11811132 郝欣晨

## Background

In Linux operating system, it is important to know how to check the resource usage of the system including memory, CPU and hard disk. This can help us better understand the operation of each program. If an application or program occupies too many resources, it will affect the execution of the system. We should fix this problem and optimize it. In this case, a favorable tool which can statistic the memory usage in real-time will be needed.

Linux already has many commends that can show the usage of resources such as `free`, `top` and `vmstat`, which can show different types of recources. All of them have similarities to the functions we want to achieve therefore they might be good reference object of our design.

The `free` command provides information about the total amount of the physical and swap memory, as well as the free and used memory. When used without any option, the `free` command will display information about the memory and swap in kibibyte. 1 kibibyte (KiB) is 1024 bytes.

The `top` command is a commonly used performance analysis tool under Linux, which can display the resource occupancy status of each process in the system in real time, similar to the task manager of Windows.

The `Vmstat` is the abbreviation of Virtual Meomory Statistics (virtual memory statistics), which can dynamically monitor the virtual memory, process, and CPU activities of the operating system in real time.

## Description

This project is to develop a tool for memory usage on the Linux platform. It can display the memory usage of the process you want to observe and possible memory leaks in real time, which is similar to the task manager of Windows. User can single invoke the tool with the arguments(if any) in the terminal and then it will quickly inform user the statistic they want to know. User can also keep runnning the tool so that the tool will update memory usage in real time.

To further describe the function, the tool can directly show the resources usage, including the processes ID, the name, the CPU percentage, the memory percentage, and some other information. If the user does not specify the process ID, the tool will show the memory usage of the entire system and sort the statistic in real time. 

If the user specifies the process ID, it will display the detailed resource usage of the process and the threads belonging to the process. In addition, the memory allocation and release of the specific process will be detected. We also expect to realize the additional feature to judge whether there is leak situation in the process according to the memory usage information.



## Implementation

We decide to write a C file that can be executed directly.

The function of the file.


1. read_args()               
   Read args and decide which function to go
3. show_all_process()  
   Print all the memory usage of all the current process in the system
4. sort_all_process()  
   Sort the memory usage of the current process
5. show_single_process(int pid)  
   Print the memory usage of the process with the income pid
6. show_threads(int pid)  
   Print the memory usage of threads of the process with the income pid
7. show_allocation(int pid)  
   Print the memory allocation of the process with the income pid
8. show_release(int pid)  
   Print the memory release of the process with the income pid
9. detect_leak(int pid)  
   Statistic process memory allocation and release, check if there is a leak 



## Expected goals

- For a single call, the program only show real-time analysis results once.

- For the continuous call, the program will monitor the usage until exiting and refresh results regularly.

- (Optional) Analyze whether there is memory leaks in the specific process.

  |                  | Single Call                                                  | Continuous Call                                              |
  | ---------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
  | All processes    | Statistic the memory usage of the entire system and sort it. | Monitor the memory usage of the entire system and sort it.   |
  | Specific process | 1. Statistic the memory usage of itself and the threads belonging to it. 2. Detect the memory allocation and release. | 1. Monitor the memory usage of itself and the threads belonging to it. 2. Detect the memory allocation and release. |

  

## Division of Labor

| 程蕴玉                                             | 郝欣晨                                 |
| -------------------------------------------------- | -------------------------------------- |
| Flow control and function of handling all process. | Function of handling a single process. |

