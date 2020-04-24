#include "functions.h"

/* Return a file size */
off_t get_size(char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        return st.st_size;
    }
    return -1;
}

/* Return a last modification time */
time_t get_last_modification_time(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        syslog(LOG_ERR, "Unable to get last modification time for the file %s", path);
        exit(EXIT_FAILURE);
    }
    return st.st_mtime;
}

/* Change modification time */
void change_modification_time(char *mainFile, char *updatedFile)
{
    struct utimbuf timebuf;
    timebuf.actime = 0;
    timebuf.modtime = get_last_modification_time(mainFile);
    if (utime(updatedFile, &timebuf) != 0)
    {
        syslog(LOG_ERR, "Time modification failed");
        exit(EXIT_FAILURE);
    }
}

/* Return full path to file */
char *get_file_path(char *path, char *filename) //1. ścieżka źródłowa, 2. nazwa pliku
{
    char *full_path = (char *)malloc(strlen(path) + 2 + strlen(filename)); //alokujemy pamięć na nową nazwę ścieżki
    strcpy(full_path, path);                                               //dodajemy początek ścieżki np. /home/zrodlo
    strcat(full_path, "/");                                                // /home/zrodlo/
    strcat(full_path, filename);                                           // /home/zrodlo/plik1
    full_path[strlen(path) + 1 + strlen(filename)] = '\0';

    return full_path;
}

/* check if file exsists in both directories and has the same modification time */
bool check_file(char *filepath, char *current_directory, char *target_directory) //1. pełna ścieżka do pliku np. home/zrodlo/file, 2. katalog zrodlowy, 3. katalog docelowy
{
    bool result = 0;
    char *filename = filepath + strlen(current_directory); //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
    // ja to mysle ze ta zmienna szukany do wywalenia ale to trzeba sprawdzic
    char *searched_path = (char *)malloc(strlen(filename));         //alokacja pamieci
    char *updated_path = get_file_path(target_directory, filename); //otrzymujemy zamieniony folder zrodlowy z docelowym

    int i = strlen(updated_path);
    for (i; updated_path[i] != '/'; i--)
        ;
    strcpy(searched_path, updated_path + i + 1); //pod "searched_path" wpisywana jest nazwa pliku, nowa sciezka zostaje sama sciekza docelowa
    updated_path[i] = '\0';
    struct dirent *file;
    DIR *dir;
    dir = opendir(updated_path); //otwieramy strumien do sciezki docelowej

    while ((file = readdir(dir)))
    {
        if (strcmp(file->d_name, searched_path) == 0) //jesli nazwa pliku z katalogu i szukanego jest taka sama to
        {
            free(searched_path);
            if ((file->d_type) == DT_DIR) //    GDY JEST FOLDEREM
            {
                return 0;
            }
            else
            {
                int time1 = (int)get_last_modification_time(filepath);
                int time2 = (int)get_last_modification_time(get_file_path(updated_path, file->d_name)); //pobieramy czas modyfikacji pliku ze zrodlowego i z docelowego
                if (time1 == time2)                                                                     //jesli czasy takie same to return 0
                {
                    return 0;
                }
                else
                {
                    return 1; //jezeli czasy rozne to trzeba poprawic wiec zwraca 1
                }
            }
        }
        else
        {
            result = 1; //jezeli nie ma takiego pliku w docelowym to trzeba przekopiowac
        }
    }
    closedir(dir);
    return result;
}

/* Delete file */
/* listing files and folders from target directory */
/* if there's someting that's not in main directory */
/* delete that */
void delete_files(char *main_directory, char *target_directory, bool flag_R)
{
    struct dirent *file;
    DIR *dir_path, *helper;
    dir_path = opendir(main_directory);
    while ((file = readdir(dir_path)))
    {
        if ((file->d_type) == DT_DIR) //    GDY JEST FOLDEREM
        {
            if (flag_R)
            {
                if (!(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0))
                {
                    char *new_main_path = get_file_path(main_directory, file->d_name);
                    char *new_target_path = get_file_path(target_directory, file->d_name);

                    // to jest w tym miejscu bo najpierw trzeba usunąć wszystko z folderu by móc go usunąć
                    delete_files(new_main_path, new_target_path, flag_R);
                    if (!(helper = opendir(new_target_path)))
                    {
                        syslog(LOG_INFO, "Directory %s deleted", new_main_path);
                        remove(new_main_path);
                    }
                    else
                    {
                        closedir(helper);
                    }
                }
            }
        }
        else
        {
            char *new_main_path = get_file_path(main_directory, file->d_name);
            char *new_target_path = get_file_path(target_directory, file->d_name);
            if (access(new_target_path, F_OK) == -1)
            {
                syslog(LOG_INFO, "File %s deleted", new_main_path);
                remove(new_main_path);
            }
        }
    }
    closedir(dir_path);
}

