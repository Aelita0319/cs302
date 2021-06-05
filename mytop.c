#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define PROC_NAME_LEN 64
#define THREAD_NAME_LEN 32
#define INIT_PROCS 50
#define THREAD_MULT 8
#define MAX_LINE 256

#define die(...) { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }

struct cpu_info {
    // The amount of time, measured in units of USER_HZ
    // (1/100ths of a second on most architectures, use
    // sysconf(_SC_CLK_TCK) to obtain the right value),

    // user   (1) Time spent in user mode.
    // nice   (2) Time spent in user mode with low priority (nice).
    // system (3) Time spent in system mode.
    // idle   (4) Time spent in the idle task.  This value should be USER_HZ times the second entry in the /proc/uptime pseudo-file.
    long unsigned user_time, nice_time, sys_time, idle_time;
    long unsigned iowtime, irqtime, sirqtime;
};

struct mem_info{
    long unsigned mem_total, mem_free;
    long unsigned buffers, cached;
};

struct proc_info {
    struct proc_info *next;
    pid_t pid;
    pid_t tid;
    uid_t uid; // User ID
    gid_t gid; // Group ID
    char name[PROC_NAME_LEN];
    char tname[THREAD_NAME_LEN];
    char state; // R Running; S Sleeping in an interruptible wait; D Waiting in uninterruptible disk sleep; Z Zombie
    long unsigned utime; // Amount of time that this process has been scheduled in user mode
    long unsigned stime; // Amount of time that this process has been scheduled in kernel mode
    int num_threads;
    long vmsize; // Virtual memory size in KB.
    long vmrss; // Resident set size in KB, which is the sum of RssAnon, RssFile, and RssShmem
    // long rss_sh; // Size of resident shared memory.
    // long rss_file; // Size of resident file mappings.
    // long rss_anon; // Size of resident anonymous memory.
};

int sort = 0; // 0: default(pid), 1: vss, 2: rss


static void refresh_p(int delay,pid_t pid,void(*show_single_process)(pid_t));
static void refresh_a(int delay,void(*show_all_process)());
static void refresh_t(int delay,pid_t pid,void(*show_thread)(pid_t));

static void print_usage();
static void show_single_process(pid_t pid);
static void show_thread(pid_t pid);
static void show_all_process(); 
static void sort_all_process(struct proc_info *head); 
static int read_stat(char *filename, struct proc_info *proc);
static int read_cmdline(char *filename, struct proc_info *proc);
static int read_status(char *filename, struct proc_info *proc);
static int read_meminfo(char *filename, struct mem_info *mem);


