#include "funkcje.h"
off_t pobierz_rozmiar(char *in)
{
	struct stat rozmiar;
	if (stat(in, &rozmiar) == 0)
	{
		return rozmiar.st_size;
	}
	return -1;
}
time_t pobierz_czas(char* wej) //zwraca date modyfikacji pliku
{
	struct stat czas;
	if (stat(wej, &czas) == -1)
	{
		syslog(LOG_ERR, "Blad z pobraniem daty modyfikacji dla pliku %s", wej);
		exit(EXIT_FAILURE);
	}
	return czas.st_mtime;
}
mode_t pobierz_chmod(char *wej)
{
	struct stat mod;
	if (stat(wej, &mod) == -1)
	{
		syslog(LOG_ERR, "Blad pobrania chmod dla pliku %s", wej);
		exit(EXIT_FAILURE);
	}
	return mod.st_mode;
}

void zmien_parametry(char* wej, char *wyj) // parametrami sa jeden i drugi plik z katalogu zrodlowego i docelowego
// do konca nie wiem jak to ma dzialac, skoro oni zadeklarowali jakas strukture, pobrali czas modyfikacji, obsluzyli bledy i co dalej? xD
{
	struct utimbuf czas; //jakas specjalna struktura do zmiany czasu dostepu i modyfikacji
	czas.actime = 0;
	czas.modtime = pobierz_czas(wej); //pobieramy czas modyfikacji z pliku zrodlowego
	if (utime(wyj, &czas) != 0)
	{
		syslog(LOG_ERR, "Blad zwiazany z data modyfikacji!");
		exit(EXIT_FAILURE);
	}
	mode_t stary = pobierz_chmod(wej);
	if (chmod(wyj, stary) != 0)
	{
		syslog(LOG_ERR, "Blad ustawienia uprawnien do pliku!");
		exit(EXIT_FAILURE);
	}
}

char *podmien_folder2(char * sciezka1, char* sciezka_folderu1, char* sciezka_folderu2) //do usuniecia
{
	char*sciezka = sciezka1 + strlen(sciezka_folderu2);
	char * nowa_sciezka = malloc(strlen(sciezka_folderu1) + strlen(sciezka) + 1);
	strcpy(nowa_sciezka, sciezka_folderu1);
	strcat(nowa_sciezka, sciezka);
	return nowa_sciezka;
}
char *podmien_folder1(char * sciezka1, char* sciezka_folderu1, char* sciezka_folderu2) //1. pełna ścieżka do pliku np.home / zrodlo / plik, 2. katalog zrodlowy, 3. katalog docelowy
{
	char*sciezka = sciezka1 + strlen(sciezka_folderu1);  //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
	char * nowa_sciezka = malloc(strlen(sciezka_folderu2) + strlen(sciezka) + 1); //alokujemy pamiec na nowa sciezke
	strcpy(nowa_sciezka, sciezka_folderu2); //zamiana katalogu ze zrodlowego na docelowy, czyli doklejamy do pustej zmiennej nazwe katalogu docelowego
	strcat(nowa_sciezka, sciezka); //dodanie nazwy pliku
	return nowa_sciezka;
}
char *dodaj_do_sciezki(char* sciezka, char *dodatek) //1. ścieżka źródłowa, 2. nazwa pliku
{
	char * nowa_sciezka = malloc(strlen(sciezka) + 2 + strlen(dodatek)); //alokujemy pamięć na nową nazwę ścieżki
	strcpy(nowa_sciezka, sciezka); //dodajemy początek ścieżki np. /home/zrodlo
	strcat(nowa_sciezka, "/"); // /home/zrodlo/
	strcat(nowa_sciezka, dodatek); // /home/zrodlo/plik1
	nowa_sciezka[strlen(sciezka) + 1 + strlen(dodatek)] = '\0';
	return nowa_sciezka; // zwracamy ścieżkę do pliku w katalogu
}
bool sprawdzanie(char * nazwa_sciezki, char* sciezka_folderu1, char* sciezka_folderu2) //1. pełna ścieżka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy
{
	bool wynik = 0;
	char *nazwa_sciezki_zm = nazwa_sciezki + strlen(sciezka_folderu1); //ucinamy poczatek katalogu i zostawiamy sama nazwe pliku
	char *szukamy = malloc(strlen(nazwa_sciezki_zm)); //alokacja pamieci
	char * nowa_sciezka = podmien_folder1(nazwa_sciezki, sciezka_folderu1, sciezka_folderu2);  //otrzymujemy zamieniony folder zrodlowy z docelowym

	int i = strlen(nowa_sciezka);
	for (i; nowa_sciezka[i] != '/'; i--);
	strcpy(szukamy, nowa_sciezka + i + 1); //pod "szukamy" wpisywana jest nazwa pliku, nowa sciezka zostaje sama sciekza docelowa
	nowa_sciezka[i] = '\0';
	struct dirent * plik;
	DIR * sciezka;
	sciezka = opendir(nowa_sciezka); //otwieramy strumien do sciezki docelowej

	while ((plik = readdir(sciezka)))
	{
		if (strcmp(plik->d_name, szukamy) == 0) //jesli nazwa pliku z katalogu i szukanego jest taka sama to
		{
			free(szukamy);
			if ((plik->d_type) == DT_DIR)  //    GDY JEST FOLDEREM
			{
				return 0;
			}
			else
			{
				int czas1 = (int)pobierz_czas(nazwa_sciezki), czas2 = (int)pobierz_czas(dodaj_do_sciezki(nowa_sciezka, plik->d_name)); //pobieramy czas modyfikacji pliku ze zrodlowego i z docelowego
				if (czas1 == czas2) //jesli czasy takie same to return 0
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
			wynik = 1; //jezeli nie ma takiego pliku w docelowym to trzeba przekopiowac
		}
	}
	closedir(sciezka);
	return wynik;
}


void Usuwanie(char * nazwa_sciezki_folder2, char* sciezka_folderu1, char* sciezka_folderu2, bool CzyR)
//1. folder docelowy, 2. folder zrodlowy, 3, folder docelowy, 4. rekurencja
{
	struct dirent * plik; //struct dirent - struktura wskazująca na element w katalogu (plik/folder), zawiera nazwę pliku
	DIR * sciezka, *pom; //funkcja otwierająca strumień do katalogu
	sciezka = opendir(nazwa_sciezki_folder2); //przechodzimy do folderu docelowego
	while ((plik = readdir(sciezka)))
	{
		if ((plik->d_type) == DT_DIR)  //    GDY JEST FOLDEREM
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
			char *nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki_folder2, plik->d_name); // tworzymy nową ścieżkę dodając do folderu docelowego nazwę pliku, który jest przetwarzany np /home/docelowy/plik
			if (access(podmien_folder2(nowa_sciezka, sciezka_folderu1, sciezka_folderu2), F_OK) == -1) //sprawdzamy czy mozna usunac plik (wedlug mnie to jest zle -1 jest wtedy, kiedy wystepuje blad, powinno byc == 0)
			{
				syslog(LOG_INFO, "Usunieto plik %s", nowa_sciezka);
				remove(nowa_sciezka);
			}
		}
	}
	closedir(sciezka);
}

