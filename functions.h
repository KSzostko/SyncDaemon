#ifndef FUNCTIONS_H
#define FUNCTIONS_H
off_t get_size(char *path);
time_t get_last_modification_time(char *path);
mode_t get_chmod(char *path);
void change_modification_time(char *mainFile, char *updatedFile);
char *change_path(char *file_path, char *current_directory, char *target_directory);
char *get_file_path(char *path, char *filename);
bool check_file(char *filepath, char *current_directory, char *target_directory);
void copy(char *read_file_path, char *write_file_path);
void map_file(char *read_file_path, char *write_file_path);
void delete_files(char *current_directory, char *main_directory, char *target_directory, bool flag_R);
void update_target_folder(char *current_directory, char *main_directory, char *target_directory, bool flag_R, int file_size);
void start_daemon(int sig);
#endif