
Kompilacja: make

Uruchomienie: mpirun -oversubscribe -np \[liczba proc\] main \[gnomy\] \[skrzaty\] \[agrafki\] \[celowniki]


##### TEMAT PROJEKTU: Skrzaty zabójcy szczurów
Są dwa rodzaje procesów - S skrzatów i G gnomów. 
Gnomy ubiegają się o jedną z A agrafek i C celowników. 
Kombinują agrafki z celownikiem tworząc broń. Skrzaty ubiegają się o broń. 
Po zdobyciu broni zabijają szczury i zwracają agrafki i celowniki do puli.


###### ALGORYTM (na podstawie alg. Lamporta)

(W1) Warunek wejścia do sekcji krytycznej:
Jeżeli żądanie procesu jest na n-tym lub wcześniejszym miejscu w kolejce do danego zasobu, 
gdzie n to liczba dostępnych zasobów, i proces otrzymał od wszystkich pozostałych procesów 
wiadomość (związaną z tym zasobem) o starszej etykiecie czasowej, to proces ma dostęp do zasobu.

Gnom:
1. Gnom ubiega się o dostęp do agrafek. Wysyła do innych gnomów żądanie REQ_AGR. 
Dodaje swoje żądania do lokalnej kolejki queue_agr.
*(watek_glowny)*

2. Otrzymuje wiadomości od innych gnomów i skrzatów. Jeżeli otrzyma wiadomość:
- REQ_AGR, to dodaje gnoma do kolejki do agrafek queue_agr i wysyła potwierdzenie ACK_AGR.
- RELEASE_BIORE_AGR od gnoma, to usuwa jego żądanie z kolejki i zmniejsza liczbę agrafek o 1.
- RELEASE_ODDAJE_AGR od skrzata, to zwiększa liczbę agrafek o 1.
*(watek_komunikacyjny)*

3. Jeżeli liczba agrafek > 0, to gnom sprawdza, czy spełnia warunek (W1). 
Jeżeli tak, to może wziąć agrafkę. Wysyła do pozostałych gnomów RELEASE_BIORE_AGR 
i zmniejsza liczbę agrafek o 1.
*(watek_glowny)*

4. Jeżeli gnom ma agrafkę, ubiega się o celownik w sposób analogiczny do agrafek 
(REQ_CEL, RELEASE_ODDAJE_CEL, itd.)

5. Jeżeli gnom ma agrafkę i celownik, to robi z nich broń. 
Gnom wysyła do skrzatów wiadomość RELEASE_DODAJE_BRON.

Skrzat:
1. Wysyła do innych skrzatów żądanie o dostęp do broni (REQ_BRON). 
Dodaje swoje żądanie do lokalnej kolejki do broni (queue_bron).
*(watek_glowny)*

2. Otrzymuje wiadomości od innych skrzatów i gnomów. Jeżeli otrzyma wiadomość:
- REQ_BRON, to dodaje skrzata do kolejki queue_bron i wysyła ACK_BRON.
- RELEASE_BIORE_BRON od skrzata, to usuwa jego żądanie z kolejki i zmniejsza liczbę broni o 1.
- RELEASE_DODAJE_BRON od gnoma, to zwiększa liczbę broni o 1.
*(watek_komunikacyjny)*

3. Jeżeli liczba broni > 0, to skrzat sprawdza, czy spełnia warunek (W1). 
Jeżeli tak, to może wziąć broń i wysyła do pozostałych skrzatów RELEASE_BIORE_BRON 
i zmniejsza liczbę broni o 1.
*(watek_glowny)*

4. Jeżeli skrzat ma broń, to zabija szczura i zwraca agrafkę i celownik. 
Wysyła RELEASE_ODDAJE_AGR i RELEASE_ODDAJE_CEL do gnomów.
*(watek_glowny)*
