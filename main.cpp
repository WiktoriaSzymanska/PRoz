/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"

/*
 * W main.h extern int rank (zapowiedź) w main.c int rank (definicja)
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami. Rank i size akurat są write-once, więc nie trzeba,
 * ale zob util.c oraz util.h - zmienną state_t state i funkcję changeState
 *
 */
int rank, size;
int LamportClock;

int ackAgrCount = 0;
int ackCelCount = 0;
int ackBronCount = 0;
int agr_count;  // aktualna liczba agrafek
int cel_count;  // aktualna liczba celowników
int bron_count; // aktualna liczba broni
ResourceQueue queue_agr;  // kolejka do agrafek
ResourceQueue queue_cel;  // kolejka do celowników
ResourceQueue queue_bron; // kolejka do broni

/*
 * Każdy proces ma dwa wątki - główny i komunikacyjny
 * w plikach, odpowiednio, watek_glowny.c oraz (siurpryza) watek_komunikacyjny.c
 */

pthread_t threadKom;

void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    pthread_mutex_destroy( &clockMut);
    pthread_mutex_destroy( &celCountMut);
    pthread_mutex_destroy( &celQueueMut);
    pthread_mutex_destroy( &agrCountMut);
    pthread_mutex_destroy( &agrQueueMut);
    pthread_mutex_destroy( &bronCountMut);
    pthread_mutex_destroy( &bronQueueMut);

    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}


int main(int argc, char **argv)
{
    if (argc != 5) {
        println("Użycie: %s <liczba_gnomow> <liczba_skrzatow> <liczba_agrafek> <liczba_celownikow>", argv[0]);
        finalizuj();
        return 1;
    }

    int gnomy = atoi(argv[1]);
    int skrzaty = atoi(argv[2]);
    int agrafki = atoi(argv[3]);
    int celowniki = atoi(argv[4]);

    agr_count = agrafki;
    cel_count = celowniki;
    bron_count = 0;

    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    srand(rank);
    /* zob. util.c oraz util.h */
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Sprawdzenie, czy liczba procesów jest równa liczbie gnomów i skrzatów
    if (gnomy + skrzaty != size) {
        println("Liczba gnomów i skrzatów musi być równa liczbie uruchomionych procesów...");
        finalizuj();
        return 1;
    }

    if (rank < gnomy) {
	pthread_create (&threadKom, NULL, startKomWatekGnom , 0);
    	mainLoopGnom(gnomy, skrzaty);
    } else {
	pthread_create(&threadKom, NULL, startKomWatekSkrzat, 0);
	mainLoopSkrzat(gnomy, skrzaty);
                // możesz także wcisnąć ctrl-] na nazwie funkcji
                // działa, bo używamy ctags (zob Makefile)
                // jak nie działa, wpisz set tags=./tags :)
    }

    finalizuj();
    return 0;
}