/* Copy content from file to another */
void copy(char *read_file_path, char *write_file_path)
{
    char buffer[16];
    int read_file, write_file;
    int read_file_desc, write_file_desc;
    read_file = open(read_file_path, O_RDONLY);
    write_file = open(write_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (read_file == -1 || write_file == -1)
    {
        syslog(LOG_ERR, "Opening file error");
        exit(EXIT_FAILURE);
    }

    while ((read_file_desc = read(read_file, buffer, sizeof(buffer))) > 0)
    {
        write_file_desc = write(write_file, buffer, (ssize_t)read_file_desc);
        if (write_file_desc != read_file_desc)
        {
            syslog(LOG_ERR, "Writing to file failed");
            exit(EXIT_FAILURE);
        }
    }
    close(read_file);
    close(write_file);
    change_modification_time(read_file_path, write_file_path);
    syslog(LOG_INFO, "File %s copied", read_file_path);
}

/* Map one file content to another */
void map_file(char *read_file_path, char *write_file_path)
{
    int read_file_size = get_size(read_file_path);
    int read_file = open(read_file_path, O_RDONLY);
    int write_file = open(write_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (read_file == -1 || write_file == -1)
    {
        syslog(LOG_ERR, "Opening file error");
        exit(EXIT_FAILURE);
    }

    char *map = (char *)mmap(0, read_file_size, PROT_READ, MAP_SHARED | MAP_FILE, read_file, 0);

    write(write_file, map, read_file_size);

    close(read_file);
    close(write_file);
    munmap(map, read_file_size); //usuwanie mapy z paamieci;
    change_modification_time(read_file_path, write_file_path);
    syslog(LOG_INFO, "File from %s successfully mapped to %s", read_file_path, write_file_path);
}

/* update files and folders */
/* in target directory if needed */
void update_target_folder(char *main_directory, char *target_directory, bool flag_R, int file_size)
//1. folder źródłowy, 2. folder źródłowy, 3. folder docelowy, 4. rekurencja, 5. wielkość pliku
{
    struct dirent *file;                //struct dirent - struktura wskazująca na element w katalogu (file/folder), zawiera nazwę pliku
    DIR *main_dir, *helper;             //funkcja otwierająca strumień do katalogu
    main_dir = opendir(main_directory); //przechodzimy do folderu źródłowego
    char *new_path;
    while ((file = readdir(main_dir)))
    {
        if ((file->d_type) == DT_DIR) //GDY JEST FOLDEREM
        {
            if (flag_R)
            {
                if (!(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0))
                {
                    char *path_to_directory = get_file_path(target_directory, file->d_name);
                    if (!(helper = opendir(path_to_directory)))
                    {
                        syslog(LOG_INFO, "Directory %s created", path_to_directory);
                        mkdir(path_to_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    }
                    else
                    {
                        closedir(helper);
                    }
                    char *new_main_path = get_file_path(main_directory, file->d_name);
                    update_target_folder(new_main_path, path_to_directory, flag_R, file_size);
                }
            }
        }
        else if ((file->d_type) == DT_REG) // GDY nie jest folderem, DT_REG to file regularny
        {
            new_path = get_file_path(main_directory, file->d_name); // tworzymy nową ścieżkę dodając do folderu źródłowego nazwę pliku, który jest przetwarzany
            int i;
            if ((i = check_file(new_path, main_directory, target_directory)) == 1) //1. pełna ścieżka do pliku np. home/zrodlo/file, 2. katalog zrodlowy, 3. katalog docelowy, zwraca nam 1 w przypadku gdy nie ma pliku w katalogu domowym lub czasy sie nie zgadzaja
            {
                if (get_size(new_path) > file_size)
                {
                    map_file(new_path, get_file_path(target_directory, file->d_name));
                }
                else
                {
                    copy(new_path, get_file_path(target_directory, file->d_name));
                }
            }
        }
    }
    closedir(main_dir);
}

/* Starting daemon */
void start_daemon(int sig)
{
    syslog(LOG_INFO, "Dameon started after receiving SIGURS1 signal");
}