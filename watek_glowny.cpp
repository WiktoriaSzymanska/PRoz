#include "main.h"
#include "watek_glowny.h"


void mainLoopGnom(int gnomy, int skrzaty)
{
    srandom(rank);
    int tag;
    int perc;
    println("Jestem gnomem!");

    while (stan != InFinish) {

	switch (stan) {
	    // AGRAFKI
	    case InRun: {
		perc = random()%100;
		if ( perc < 25 ) {
		    debug("Perc: %d", perc);
		    println("GNOM: Staję w kolejce do agrafek")
		    debug("Zmieniam stan na wysyłanie");
		    packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
		    pkt->data = perc;

		    ackAgrCount = 0;
		    for (int i=0;i<=gnomy-1;i++) {  //iteracja przez wszystkie procesy gnomy
			if (i==rank){
				pthread_mutex_lock( &agrQueueMut);
				queue_agr.enqueue(i, LamportClock); //dodanie własnego żądania do kolejki
                                pthread_mutex_unlock( &agrQueueMut);
			} else {
				sendPacket( pkt, i, REQ_AGR); //wysłanie żądania do innych gnomów
			}
		    }

		    changeState( InWantAgr );
		    free(pkt);
		}
		debug("Skończyłem myśleć");
		break; }
	    case InWantAgr: {
		println("GNOM: Czekam na agrafkę");
		pthread_mutex_lock(&agrCountMut);
		if (ackAgrCount == gnomy - 1 && queue_agr.check_position(rank, agr_count) == TRUE) {
			changeState(InSectionAgr); }
		pthread_mutex_unlock(&agrCountMut);
		break; }
	    case InSectionAgr: {
		println("GNOM: Mogę wziąć agrafkę")
		sleep(5);

		debug("Zmieniam stan na wysyłanie");
		packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
		pkt->data = perc;
		//wysłanie RELEASE do pozostałych gnomów
		for (int i=0;i<=gnomy-1;i++)
			if (i!=rank)
				sendPacket( pkt, i, RELEASE_BIORE_AGR);
		// zmniejszenie liczby dostępnych agrafek
		pthread_mutex_lock( &agrCountMut);
		agr_count--;
                pthread_mutex_unlock( &agrCountMut);
		// usunięcie własnego żądania z kolejki
		pthread_mutex_lock(&agrQueueMut);
		queue_agr.dequeue(rank);
		pthread_mutex_unlock(&agrQueueMut);

		println("GNOM: Biorę AGRAFKĘ, zostało %d", agr_count);
		changeState( InRunCel );
		free(pkt);
		break; }

	    // CELOWNIKI
	    case InRunCel: {
                perc = random()%100;
                if ( perc < 25 ) {
                    debug("Perc: %d", perc);
                    println("GNOM: Staję w kolejce do celowników")
                    debug("Zmieniam stan na wysyłanie");
                    packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                    pkt->data = perc;

                    ackCelCount = 0;
                    for (int i=0;i<=gnomy-1;i++) {  //iteracja przez wszystkie  procesy gnomy
                        if (i==rank){
                                pthread_mutex_lock( &celQueueMut);
                                queue_cel.enqueue(i, LamportClock); //dodanie własnego żądania do kolejki
                                pthread_mutex_unlock( &celQueueMut);
                        } else {
                                sendPacket( pkt, i, REQ_CEL); //wysłanie żądania do innych gnomów
                        }
                    }

                    changeState( InWantCel );
                    free(pkt);
                }
                debug("Skończyłem myśleć");
                break; }
            case InWantCel: {
                println("GNOM: Czekam na celownik");
                pthread_mutex_lock(&celCountMut);
                if (ackCelCount == gnomy - 1 && queue_cel.check_position(rank, cel_count) == TRUE) {
                        changeState(InSectionCel); }
                pthread_mutex_unlock(&celCountMut);
                break; }
            case InSectionCel: {
                println("GNOM: Mogę wziąć celownik")
                sleep(5);

                debug("Zmieniam stan na wysyłanie");
                packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                pkt->data = perc;
                //wysłanie RELEASE do pozostałych gnomów
                for (int i=0;i<=gnomy-1;i++)
                        if (i!=rank)
                                sendPacket( pkt, i, RELEASE_BIORE_CEL);
                // zmniejszenie liczby dostępnych celowników
                pthread_mutex_lock( &celCountMut);
                cel_count--;
                pthread_mutex_unlock( &celCountMut);
                // usunięcie własnego żądania z kolejki
                pthread_mutex_lock(&celQueueMut);
                queue_cel.dequeue(rank);
                pthread_mutex_unlock(&celQueueMut);

                println("GNOM: Biorę CELOWNIK, zostało %d", cel_count);
                changeState( InAddBron );
                free(pkt);
                break; }
	    case InAddBron: {
		println("GNOM: Tworzę nową broń");
		sleep(5);

		debug("Zmieniam stan na wysyłanie");
                packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                pkt->data = perc;
                //wysłanie RELEASE do skrzatów
                for (int i=0; i<=size-1; i++)
                	sendPacket( pkt, i, RELEASE_DODAJE_BRON);
		println("GNOM: Dodałem nową broń");
		changeState( InRun );
		free(pkt);
		break; }

	    default: {
		break;}
            }
        sleep(SEC_IN_STATE);
    }
}


