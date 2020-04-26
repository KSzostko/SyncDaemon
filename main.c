#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include "functions.h"

int main(int argc, char **argv)
{
    openlog("Sync Daemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);

    if (argc < 4)
    {
        printf("You have not specified enough arguments\n");
        syslog(LOG_ERR, "You have not specified enough arguments");
        exit(EXIT_FAILURE);
    }

    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }

    struct stat st;
    int c;
    int size = 10 * 1000000, time = 300, recursion = 0;
    char *in_path = NULL;
    char *out_path = NULL;

    while ((c = getopt(argc, argv, "i:o:s:t:R")) != -1)
    {
        switch (c)
        {
        case 'i':
            if (stat(optarg, &st) == 0)
            {
                if (S_IFDIR & st.st_mode)
                {
                    in_path = optarg;
                }
                else
                {
                    printf("You have not specified valid directory\n");
                    syslog(LOG_ERR, "You have not specified valid input directory");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case 'o':
            if (stat(optarg, &st) == 0)
            {
                if (S_IFDIR & st.st_mode)
                {
                    out_path = optarg;
                }
                else
                {
                    printf("You have not specified valid directory\n");
                    syslog(LOG_ERR, "You have not specified valid output directory");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case 's':
            size = atoi(optarg);
            size *= 1000000;
            break;
        case 't':
            time = atoi(optarg);
            break;
        case 'R':
            recursion = 1;
            break;
        default:
            printf("You have specified unknown parameter\n");
            syslog(LOG_ERR, "You have specified unknown parameter\n");
            abort();
        }
    }

    if (in_path == NULL || out_path == NULL)
    {
        printf("You have not specified both requiered paths\n");
        syslog(LOG_ERR, "You have not specified both requiered paths\n");
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (signal(SIGUSR1, start_daemon) == SIG_ERR)
    {
        syslog(LOG_ERR, "SIGUSR1 signal error");
        exit(EXIT_FAILURE);
    }

    /* The Big Loop */
    while (1)
    {
        delete_files(out_path, in_path, recursion);
        update_target_folder(in_path, out_path, recursion, size);
        syslog(LOG_INFO, "Folders synchronized");

        if (sleep(time) == 0)
        {
            syslog(LOG_INFO, "Daemon working again");
        }
    }

    closelog();
    exit(EXIT_SUCCESS);
}