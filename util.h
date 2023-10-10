#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK_AGR 1
#define ACK_CEL 2
#define ACK_BRON 3
#define REQ_AGR 4
#define REQ_CEL 5
#define REQ_BRON 6
#define RELEASE_BIORE_AGR 7
#define RELEASE_BIORE_CEL 8
#define RELEASE_ODDAJE_AGR_CEL 9
#define RELEASE_BIORE_BRON 10
#define RELEASE_DODAJE_BRON 11
#define APP_PKT 12
#define FINISH 13

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InMonitor, InWantAgr, InSectionAgr, InRunCel, InWantCel, InSectionCel, InAddBron, InWantBron, InSectionBron, InReturnResources, InFinish} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeState( state_t );

// KOLEJKI DO ZASOBÓW
class ResourceQueue {
public:
    ResourceQueue();

    void display_queue();
    void enqueue(int process_id, int lamport_clock);
    void dequeue(int process_id);
    bool check_position(int process_id, int n);

private:
    std::vector<std::pair<int, int>> queue;
};

#endif
