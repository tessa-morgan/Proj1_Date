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
#include <unistd.h>

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
    static char err3[] = "Failed to run top \0";
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
                char buffer[MAX_LEN];

                // Run 'top' in batch mode to get CPU usage in a single snapshot
                fp = popen("top -bn1 | grep 'Cpu(s)'", "r");
                // Example output: %Cpu(s):  0.7 us,  1.1 sy,  0.1 ni, 98.0 id
                if (fp == NULL) {
                        ptr=err2;
                        break;
                }

                // Read the output of 'top'
                if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                        // Extract the CPU usage (idle percentage)
                        float user, idle, usage;
                        sscanf(buffer, "Cpu(s): %*f us, %*f sy, %*f ni, %f id,", &user, &usage, &usage, &idle);
                        usage = 100.0 - idle; // CPU usage is (100 - idle)

                        // Format the result as a string
                        snprintf(s, MAX_LEN, "User CPU Usage: %.2f%%\nTotal CPU Usage: %.2f%%", user, usage);
                        s[MAX_LEN - 1] = '\0';
                }

                pclose(fp);

                ptr=s;
                break;}

        case 5: {// Memory Usage
                FILE *fp;
                char buffer[MAX_LEN];
                double total_mem, free_mem, used_mem, mem_usage = 0.0;

                // Run 'top' and read its output
                fp = popen("top -b -n1 | grep 'MiB Mem'", "r");
                if (fp == NULL) {
                        ptr=err3;
                        break;
                }

                // Read the line from top output
                if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                        // Parse the memory values (KiB Mem:  total,  free,  used,  buff/cache)
                        sscanf(buffer, "MiB Mem : %lf total, %*lf free, %lf used,", &total_mem, &used_mem);
                        
                        if (total_mem > 0) {
                                mem_usage = (used_mem / total_mem) * 100.0;
                        }
                }

                pclose(fp);
                snprintf(s, MAX_LEN, "Current Memory Usage: %.2f%%\n", mem_usage);
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

                snprintf(s, MAX_LEN, "Total number of processes running: %d", process_count);
                s[MAX_LEN - 1] = '\0';
                ptr=s;
                break;}
        
        case 7: {// Load average
                FILE *fp;
                char buffer[MAX_LEN];
                char cpu_usage[MAX_LEN];

                // Run 'top' in batch mode
                fp = popen("top -bn1 | grep 'load average'", "r");
                if (fp == NULL) {
                        ptr=err2;
                        break;
                }

                // Read the output and store it in s
                if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                        char load_avg_1[10], load_avg_2[10], load_avg_3[10];

                // Look for "load average:" and extract the three values
                if (sscanf(buffer, "%*[^l]load average: %9[^,], %9[^,], %9s", load_avg_1, load_avg_2, load_avg_3) == 3) {
                        snprintf(s, MAX_LEN, "System Load Average (1, 5, and 15 min): %s, %s, %s", load_avg_1, load_avg_2, load_avg_3);
                } else {
                        snprintf(s, MAX_LEN, "Error: Unable to parse load average.");
                }
}

                pclose(fp);
                ptr=s;
                break; }

        default: ptr=err;
                 break;
        }
    return(&ptr);
}