int main(int argc, char *argv[]){
    if(argc == 1){
        print_usage();
    }
    int type;
    char c;
    char op='z';
    int delay=1;
    pid_t pid;
    int if_f=0;
    for(int i=0;i<argc;i++){
        if(!strncmp(argv[i],"-f",2)){
             if_f=1;
             char s2[10];
             strncpy(s2,argv[i]+2,10);
             delay=atoi(s2);
             if(delay<=0)delay=1;
        }
    }   
    while((type = getopt(argc, argv, "has::p:t:f::")) != -1){  
        switch(type)
        {
            case 'a': //option a   -a，no parameter           
                if(if_f){
                    refresh_a(delay,show_all_process);
                    break;
                }  
                show_all_process();
                break;
            case 's': //option s  -s，one parameter
                if(optarg!=NULL){
                    sort=atoi(optarg);
                    if(sort!=1&&sort!=2){
                        printf("parameter can only be 0,1,2\n\n");
                        break;
                    }
                    printf("sort= %d\n",sort);
                }      
                if(if_f){
                    refresh_a(delay,show_all_process);
                    break;
                }                          
                show_all_process();
                break;
            case 'p': //option p optarg as parameter，-p 1024/-p1024                      
                pid=atoi(optarg); 
                if(if_f){
                    refresh_p(delay,pid,show_single_process);
                    break;
                }  
                show_single_process(pid);
                break;
            case 't': //option t optarg as parameter，-t 1024/-t1024
               // printf("ttpid:%s\n\n",optarg);
                pid=atoi(optarg); 
                if(if_f){
                    refresh_t(delay,pid,show_thread);
                    break;
                }  
                show_thread(pid);
                break;
            case 'h'://help
                print_usage();
                break;
        }
    }
    return 0;
}
static void refresh_p(int delay,pid_t pid,void(*show_single_process)(pid_t)){
    while(1){
    sleep(delay);
    show_single_process(pid);
    }
}
static void refresh_a(int delay,void(*show_all_process)()){
    while(1){
    sleep(delay);
    show_all_process();
    }
}
static void refresh_t(int delay,pid_t pid,void(*show_thread)(pid_t)){
    while(1){
    sleep(delay);
    show_thread(pid);
    }
}
static void show_single_process(pid_t pid){
    DIR *proc_dir;
    struct dirent *pid_dir;
    char filename[64];
    FILE *file;
    int proc_num;
    struct proc_info *proc;
    struct cpu_info cpu;
    struct passwd *user;
    char *user_str, user_buf[20];

    proc_dir = opendir("/proc");
    if (!proc_dir) die("Could not open /proc.\n");
 
    file = fopen("/proc/stat", "r");
    if (!file) die("Could not open /proc/stat.\n");

    fscanf(file, "cpu  %lu %lu %lu %lu %lu %lu %lu", &cpu.user_time, &cpu.nice_time, &cpu.sys_time,
            &cpu.idle_time, &cpu.iowtime, &cpu.irqtime, &cpu.sirqtime);
    fclose(file);
    
    printf("CPU(jiffies): user: %lu, system: %lu, nice: %lu, idle: %lu, ", cpu.user_time, cpu.sys_time, cpu.nice_time,cpu.idle_time);
    printf("iowait: %lu, irq: %lu, softirq: %lu.\n", cpu.iowtime, cpu.irqtime, cpu.sirqtime);

    while ((pid_dir = readdir(proc_dir))){
        if (!isdigit(pid_dir->d_name[0]))
            continue;
        if (pid == atoi(pid_dir->d_name)){
            proc = malloc(sizeof(*proc));
            if (!proc) die("Could not allocate struct process_info.\n");
            proc->pid = pid;

            sprintf(filename, "/proc/%d/stat", pid);
            read_stat(filename, proc);

            sprintf(filename, "/proc/%d/cmdline", pid);
            read_cmdline(filename, proc);

            sprintf(filename, "/proc/%d/status", pid);
            read_status(filename, proc);
        }
    }
    user = getpwuid(proc->uid);
    if (user && user->pw_name) {
        user_str = user->pw_name;
    } else {
        snprintf(user_buf, 20, "%d", proc->uid);
        user_str = user_buf;
    }
    printf("PID: %d\n", pid);
    printf("UID: %s, Command: %s\n", user_str, proc->name);
    printf("State: %c\n", proc->state);
    // printf("VSS: %ldKb, RSS: %ldKb\n", proc->vsize / 1024, proc->rss * getpagesize() / 1024);
    printf("VmSize: %ld KB\n", proc->vmsize);
    printf("VmRSS: %ld KB\n", proc->vmrss);
    // printf("VmRSS: %ld KB (RssAnon: %ld KB, RssFile: %ld KB, RssShmem: %ld KB)\n", proc->vmrss, 
    //     proc->rss_anon, proc->rss_file, proc->rss_sh);
    printf("Number of threads: %d\n", proc->num_threads);
    printf("\n\n");
}

static void show_thread(pid_t pid){
    DIR *proc_dir, *task_dir;
    struct dirent *pid_dir, *tid_dir;
    char filename[64];
    FILE *file;
    int proc_num;
    struct proc_info *proc, *thr;
    struct cpu_info cpu;
    struct passwd *user;
    char *user_str, user_buf[20];
    pid_t tid;

    proc_dir = opendir("/proc");
    if (!proc_dir) die("Could not open /proc.\n");
 
    file = fopen("/proc/stat", "r");
    if (!file) die("Could not open /proc/stat.\n");

    fscanf(file, "cpu  %lu %lu %lu %lu %lu %lu %lu", &cpu.user_time, &cpu.nice_time, &cpu.sys_time,
            &cpu.idle_time, &cpu.iowtime, &cpu.irqtime, &cpu.sirqtime);
    fclose(file);
    printf("CPU(jiffies): user: %lu, system: %lu, nice: %lu, idle: %lu, ", cpu.user_time, cpu.sys_time, cpu.nice_time,cpu.idle_time);
    printf("iowait: %lu, irq: %lu, softirq: %lu.\n", cpu.iowtime, cpu.irqtime, cpu.sirqtime);

    while ((pid_dir = readdir(proc_dir))){
        if (!isdigit(pid_dir->d_name[0]))
            continue;  
        if (pid == atoi(pid_dir->d_name)){
            proc = malloc(sizeof(*proc));
            if (!proc) die("Could not allocate struct process_info.\n");
            proc->pid = pid;

            sprintf(filename, "/proc/%d/stat", pid);
            read_stat(filename, proc);

            sprintf(filename, "/proc/%d/cmdline", pid);
            read_cmdline(filename, proc);

            sprintf(filename, "/proc/%d/status", pid);
            read_status(filename, proc);

            user = getpwuid(proc->uid);
            if (user && user->pw_name) {
                user_str = user->pw_name;
            } else {
                snprintf(user_buf, 20, "%d", proc->uid);
                user_str = user_buf;
            }           
            printf("PID: %d\n", pid);
            printf("UID: %s, Command: %s\n", user_str, proc->name);
            printf("State: %c\n", proc->state);
            printf("VmSize: %ld KB\n", proc->vmsize);
            printf("VmRSS: %ld KB\n", proc->vmrss);
            printf("Number of threads: %d\n", proc->num_threads);  
            printf("The following are the threads memory usage:\n\n");          

            sprintf(filename, "/proc/%d/task", pid);
            task_dir = opendir(filename);
            if (!task_dir) continue;  

            while ((tid_dir = readdir(task_dir))) {
                if (!isdigit(tid_dir->d_name[0]))
                    continue;
                tid = atoi(tid_dir->d_name);
                thr = malloc(sizeof(*thr)); 
                sprintf(filename, "/proc/%d/task/%d/stat", pid, tid);
                read_stat(filename, thr);
                sprintf(filename, "/proc/%d/task/%d/cmdline", pid, tid);
                read_cmdline(filename, proc);
                sprintf(filename, "/proc/%d/task/%d/status", pid, tid);
                read_stat(filename, thr);
                
                printf("--------------------------------------\n");
                printf("TID: %d, Command: %s\n", tid, proc->name);
                printf("State: %c\n", thr->state);
                printf("VmSize: %ld KB, VmRSS: %ld KB\n", proc->vmsize, proc->vmrss);
                
            }
        }   
    }
printf("\n\n");
}

