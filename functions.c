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
        syslog(LOG_ERR, "Blad z pobraniem daty modyfikacji dla pliku %s", path);
        exit(EXIT_FAILURE);
    }
    return st.st_mtime;
}

/* Return a chmod */
mode_t get_chmod(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        syslog(LOG_ERR, "Blad pobrania chmod dla pliku %s", path);
        exit(EXIT_FAILURE);
    }
    return st.st_mode;
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

    // po co to tutaj teraz???
    mode_t old = get_chmod(mainFile);
    if (chmod(updatedFile, old) != 0)
    {
        syslog(LOG_ERR, "Blad ustawienia uprawnien do pliku!");
        exit(EXIT_FAILURE);
    }
}

/* change file directory */
char *change_path(char *file_path, char *current_directory, char *target_directory) //1. pełna ścieżka do pliku np.home / zrodlo / file, 2. katalog zrodlowy, 3. katalog docelowy
{
    char *filename = file_path + strlen(current_directory);                                    //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
    char *updated_file_path = (char *)malloc(strlen(target_directory) + strlen(filename) + 1); //alokujemy pamiec na nowa sciezke
    strcpy(updated_file_path, target_directory);                                               //zamiana katalogu ze zrodlowego na docelowy, czyli doklejamy do pustej zmiennej nazwe katalogu docelowego
    strcat(updated_file_path, filename);                                                       //dodanie nazwy pliku

    return updated_file_path;
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
    char *searched_path = (char *)malloc(strlen(filename));                          //alokacja pamieci
    char *updated_path = change_path(filepath, current_directory, target_directory); //otrzymujemy zamieniony folder zrodlowy z docelowym

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
void delete_files(char *current_directory, char *main_directory, char *target_directory, bool flag_R)
{
    struct dirent *file;
    DIR *dir_path, *helper;
    dir_path = opendir(current_directory);
    while ((file = readdir(dir_path)))
    {
        if ((file->d_type) == DT_DIR) //    GDY JEST FOLDEREM
        {
            if (flag_R)
            {
                if (!(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0))
                {
                    char *new_dir_path = get_file_path(current_directory, file->d_name);
                    if (!(helper = opendir(change_path(new_dir_path, target_directory, main_directory))))
                    {
                        syslog(LOG_INFO, "Usunieto katalog %s", new_dir_path);
                        remove(new_dir_path);
                    }
                    else
                    {
                        closedir(helper);
                        delete_files(new_dir_path, main_directory, target_directory, flag_R);
                    }
                }
            }
        }
        else
        {
            char *new_dir_path = get_file_path(current_directory, file->d_name);
            if (access(change_path(new_dir_path, target_directory, main_directory), F_OK) == -1)
            {
                syslog(LOG_INFO, "Usunieto file %s", new_dir_path);
                remove(new_dir_path);
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
        syslog(LOG_ERR, "Blad w otwarciu pliku!");
        exit(EXIT_FAILURE);
    }

    while ((read_file_desc = read(read_file, buffer, sizeof(buffer))) > 0)
    {
        write_file_desc = write(write_file, buffer, (ssize_t)read_file_desc);
        if (write_file_desc != read_file_desc)
        {
            perror("BLAD");
            exit(EXIT_FAILURE);
        }
    }
    close(read_file);
    close(write_file);
    change_modification_time(read_file_path, write_file_path);
    syslog(LOG_INFO, "Skopiowano file %s", read_file_path);
}

/* Map one file content to another */
void map_file(char *read_file_path, char *write_file_path)
{
    int read_file_size = get_size(read_file_path);
    int read_file = open(read_file_path, O_RDONLY);
    int write_file = open(write_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (read_file == -1 || write_file == -1)
    {
        syslog(LOG_ERR, "Blad w otwarciu pliku!");
        exit(EXIT_FAILURE);
    }

    char *map = (char *)mmap(0, read_file_size, PROT_READ, MAP_SHARED | MAP_FILE, read_file, 0);

    write(write_file, map, read_file_size);

    close(read_file);
    close(write_file);
    munmap(map, read_file_size); //usuwanie mapy z paamieci;
    zmien_parametry(read_file_path, write_file_path);
    syslog(LOG_INFO, "Z uzyciem mapowania skopiowano file %s do miejsca %s", read_file_path, write_file_path);
}

/* update files and folders */
/* in target directory if needed */
void update_target_folder(char *current_directory, char *main_directory, char *target_directory, bool flag_R, int file_size)
//1. folder źródłowy, 2. folder źródłowy, 3. folder docelowy, 4. rekurencja, 5. wielkość pliku
{
    printf("JESTESMY W : %s\n", current_directory);
    struct dirent *file;                   //struct dirent - struktura wskazująca na element w katalogu (file/folder), zawiera nazwę pliku
    DIR *curr_dir, *helper;                //funkcja otwierająca strumień do katalogu
    curr_dir = opendir(current_directory); //przechodzimy do folderu źródłowego
    char *new_path;
    while ((file = readdir(curr_dir)))
    {
        printf("%s  \n", file->d_name);
        if ((file->d_type) == DT_DIR) //GDY JEST FOLDEREM
        {
            if (flag_R)
            {
                if (!(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0))
                {
                    char *path_to_directory = change_path(get_file_path(current_directory, file->d_name), main_directory, target_directory);
                    if (!(helper = opendir(path_to_directory)))
                    {
                        syslog(LOG_INFO, "Stworzono folder %s", path_to_directory);
                        mkdir(path_to_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    }
                    else
                    {
                        closedir(helper);
                    }
                    new_path = get_file_path(current_directory, file->d_name);
                    update_target_folder(new_path, main_directory, target_directory, flag_R, file_size);
                }
            }
        }
        else if ((file->d_type) == DT_REG) // GDY nie jest folderem, DT_REG to file regularny
        {
            new_path = get_file_path(current_directory, file->d_name); // tworzymy nową ścieżkę dodając do folderu źródłowego nazwę pliku, który jest przetwarzany
            int i;
            if ((i = check_file(new_path, main_directory, target_directory)) == 1) //1. pełna ścieżka do pliku np. home/zrodlo/file, 2. katalog zrodlowy, 3. katalog docelowy, zwraca nam 1 w przypadku gdy nie ma pliku w katalogu domowym lub czasy sie nie zgadzaja
            {
                if (get_size(new_path) > file_size)
                {
                    map_file(new_path, change_path(new_path, main_directory, target_directory));
                }
                else
                {
                    copy(new_path, change_path(new_path, main_directory, target_directory));
                }
            }
        }
    }
    closedir(curr_dir);
}

/* Starting daemon */
void start_daemon(int sig)
{
    syslog(LOG_INFO, "Wybudzenie demona przez sygnal SIGUSR1");
}