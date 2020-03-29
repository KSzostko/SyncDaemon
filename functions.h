#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <syslog.h>
off_t pobierz_rozmiar(char *in);
time_t pobierz_czas(char *wej);
mode_t pobierz_chmod(char *wej);
void zmien_parametry(char *wej, char *wyj);
char *podmien_folder2(char *sciezka1, char *sciezka_folderu1, char *sciezka_folderu2);
char *podmien_folder1(char *sciezka1, char *sciezka_folderu1, char *sciezka_folderu2);
char *dodaj_do_sciezki(char *sciezka, char *dodatek);
bool sprawdzanie(char *nazwa_sciezki, char *sciezka_folderu1, char *sciezka_folderu2);
void Usuwanie(char *nazwa_sciezki_folder2, char *sciezka_folderu1, char *sciezka_folderu2, bool CzyR);
void kopiuj(char *wej, char *wyj);
void kopiuj_mapowanie(char *wej, char *wyj);
void PrzegladanieFolderu(char *nazwa_sciezki1, char *sciezka_folderu1, char *sciezka_folderu2, bool CzyR, int Wielkosc_pliku);
void Logowanie();

#endif // FUNCTIONS_H_INCLUDED
