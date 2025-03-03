/*
 * date_proc.c - remote procedures; called by server stub.
 */
#include <rpc/rpc.h>	/* standard RPC include file */
#include <time.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/resource.h>

#include "date.h"	/* this file is generated by rpcgen */

#define MAX_LEN 100
#define MAX_LINE_LEN 256
#define MAX_STR_LEN 100



/*
 * Return the binary date and time.
 */
char ** date_1(long *option)
{
    struct tm *timeptr; /* Pointer to time structure      */
    time_t clock;       /* Clock value (in secs)          */
    static char *ptr;   /* Return string                  */
    static char err[] = "Invalid Response \0";
    static char err2[] = "Error executing command \0";
    static char s[MAX_LEN];

    clock = time(0);
    timeptr = localtime(&clock);

    switch(*option)
        {
        case 1: // Date Only
                strftime(s,MAX_LEN,"%A, %B %d, %Y",timeptr);
                ptr=s;
                break;

        case 2: // Time Only
                strftime(s,MAX_LEN,"%T",timeptr);
                ptr=s;
                break;

        case 3: // Time and Date
                strftime(s,MAX_LEN,"%A, %B %d, %Y - %T",timeptr);
                ptr=s;
                break;

        case 4: {// CPU Usage
                FILE *fp;
                double cpu_usage = 0.0;
                char line[256];
                unsigned long long prev_total = 0, prev_idle = 0;
                unsigned long long total, idle, user, nice, system, iowait, irq, softirq, steal;
                
                // Read first line of /proc/stat
                fp = fopen("/proc/stat", "r");
                if (fp == NULL) {
                        ptr = err2;
                        break;
                }
                
                if (fgets(line, sizeof(line), fp) != NULL) {
                        sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
                        prev_total = user + nice + system + idle + iowait + irq + softirq + steal;
                        prev_idle = idle;
                }
                fclose(fp);
                
                // Wait for 1 second
                sleep(1);
                
                // Read the updated CPU stats
                fp = fopen("/proc/stat", "r");
                if (fp == NULL) {
                        perror("Error opening /proc/stat");
                        return;
                }

                if (fgets(line, sizeof(line), fp) != NULL) {
                        sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
                        total = user + nice + system + idle + iowait + irq + softirq + steal;
                }
                fclose(fp);

                // Calculate deltas
                unsigned long long total_delta = total - prev_total;
                unsigned long long idle_delta = idle - prev_idle;
                
                // Calculate CPU usage percentage
                if (total_delta > 0) {
                        cpu_usage = (double)(total_delta - idle_delta) / total_delta * 100.0;
                } else {
                        cpu_usage = 0.0;
                }

                snprintf(s, MAX_LEN, "CPU Usage: %.2f%%\n", cpu_usage);

                ptr=s;
                break;}

        case 5: {// Memory Usage
                struct rusage r_usage;
                getrusage(RUSAGE_SELF,&r_usage);
                snprintf(s, MAX_LEN, "Memory usage: %ld kilobytes", r_usage.ru_maxrss);
                s[MAX_LEN - 1] = '\0';
                ptr=s;
                break;}

        case 6: {// Process Count
                int process_count = 0;
                FILE *fp;
                char command[50];
                char path[100];

                sprintf(command, "ps -e | wc -l");

                fp = popen(command, "r");
                if (fp == NULL) {
                        ptr=err2;
                        break;
                }

                if (fgets(path, sizeof(path), fp) != NULL) {
                        process_count = atoi(path);
                        process_count--; 
                }

                pclose(fp);

                snprintf(s, MAX_LEN, "Number of processes running: %d", process_count);
                s[MAX_LEN - 1] = '\0';
                ptr=s;
                break;}
        
        case 7: // Load procs per minute
                ptr=s;
                break;

        case 8: // End
                break;

        default: ptr=err;
                 break;
        }
    return(&ptr);
}