void mainLoopSkrzat(int gnomy, int skrzaty)
{
    srandom(rank);
    int tag;
    int perc;
    println("Jestem skrzatem!");

    while (stan != InFinish) {

        switch (stan) {
            case InRun: {
                perc = random()%100;
                if ( perc < 25 ) {
                    debug("Perc: %d", perc);
                    println("SKRZAT: Staję w kolejce po broń")
                    debug("Zmieniam stan na wysyłanie");
                    packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                    pkt->data = perc;

                    ackBronCount = 0;
                    for (int i=skrzaty; i<=skrzaty+gnomy-1;i++) {  //iteracja przez wszystkie  skrzaty
                        if (i==rank){
                                pthread_mutex_lock( &bronQueueMut);
                                queue_bron.enqueue(i, LamportClock); //dodanie własnego żądania do kolejki
                                pthread_mutex_unlock( &bronQueueMut);
                        } else {
                                sendPacket( pkt, i, REQ_BRON); //wysłanie żądania do innych skrzatów
                        }
                    }

                    changeState( InWantBron );
                    free(pkt);
                }
                debug("Skończyłem myśleć");
                break; }
            case InWantBron: {
                println("SKRZAT: Czekam na broń");
		// sprawdzenie warunku wejścia do sekcji krytycznej broni
                pthread_mutex_lock(&bronCountMut);
                if (ackBronCount == skrzaty - 1 && queue_bron.check_position(rank, bron_count) == TRUE) {
                        changeState(InSectionBron); }
                pthread_mutex_unlock(&bronCountMut);
                break; }
            case InSectionBron: {
                println("SKRZAT: Mogę wziąć broń")
                sleep(5);

                debug("Zmieniam stan na wysyłanie");
                packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                pkt->data = perc;
                //wysłanie RELEASE do pozostałych skrzatów
                for (int i=gnomy; i<=gnomy+skrzaty-1; i++)
                        if (i!=rank)
                                sendPacket( pkt, i, RELEASE_BIORE_BRON);
                // zmniejszenie liczby dostępnych broni
                pthread_mutex_lock( &bronCountMut);
                bron_count--;
                pthread_mutex_unlock( &bronCountMut);
                // usunięcie własnego żądania z kolejki
                pthread_mutex_lock(&bronQueueMut);
                queue_bron.dequeue(rank);
                pthread_mutex_unlock(&bronQueueMut);

                println("SKRZAT: Wziąłem BROŃ, zostało %d", bron_count);
                changeState( InReturnResources);
                free(pkt);
                break; }
	    case InReturnResources: {
		sleep(5);
		println("SKRZAT: Zabiłem szczura! Oddaje agrafkę i celownik");
		debug("Zmieniam stan na wysyłanie");
                packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
                pkt->data = perc;
                //wysłanie RELEASE AGR i CEL do gnomów
                for (int i=0; i<=gnomy-1; i++) {
                	sendPacket( pkt, i, RELEASE_ODDAJE_AGR_CEL);
		}
		changeState( InRun );
                free(pkt);
		break; }

            default: {
                break;}
            }
        sleep(SEC_IN_STATE);
    }
}