void kopiuj(char *wej, char *wyj)
//tutaj raczej wszystko jasne, podobny kod jak na PS1, z tym, że czytanie do drugiego pliku, a nie na ekran
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
	int rozmiar = pobierz_rozmiar(wej); //pobieramy rozmiar wejscia w bitach
	int plikwej = open(wej, O_RDONLY);
	int plikwyj = open(wyj, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	if (plikwej == -1 || plikwyj == -1)
	{
		syslog(LOG_ERR, "Blad w otwarciu pliku!");
		exit(EXIT_FAILURE);
	}

	char *mapa = (char*)mmap(0, rozmiar, PROT_READ, MAP_SHARED | MAP_FILE, plikwej, 0); //tutaj cos z neta przeklejone 1:1 https://stackoverflow.com/questions/31097935/copy-whole-of-a-file-into-memory-using-mmap

	write(plikwyj, mapa, rozmiar); // zapis do pliku

	close(plikwej);
	close(plikwyj);
	munmap(mapa, rozmiar); //usuwanie mapy z pamieci;
	zmien_parametry(wej, wyj);
	syslog(LOG_INFO, "Z uzyciem mapowania skopiowano plik %s do miejsca %s", wej, wyj);
}

void PrzegladanieFolderu(char * nazwa_sciezki1, char* sciezka_folderu1, char* sciezka_folderu2, bool CzyR, int Wielkosc_pliku)
//1. folder źródłowy, 2. folder źródłowy, 3. folder docelowy, 4. rekurencja, 5. wielkość pliku
{
	printf("JESTESMY W : %s\n", nazwa_sciezki1);
	struct dirent * plik; //struct dirent - struktura wskazująca na element w katalogu (plik/folder), zawiera nazwę pliku
	DIR * sciezka, *pom; //funkcja otwierająca strumień do katalogu
	sciezka = opendir(nazwa_sciezki1); //przechodzimy do folderu źródłowego
	char* nowa_sciezka;
	while ((plik = readdir(sciezka)))
	{
		printf("%s  \n", plik->d_name);
		if ((plik->d_type) == DT_DIR)  //GDY JEST FOLDEREM
		{
			if (CzyR)
			{
				if (!(strcmp(plik->d_name, ".") == 0 || strcmp(plik->d_name, "..") == 0))
				{
					char * sciekza_do_folderu = podmien_folder1(dodaj_do_sciezki(nazwa_sciezki1, plik->d_name), sciezka_folderu1, sciezka_folderu2);
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
		else  if ((plik->d_type) == DT_REG)// GDY nie jest folderem, DT_REG to plik regularny
		{
			nowa_sciezka = dodaj_do_sciezki(nazwa_sciezki1, plik->d_name); // tworzymy nową ścieżkę dodając do folderu źródłowego nazwę pliku, który jest przetwarzany np /home/projekt/plik
			int i;
			if ((i = sprawdzanie(nowa_sciezka, sciezka_folderu1, sciezka_folderu2)) == 1) //1. pełna ścieżka do pliku np. home/zrodlo/plik, 2. katalog zrodlowy, 3. katalog docelowy, zwraca nam 1 w przypadku gdy nie ma pliku w katalogu domowym lub czasy sie nie zgadzaja
			{
				if (pobierz_rozmiar(nowa_sciezka) > Wielkosc_pliku)
				{
					kopiuj_mapowanie(nowa_sciezka, podmien_folder1(nowa_sciezka, sciezka_folderu1, sciezka_folderu2));
				}
				else
				{
					kopiuj(nowa_sciezka, podmien_folder1(nowa_sciezka, sciezka_folderu1, sciezka_folderu2)); //kopiuj ze zrodlowego do docelowego (przekazujemy np. (/home/zrodlowy/plik, /home/docelowy/plik))
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