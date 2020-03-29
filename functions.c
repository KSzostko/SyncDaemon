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
char *change_path(char *file_path, char *current_directory, char *target_directory) //1. pełna ścieżka do pliku np.home / zrodlo / plik, 2. katalog zrodlowy, 3. katalog docelowy
{
    char *filename = file_path + strlen(current_directory);                            //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
    char *updated_file_path = malloc(strlen(target_directory) + strlen(filename) + 1); //alokujemy pamiec na nowa sciezke
    strcpy(updated_file_path, target_directory);                                       //zamiana katalogu ze zrodlowego na docelowy, czyli doklejamy do pustej zmiennej nazwe katalogu docelowego
    strcat(updated_file_path, filename);                                               //dodanie nazwy pliku

    return updated_file_path;
}

/* Return full path to file */
char *get_file_path(char *path, char *filename) //1. ścieżka źródłowa, 2. nazwa pliku
{
    char *full_path = malloc(strlen(path) + 2 + strlen(filename)); //alokujemy pamięć na nową nazwę ścieżki
    strcpy(full_path, path);                                       //dodajemy początek ścieżki np. /home/zrodlo
    strcat(full_path, "/");                                        // /home/zrodlo/
    strcat(full_path, filename);                                   // /home/zrodlo/plik1
    full_path[strlen(path) + 1 + strlen(filename)] = '\0';

    return full_path;
}