static void show_all_process(){
    DIR *proc_dir;
    struct dirent *pid_dir;
    char filename[64];
    FILE *file;
    int proc_num;
    struct proc_info *head;
    struct cpu_info cpu;
    struct mem_info *mem;
    struct passwd *user;
    char *user_str, user_buf[20];
    pid_t pid;

    proc_dir = opendir("/proc");
    if (!proc_dir) die("Could not open /proc.\n");
 
    file = fopen("/proc/stat", "r");
    if (!file) die("Could not open /proc/stat.\n");

    fscanf(file, "cpu  %lu %lu %lu %lu %lu %lu %lu", &cpu.user_time, &cpu.nice_time, &cpu.sys_time,
            &cpu.idle_time, &cpu.iowtime, &cpu.irqtime, &cpu.sirqtime);
    fclose(file);
    printf("CPU(jiffies): user: %lu, system: %lu, nice: %lu, idle: %lu, ", cpu.user_time, cpu.sys_time, cpu.nice_time,cpu.idle_time);
    printf("iowait: %lu, irq: %lu, softirq: %lu.\n", cpu.iowtime, cpu.irqtime, cpu.sirqtime);    

    mem = malloc(sizeof(*mem));
    read_meminfo("/proc/meminfo", mem);
    printf("Memory usage: %lu kB total, %lu kB free.\n", mem->mem_total, mem->mem_free);
    printf("Buffers: %lu kB.\n", mem->buffers);
    // printf("Buffers: %lu kB, Cached: %lu kB\n", mem->buffers, mem->cached);
    

    printf("%5s %-8s %1s %5s %7s %7s %s\n", "PID", "USER", "S", "#THR", "VSS", "RSS", "COMMAND");
    
    head = malloc(sizeof(*head));
    struct proc_info *p = head;
    while ((pid_dir = readdir(proc_dir))){
        if (!isdigit(pid_dir->d_name[0]))
            continue;
        pid = atoi(pid_dir->d_name);
        struct proc_info *proc;
        proc = malloc(sizeof(*proc));
        if (!proc) die("Could not allocate struct process_info.\n");

        proc->pid = pid;
        proc->next = NULL;

        sprintf(filename, "/proc/%d/stat", pid);
        read_stat(filename, proc);

        sprintf(filename, "/proc/%d/cmdline", pid);
        read_cmdline(filename, proc);

        sprintf(filename, "/proc/%d/status", pid);
        read_status(filename, proc);
        
        p->next = proc;
        p = proc;
        // printf("%5d %-8.8s %c %5d %6ldK %6ldK %s\n", pid, user_str, proc->state, proc->num_threads, proc->vmsize, proc->vmrss, proc->name);  
    }

    sort_all_process(head);

    p = head;
    while (p->next != NULL)
    {
        p = p->next;

        user = getpwuid(p->uid);
        if (user && user->pw_name) {
            user_str = user->pw_name;
        } else {
            snprintf(user_buf, 20, "%d", p->uid);
            user_str = user_buf;
        }
        printf("%5d %-8.8s %c %5d %6ldK %6ldK %s\n", p->pid, user_str, p->state, p->num_threads, p->vmsize, p->vmrss, p->name);  
    } 
    printf("\n\n");
}

