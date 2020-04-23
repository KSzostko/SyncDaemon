#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
off_t get_size(char *path);
time_t get_last_modification_time(char *path);
void change_modification_time(char *mainFile, char *updatedFile);
char *get_file_path(char *path, char *filename);
bool check_file(char *filepath, char *current_directory, char *target_directory);
void copy(char *read_file_path, char *write_file_path);
void map_file(char *read_file_path, char *write_file_path);
void delete_files(char *main_directory, char *target_directory, bool flag_R);
void update_target_folder(char *main_directory, char *target_directory, bool flag_R, int file_size);
void start_daemon(int sig);
#endif