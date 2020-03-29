// do testowania synchronizacji
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* Return a string that describes the type of the file system entry PATH. */
const char *get_file_type(const char *path)
{
    struct stat st;
    if (lstat(path, &st) == 0)
    {
        if (S_ISLNK(st.st_mode))
            return "symbolic link";
        else if (S_ISDIR(st.st_mode))
            return "directory";
        else if (S_ISCHR(st.st_mode))
            return "character device";
        else if (S_ISBLK(st.st_mode))
            return "block device";
        else if (S_ISFIFO(st.st_mode))
            return "fifo";
        else if (S_ISSOCK(st.st_mode))
            return "socket";
        else if (S_ISREG(st.st_mode))
            return "regular file";
        else
            /* Unexpected. Each entry should be one of the types above. */
            assert(0);
    }
    else
    {
        printf("You specified wrong file type\n");
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char *argv[])
{
    char *dir_path1;
    char *dir_path2;
    DIR *dir1;
    DIR *dir2;
    struct dirent *entry1;
    struct dirent *entry2;
    char entry_path1[PATH_MAX + 1];
    char entry_path2[PATH_MAX + 1];
    size_t path_len1;
    size_t path_len2;

    // czy podane dwa argumenty
    if (argc < 3)
    {
        printf("You have't specified two paths\n");
        exit(EXIT_FAILURE);
    }

    dir_path1 = argv[1];
    dir_path2 = argv[2];

    // czy sciezki sa poprawne
    if (get_file_type(dir_path1) != "directory" || get_file_type(dir_path2) != "directory")
    {
        printf("You specified incorrect directories");
        exit(EXIT_FAILURE);
    }

    /* Copy the directory path into entry_path. */
    // tu sie potem funkcje najwyzej napisze na razie byle dzialalo
    strncpy(entry_path1, dir_path1, sizeof(entry_path1));
    path_len1 = strlen(dir_path1);
    /* If the directory path doesn't end with a slash, append a slash. */
    if (entry_path1[path_len1 - 1] != '/')
    {
        entry_path1[path_len1] = '/';
        entry_path1[path_len1 + 1] = '\0';
        ++path_len1;
    }
    strncpy(entry_path2, dir_path2, sizeof(entry_path2));
    path_len2 = strlen(dir_path2);
    if (entry_path2[path_len2 - 1] != '/')
    {
        entry_path2[path_len2] = '/';
        entry_path2[path_len2 + 1] = '\0';
        ++path_len2;
    }

    /* Start the listing operation of the directory specified on the
    command line. */
    dir1 = opendir(dir_path1);
    /* Loop over all directory entries. */
    /*
        Tutaj mamy taki problem, ze wypisuje te pliki ze sciezki w dowolnej kolejnosci,
        wiec proponuje z sciezek wstawic nazwy do tablicy posortowac i potem przechodzic po tych
        tablicach i patrzec czy sie zgadza(jak masz inny pomysl to sprobuj)
    */
    char files_path1[100][PATH_MAX + 1];
    int files_count_path1 = 0;
    while ((entry1 = readdir(dir1)) != NULL)
    {
        const char *type;
        /* Build the path to the directory entry by appending the entry
        name to the path name. */
        strncpy(entry_path1 + path_len1, entry1->d_name,
                sizeof(entry_path1) - path_len1);
        /* Determine the type of the entry. */
        type = get_file_type(entry_path1);
        /* Print the type and path of the entry. */
        printf("%-18s: %s\n", type, entry_path1);

        // tylko pliki trzeba porownywac (w 2 podpunkcie tez katalogi ale to pozniej)
        if (type == "regular file")
        {
            // tylko pliki do tablicy wrzucamy bo rzeszty nie musimy sprawdzac
            // popierdolone wiem ale normlane wstawienie do tablicy takie files_path1[files_count_path1] = entry_path1 nie dziala
            strncpy(files_path1[files_count_path1], entry_path1, sizeof(files_path1[files_count_path1]));
            files_count_path1++;
            // w tym struct bedzie cale info o pliku tu masz link do tego
            // https://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html#Attribute-Meanings
            struct stat st;
            lstat(entry_path1, &st);

            time_t last_modification;
            last_modification = st.st_mtime;
            printf("Last modification time of this file is: %s", asctime(gmtime(&last_modification)));

            // porownanie czasow
            // funkcja do tego to difftime(pierwszy_czas, drugi_czas)
            // 1. jak 0 to zajebiscie czilerka
            // 2. jak w docelowym pozniej to trzeba skopiowac ze zrodlowego zawartosc i zmienic czas na ze zrodlowego
            // 3. jak w zrodlowym pozniej to nie napisane xD ale pewnie tez zmienic w docelowym na to samo

            // jak jakis plik tylko w jednym katalogu jest
            // jak tylko plik w zrodlowm to kopia do docelowego
            // jak tylko plik w docelowym to wyjebac
        }
    }

    dir2 = opendir(dir_path2);
    char files_path2[100][PATH_MAX + 1];
    int files_count_path2 = 0;
    while ((entry2 = readdir(dir2)) != NULL)
    {
        const char *type;
        /* Build the path to the directory entry by appending the entry
        name to the path name. */
        strncpy(entry_path2 + path_len2, entry2->d_name,
                sizeof(entry_path2) - path_len2);
        /* Determine the type of the entry. */
        type = get_file_type(entry_path2);
        /* Print the type and path of the entry. */
        printf("%-18s: %s\n", type, entry_path2);

        // tylko pliki trzeba porownywac (w 2 podpunkcie tez katalogi ale to pozniej)
        if (type == "regular file")
        {
            // tylko pliki do tablicy wrzucamy bo rzeszty nie musimy sprawdzac
            // popierdolone wiem ale normlane wstawienie do tablicy takie files_path1[files_count_path1] = entry_path1 nie dziala
            strncpy(files_path2[files_count_path2], entry_path2, sizeof(files_path2[files_count_path2]));
            files_count_path2++;
            // w tym struct bedzie cale info o pliku tu masz link do tego
            // https://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html#Attribute-Meanings
            struct stat st;
            lstat(entry_path2, &st);

            time_t last_modification;
            last_modification = st.st_mtime;
            printf("Last modification time of this file is: %s", asctime(gmtime(&last_modification)));

            // porownanie czasow
            // funkcja do tego to difftime(pierwszy_czas, drugi_czas)
            // 1. jak 0 to zajebiscie czilerka
            // 2. jak w docelowym pozniej to trzeba skopiowac ze zrodlowego zawartosc i zmienic czas na ze zrodlowego
            // 3. jak w zrodlowym pozniej to nie napisane xD ale pewnie tez zmienic w docelowym na to samo

            // jak jakis plik tylko w jednym katalogu jest
            // jak tylko plik w zrodlowm to kopia do docelowego
            // jak tylko plik w docelowym to wyjebac
        }
    }

    // tutaj tylko sprawdzenie czy dobrze sie dodalo do tablicy
    printf("\n\n\n\n");
    int i = 0;
    printf("Pliki w pierwszej sciezce:\n");
    for (i = 0; i < files_count_path1; i++)
    {
        printf("%s\n", files_path1[i]);
    }
    printf("Pliki w drugiej sciezce:\n");
    for (i = 0; i < files_count_path2; i++)
    {
        printf("%s\n", files_path2[i]);
    }

    /* All done. */
    closedir(dir1);
    closedir(dir2);
    return 0;
}