static void sort_all_process(struct proc_info *head){
    struct proc_info *p, *pb, temp;
    p = head->next;

    if(sort == 0 || p->next == NULL)
        return;

    while (p->next != NULL)
    {
        pb = p->next;
        while (pb != NULL)
        {
            if ((sort==1 && p->vmsize>pb->vmsize)||(sort==2 && p->vmrss>pb->vmrss))
            {
                temp = *p;
                *p = *pb;
                *pb = temp;
                temp.next = p->next;
                p->next = pb->next;
                pb->next = temp.next;
            }
            pb = pb->next;          
        }
        p = p->next;
    }  
}

static int read_stat(char *filename, struct proc_info *proc){
    FILE *file;
    char buf[MAX_LINE], *open_paren, *close_paren;
    int res, idx;

    file = fopen(filename, "r");
    if (!file) return 1;
    fgets(buf, MAX_LINE, file);
    fclose(file);

    open_paren = strchr(buf, '(');
    close_paren = strrchr(buf, ')');
    if (!open_paren || !close_paren) return 1;

    *open_paren = *close_paren = '\0';
    strncpy(proc->tname, open_paren + 1, THREAD_NAME_LEN);
    proc->tname[THREAD_NAME_LEN-1] = 0;

    sscanf(close_paren + 1, " %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "
                 "%lu %lu %*d %*d %*d %*d %*d %*d %*d",
                 &proc->state, &proc->utime, &proc->stime);
                //                  "%lu %lu %*d %*d %*d %*d %*d %*d %*d %lu %ld",
                //  &proc->state, &proc->utime, &proc->stime, &proc->vmsize, &proc->vmrss);
 
    return 0;
}

static int read_cmdline(char *filename, struct proc_info *proc){
    FILE *file;
    char line[MAX_LINE];

    line[0] = '\0';
    file = fopen(filename, "r");
    if (!file) return 1;
    fgets(line, MAX_LINE, file);
    fclose(file);
    if (strlen(line) > 0) {
        strncpy(proc->name, line, PROC_NAME_LEN);
        proc->name[PROC_NAME_LEN-1] = 0;
    } else
        proc->name[0] = 0;
    return 0;
}

static int read_status(char *filename, struct proc_info *proc){
    FILE *file;
    char line[MAX_LINE];
    unsigned int uid = 0, gid = 0;
    long vmsize = 0, vmrss = 0;
    // long rss_sh, rss_file, rss_anon;
    int thread_num = 0;
 
    file = fopen(filename, "r");
    if (!file) return 1;
    while (fgets(line, MAX_LINE, file)) {
        sscanf(line, "Uid: %u", &uid);
        sscanf(line, "Gid: %u", &gid);
        sscanf(line, "VmSize: %ld", &vmsize);
        sscanf(line, "VmRSS: %ld", &vmrss);
        // sscanf(line, "RssAnon: %ld", &rss_anon);
        // sscanf(line, "RssFile: %ld", &rss_file);
        // sscanf(line, "RssShmem: %ld", &rss_sh);
        sscanf(line, "Threads: %d", &thread_num);
    }
    fclose(file);
    proc->uid = uid; proc->gid = gid;
    proc->vmsize = vmsize; proc->vmrss = vmrss;
    // proc->rss_anon = rss_anon; proc->rss_file = rss_file; proc->rss_sh = rss_sh;
    proc->num_threads = thread_num;
    return 0;
}

static int read_meminfo(char *filename, struct mem_info *mem){
    FILE *file;
    char line[MAX_LINE];
    unsigned long mem_total = 0, mem_free = 0, mem_buffers = 0, mem_cached = 0;

    file = fopen(filename, "r");
    if (!file) return 1;
    while (fgets(line, MAX_LINE, file)) {
        sscanf(line,"MemTotal: %lu",&mem_total);
        sscanf(line,"MemFree: %lu", &mem_free);
        sscanf(line,"Buffers: %lu", &mem_buffers);
        sscanf(line,"Cached: %lu",&mem_cached);
    }
    fclose(file);
    mem->mem_total = mem_total;
    mem->mem_free = mem_free;
    mem->buffers = mem_buffers;
    mem->cached = mem->cached;
    return 0;
}

static void print_usage(){
    printf("show all process: -a\n");
    printf("show single process: -p\n");
    printf("show single thread: -t\n");
    printf("refresh showing: -f\n");
    printf("sort the process by vss, rss: -s1, -s2\n");
    printf("change refresh time: -fx, x(s) is the period you want, x must larger than 0\n\n");
}