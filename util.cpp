#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

/* 
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan=InRun;

/* zamek wokół zmiennej współdzielonej między wątkami. 
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clockMut = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t agrCountMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t agrQueueMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t celCountMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t celQueueMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bronCountMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bronQueueMut = PTHREAD_MUTEX_INITIALIZER;



struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { { "pakiet aplikacyjny", APP_PKT }, { "finish", FINISH}, 
                { "potwierdzenie (agrafki)", ACK_AGR}, {"potwierdzenie (celowniki)", ACK_CEL},
		{"potwierdzenie (bronie)", ACK_BRON}, {"prośba o agrafki", REQ_AGR},
		{"prośba o celownik", REQ_CEL}, {"prośba o broń", REQ_BRON},
		{"zwolnienie dostępu do agrafek przez gnoma", RELEASE_BIORE_AGR}, 
		{"zwolnienie dostępu do celowników przez gnoma", RELEASE_BIORE_CEL},
		{"dodanie agrafki i celownika przez skrzata", RELEASE_ODDAJE_AGR_CEL},
		{"dodanie broni przez gnoma", RELEASE_DODAJE_BRON},
		{"zwolnienie dostępu do broni przez skrzata", RELEASE_BIORE_BRON} };

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = (packet_t *) malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    pthread_mutex_lock( &clockMut );
    LamportClock++;
    pthread_mutex_unlock( &clockMut );
    pkt->ts = LamportClock;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}


// KOLEJKI DO ZASOBÓW
ResourceQueue::ResourceQueue() {}

void ResourceQueue::display_queue() {
        printf("Kolejka %d: ", rank);
        for (const auto& request : queue) {
            printf("(%d,%d) ", request.first, request.second);
        }
        printf("\n");
}

// Dodawanie żądania na odpowiednie miejsce w kolejce (sortowanie)
void ResourceQueue::enqueue(int process_id, int lamport_clock) {
        queue.emplace_back(process_id, lamport_clock);
        std::sort(queue.begin(), queue.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second < b.second; // Sortuj wg zegara Lamporta
        });

	//TODO usunąć
        printf("+ Kolejka %d %d: ", rank, LamportClock);
        for (const auto& request : queue) {
            printf("(%d,%d) ", request.first, request.second);
        }
        printf("\n");
}

// Usunięcie żądania z kolejki
void ResourceQueue::dequeue(int process_id) {
        queue.erase(std::remove_if(queue.begin(), queue.end(), [process_id](const std::pair<int, int>& request) {
            return request.first == process_id;
        }), queue.end());

	//TODO usunąć
        printf("- Kolejka %d %d: ", rank, LamportClock);
        for (const auto& request : queue) {
            printf("(%d,%d) ", request.first, request.second);
        }
        printf("\n");
}

// Sprawdzenie, czy żądanie procesu jest na n-tym lub wcześniejszym miejscu w kolejce
bool ResourceQueue::check_position(int process_id, int n) {
	for (int i = 0; i < n && i < queue.size(); i++) {
            if (queue[i].first == process_id) {
                // Znaleziono żądanie
                return true;
            }
        }
        // Nie znaleziono żądania lub jest na pozycji > n
        return false;
}
