// do testowania synchronizacji
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
    lstat(path, &st);
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
        printf("You have't specified two paths");
        exit(EXIT_FAILURE);
    }

    dir_path1 = argv[1];
    dir_path2 = argv[2];

    // czy sciezki sa poprawne
    if (get_file_type(dir_path1) != = "directory" || get_file_type(dir_path2) != = "directory")
    {
        printf("You specified correct directories");
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
    }

    /* All done. */
    closedir(dir1);
    return 0;
}