/* check if file exsists in both directories and has the same modification time */
bool check_file(char *filepath, char *current_directory, char *target_directory) //1. pełna ścieżka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy
{
    bool result = 0;
    char *filename = filepath + strlen(current_directory); //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
    // ja to mysle ze ta zmienna szukany do wywalenia ale to trzeba sprawdzic
    char *szukamy = malloc(strlen(filename));                                        //alokacja pamieci
    char *updated_path = change_path(filepath, current_directory, target_directory); //otrzymujemy zamieniony folder zrodlowy z docelowym

    int i = strlen(updated_path);
    for (i; updated_path[i] != '/'; i--)
        ;
    strcpy(szukamy, updated_path + i + 1); //pod "szukamy" wpisywana jest nazwa pliku, nowa sciezka zostaje sama sciekza docelowa
    updated_path[i] = '\0';
    struct dirent *plik;
    DIR *dir;
    dir = opendir(updated_path); //otwieramy strumien do sciezki docelowej

    while ((file = readdir(dir)))
    {
        if (strcmp(file->d_name, szukamy) == 0) //jesli nazwa pliku z katalogu i szukanego jest taka sama to
        {
            free(szukamy);
            if ((file->d_type) == DT_DIR) //    GDY JEST FOLDEREM
            {
                return 0;
            }
            else
            {
                int time1 = (int)pobierz_czas(filepath), time2 = (int)pobierz_czas(dodaj_do_sciezki(updated_path, file->d_name)); //pobieramy czas modyfikacji pliku ze zrodlowego i z docelowego
                if (time1 == time2)                                                                                               //jesli czasy takie same to return 0
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

void Usuwanie(char *nazwa_sciezki_folder2, char *sciezka_folderu1, char *sciezka_folderu2, bool CzyR)
{
    struct dirent *plik;
    DIR *sciezka, *pom;
    sciezka = opendir(nazwa_sciezki_folder2);
    while ((plik = readdir(sciezka)))
    {
        if ((plik->d_type) == DT_DIR) //    GDY JEST FOLDEREM
        {
            if (CzyR)
            {
                if (!(strcmp(plik->d_name, ".") == 0 || strcmp(plik->d_name, "..") == 0))
                {
                    char *nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki_folder2, plik->d_name);
                    Usuwanie(nowa_sciezka, sciezka_folderu1, sciezka_folderu2, CzyR);
                    if (!(pom = opendir(podmien_folder2(nowa_sciezka, sciezka_folderu1, sciezka_folderu2))))
                    {
                        syslog(LOG_INFO, "Usunieto katalog %s", nowa_sciezka);
                        remove(nowa_sciezka);
                    }
                    else
                    {
                        closedir(pom);
                    }
                }
            }
        }
        else
        {
            char *nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki_folder2, plik->d_name);
            if (access(podmien_folder2(nowa_sciezka, sciezka_folderu1, sciezka_folderu2), F_OK) == -1)
            {
                syslog(LOG_INFO, "Usunieto plik %s", nowa_sciezka);
                remove(nowa_sciezka);
            }
        }
    }
    closedir(sciezka);
}

void kopiuj(char *wej, char *wyj)
{
    char bufor[16];
    int plikwej, plikwyj;
    int czytajwej, czytajwyj;
    plikwej = open(wej, O_RDONLY);
    plikwyj = open(wyj, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (plikwej == -1 || plikwyj == -1)
    {
        syslog(LOG_ERR, "Blad w otwarciu pliku!");
        exit(EXIT_FAILURE);
    }

    while ((czytajwej = read(plikwej, bufor, sizeof(bufor))) > 0)
    {
        czytajwyj = write(plikwyj, bufor, (ssize_t)czytajwej);
        if (czytajwyj != czytajwej)
        {
            perror("BLAD");
            exit(EXIT_FAILURE);
        }
    }
    close(plikwej);
    close(plikwyj);
    zmien_parametry(wej, wyj);
    syslog(LOG_INFO, "Skopiowano plik %s", wej);
}
void kopiuj_mapowanie(char *wej, char *wyj)
{
    int rozmiar = pobierz_rozmiar(wej);
    int plikwej = open(wej, O_RDONLY);
    int plikwyj = open(wyj, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (plikwej == -1 || plikwyj == -1)
    {
        syslog(LOG_ERR, "Blad w otwarciu pliku!");
        exit(EXIT_FAILURE);
    }

    char *mapa = (char *)mmap(0, rozmiar, PROT_READ, MAP_SHARED | MAP_FILE, plikwej, 0);

    write(plikwyj, mapa, rozmiar);

    close(plikwej);
    close(plikwyj);
    munmap(mapa, rozmiar); //usuwanie mapy z paamieci;
    zmien_parametry(wej, wyj);
    syslog(LOG_INFO, "Z uzyciem mapowania skopiowano plik %s do miejsca %s", wej, wyj);
}
void PrzegladanieFolderu(char *nazwa_sciezki1, char *sciezka_folderu1, char *sciezka_folderu2, bool CzyR, int Wielkosc_pliku)
//1. folder źródłowy, 2. folder źródłowy, 3. folder docelowy, 4. rekurencja, 5. wielkość pliku
{
    printf("JESTESMY W : %s\n", nazwa_sciezki1);
    struct dirent *plik;               //struct dirent - struktura wskazująca na element w katalogu (plik/folder), zawiera nazwę pliku
    DIR *sciezka, *pom;                //funkcja otwierająca strumień do katalogu
    sciezka = opendir(nazwa_sciezki1); //przechodzimy do folderu źródłowego
    char *nowa_sciezka;
    while ((plik = readdir(sciezka)))
    {
        printf("%s  \n", plik->d_name);
        if ((plik->d_type) == DT_DIR) //GDY JEST FOLDEREM
        {
            if (CzyR)
            {
                if (!(strcmp(plik->d_name, ".") == 0 || strcmp(plik->d_name, "..") == 0))
                {
                    char *sciekza_do_folderu = podmien_folder1(dodaj_do_sciezki(nazwa_sciezki1, plik->d_name), sciezka_folderu1, sciezka_folderu2);
                    if (!(pom = opendir(sciekza_do_folderu)))
                    {
                        syslog(LOG_INFO, "Stworzono folder %s", sciekza_do_folderu);
                        mkdir(sciekza_do_folderu, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    }
                    else
                    {
                        closedir(pom);
                    }
                    nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki1, plik->d_name);
                    PrzegladanieFolderu(nowa_sciezka, sciezka_folderu1, sciezka_folderu2, CzyR, Wielkosc_pliku);
                }
            }
        }
        else if ((plik->d_type) == DT_REG) // GDY nie jest folderem, DT_REG to plik regularny
        {
            nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki1, plik->d_name); // tworzymy nową ścieżkę dodając do folderu źródłowego nazwę pliku, który jest przetwarzany
            int i;
            if ((i = sprawdzanie(nowa_sciezka, sciezka_folderu1, sciezka_folderu2)) == 1) //1. pełna ścieżka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy, zwraca nam 1 w przypadku gdy nie ma pliku w katalogu domowym lub czasy sie nie zgadzaja
            {
                if (pobierz_rozmiar(nowa_sciezka) > Wielkosc_pliku)
                {
                    kopiuj_mapowanie(nowa_sciezka, podmien_folder1(nowa_sciezka, sciezka_folderu1, sciezka_folderu2));
                }
                else
                {
                    kopiuj(nowa_sciezka, podmien_folder1(nowa_sciezka, sciezka_folderu1, sciezka_folderu2));
                }
            }
        }
    }
    closedir(sciezka);
}
void Logowanie(int sig)
{
    syslog(LOG_INFO, "Wybudzenie demona przez sygnal SIGUSR1");
}