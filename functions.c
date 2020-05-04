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
void change_modification_time(char *mainFile, char *updatedFile) // parametrami pliki z katalogu zrodlowego i docelowego
{
    struct utimbuf timebuf; //specjalna struktura do zmiany czasu dostepu i modyfikacji
    timebuf.actime = 0;
    timebuf.modtime = get_last_modification_time(mainFile); //pobieramy czas modyfikacji z pliku zrodlowego
    if (utime(updatedFile, &timebuf) != 0)
    {
        syslog(LOG_ERR, "Time modification failed");
        exit(EXIT_FAILURE);
    }
}

/* Return full path to file */
char *get_file_path(char *path, char *filename)
{
    char *full_path = (char *)malloc(strlen(path) + 2 + strlen(filename)); //alokujemy pamiec na nowa sciezke
    strcpy(full_path, path); //doklejamy nazwe katalogu docelowego
    strcat(full_path, "/");
    strcat(full_path, filename); //doklejamy do zmiennej nazwe pliku
    full_path[strlen(path) + 1 + strlen(filename)] = '\0';

    return full_path;
}

/* check if file exsists in both directories and has the same modification time */
bool check_file(char *filepath, char *current_directory, char *target_directory) //1. pe³na œcie¿ka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy
{
    bool result = 0;
    char *filename = filepath + strlen(current_directory);  //otrzymujemy sama nazwe pliku przez uciecie nazwy katalogu
    char *searched_path = (char *)malloc(strlen(filename)); //alokacja pamieci
    char *updated_path = get_file_path(target_directory, filename); //otrzymujemy zamieniony folder zrodlowy z docelowym

    int i = strlen(updated_path);
    for (i; updated_path[i] != '/'; i--);
    strcpy(searched_path, updated_path + i + 1); //pod "searched_path" zostaje wpisana nazwa pliku
    updated_path[i] = '\0';
    struct dirent *file;
    DIR *dir;
    dir = opendir(updated_path); //otwieramy strumien do sciezki docelowej

    while ((file = readdir(dir)))
    {
        if (strcmp(file->d_name, searched_path) == 0) //jesli nazwa pliku z katalogu i szukanego jest taka sama
        {
            free(searched_path); 
            if ((file->d_type) == DT_DIR) //GDY JEST FOLDEREM
            {
                return 0;
            }
            else
            {
                int time1 = (int)get_last_modification_time(filepath); //pobieramy ostatni czas modyfikacji pliku z katalogu zrodlowego
                int time2 = (int)get_last_modification_time(get_file_path(updated_path, file->d_name)); //pobieramy ostatni czas modyfikacji pliku z katalogu docelowego
                if (time1 == time2)
                {
                    return 0; //jesli czasy takie same to return 0
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
/* if there's something that's not in main directory */
/* delete that */
void delete_files(char *main_directory, char *target_directory, bool flag_R)
{
    struct dirent *file; //struct dirent - struktura wskazuj¹ca na element w katalogu (plik/folder), zawiera nazwê pliku
    DIR *dir_path, *helper; //funkcja otwieraj¹ca strumieñ do katalogu
    dir_path = opendir(main_directory); //przechodzimy do folderu docelowego
    while ((file = readdir(dir_path)))
    {
        if ((file->d_type) == DT_DIR) //GDY JEST FOLDEREM
        {
            if (flag_R)
            {
                if (!(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0))
                {
                    char *new_main_path = get_file_path(main_directory, file->d_name);
                    char *new_target_path = get_file_path(target_directory, file->d_name);

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
            char *new_main_path = get_file_path(main_directory, file->d_name); //tworzymy now¹ œcie¿kê dodaj¹c do folderu docelowego nazwê pliku, który jest przetwarzany np /home/docelowy/plik
            char *new_target_path = get_file_path(target_directory, file->d_name); //tworzymy now¹ œcie¿kê dodaj¹c do folderu docelowego nazwê pliku, który jest przetwarzany np /home/zrodlowy/plik
            if (access(new_target_path, F_OK) == -1) //sprawdzamy czy mozna usunac plik
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
    int read_file_size = get_size(read_file_path); //pobieramy rozmiar wejscia w bitach
    int read_file = open(read_file_path, O_RDONLY);
    int write_file = open(write_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (read_file == -1 || write_file == -1)
    {
        syslog(LOG_ERR, "Opening file error");
        exit(EXIT_FAILURE);
    }

    char *map = (char *)mmap(0, read_file_size, PROT_READ, MAP_SHARED | MAP_FILE, read_file, 0);

    write(write_file, map, read_file_size); //zapis do pliku

    close(read_file);
    close(write_file);
    munmap(map, read_file_size); //usuwanie mapy z pamieci;
    change_modification_time(read_file_path, write_file_path);
    syslog(LOG_INFO, "File from %s successfully mapped to %s", read_file_path, write_file_path);
}

/* update files and folders */
/* in target directory if needed */
void update_target_folder(char *main_directory, char *target_directory, bool flag_R, int file_size)
{
    struct dirent *file; //struct dirent - struktura wskazuj¹ca na element w katalogu (plik/folder), zawiera nazwê pliku
    DIR *main_dir, *helper; //funkcja otwieraj¹ca strumieñ do katalogu
    main_dir = opendir(main_directory); //przechodzimy do folderu Ÿród³owego
    char *new_path;

    while ((file = readdir(main_dir)))
    {
        if ((file->d_type) == DT_DIR)
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
        else if ((file->d_type) == DT_REG) // GDY jest to plik regularny
        {
            new_path = get_file_path(main_directory, file->d_name); //tworzymy now¹ œcie¿kê dodaj¹c do folderu Ÿród³owego nazwê pliku, który jest przetwarzany np /home/projekt/plik
            int i;

            if ((i = check_file(new_path, main_directory, target_directory)) == 1) //1. pe³na œcie¿ka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy, zwraca nam 1 w przypadku gdy nie ma pliku w katalogu domowym lub czasy sie nie zgadzaja
            {
                if (get_size(new_path) > file_size) //jesli rozmiar pliku jest wiekszy od podanego przez parametr to plik kopiujemy przez mapowanie
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