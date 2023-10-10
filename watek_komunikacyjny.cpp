#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatekGnom(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Aktualizacja zegara Lamporta
        pthread_mutex_lock( &clockMut );
        if (pakiet.ts > LamportClock) LamportClock = pakiet.ts + 1;
        else LamportClock = LamportClock + 1;
        pthread_mutex_unlock( &clockMut );

        switch ( status.MPI_TAG ) {
	    // AGRAFKI - komunikaty odbierane od innych gnomów
	    case REQ_AGR:
		// dodanie procesu do kolejki queue_agr
		pthread_mutex_lock( &agrQueueMut);
		queue_agr.enqueue(status.MPI_SOURCE, pakiet.ts);
		pthread_mutex_unlock(&agrQueueMut);
		// odesłanie potwierdzenia
		sendPacket( 0, status.MPI_SOURCE, ACK_AGR );
	    break;
	    case ACK_AGR: 
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackAgrCount);
	        ackAgrCount++;
	    break;
	    case RELEASE_BIORE_AGR:
		// usunięcie żądania z kolejki
		pthread_mutex_lock( &agrQueueMut);
		queue_agr.dequeue(status.MPI_SOURCE);
                pthread_mutex_unlock(&agrQueueMut);
		// zmniejszenie liczby agrafek
                pthread_mutex_lock( &agrCountMut);
		agr_count--;
                pthread_mutex_unlock( &agrCountMut);
	    break;

	    // CELOWNIKI - komunikaty odbierane od innych gnomów
	    case REQ_CEL:
                // dodanie procesu do kolejki queue_cel
                pthread_mutex_lock( &celQueueMut);
                queue_cel.enqueue(status.MPI_SOURCE, pakiet.ts);
                pthread_mutex_unlock(&celQueueMut);
                // odesłanie potwierdzenia
                sendPacket( 0, status.MPI_SOURCE, ACK_CEL );
            break;
            case ACK_CEL:
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCelCount);
                ackCelCount++;
            break;
            case RELEASE_BIORE_CEL:
                // usunięcie żądania z kolejki
                pthread_mutex_lock( &celQueueMut);
                queue_cel.dequeue(status.MPI_SOURCE);
                pthread_mutex_unlock(&celQueueMut);
                // zmniejszenie liczby celowników
                pthread_mutex_lock( &celCountMut);
                cel_count--;
                pthread_mutex_unlock( &celCountMut);
            break;

	    // BRON - skrzat oddaje agrafkę i celownik
            case RELEASE_ODDAJE_AGR_CEL:
                // zwiększenie liczby agrafek
		pthread_mutex_lock( &agrCountMut);
                agr_count++;
                pthread_mutex_unlock( &agrCountMut);
                // zwiększenie liczby celowników
                pthread_mutex_lock( &celCountMut);
                cel_count++;
                pthread_mutex_unlock( &celCountMut);
            break;

	    default:
	    break;
        }
    }
}


void *startKomWatekSkrzat(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
        debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Aktualizacja zegara Lamporta
        pthread_mutex_lock( &clockMut );
        if (pakiet.ts > LamportClock) LamportClock = pakiet.ts + 1;
        else LamportClock = LamportClock + 1;
        pthread_mutex_unlock( &clockMut );

        switch ( status.MPI_TAG ) {
	    // BRONIE
            case REQ_BRON:
                // dodanie procesu do kolejki queue_bron
                pthread_mutex_lock( &bronQueueMut);
                queue_bron.enqueue(status.MPI_SOURCE, pakiet.ts);
                pthread_mutex_unlock(&bronQueueMut);
                // odesłanie potwierdzenia
                sendPacket( 0, status.MPI_SOURCE, ACK_BRON );
            break;
            case ACK_BRON:
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackBronCount);
                ackBronCount++;
            break;
            case RELEASE_BIORE_BRON:
                // usunięcie żądania z kolejki
                pthread_mutex_lock( &bronQueueMut);
                queue_bron.dequeue(status.MPI_SOURCE);
                pthread_mutex_unlock(&bronQueueMut);
                // zmniejszenie liczby broni
                pthread_mutex_lock( &bronCountMut);
                bron_count--;
                pthread_mutex_unlock( &bronCountMut);
            break;
            case RELEASE_DODAJE_BRON:
                // zwiększenie liczby broni
                pthread_mutex_lock( &bronCountMut);
                bron_count++;
                pthread_mutex_unlock( &bronCountMut);
            break;

            default:
	    break;
	}
    }
}
