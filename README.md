344C1

PROIECT SM - SHARPEN FILTER

PETRUC RARES

Am realizat urmatoarele tipuri de implementari ale filtrului:
  
- serial
- omp
- pthreads
- mpi
- mpi_omp
- mpi_pthreads

*** TODO  PUNE HEADER ***

Idee tema, pune sursa de unde te-ai inspirat cu implementarea (aia cu trandafiru').
Precizeaza ca le-ai folosit ideea de wrap-around la calcul la filtru.

Pentru testarea si masurarea finala, ca sa avem rezultate cat de cat relevante am folosit o poza mare, rezolutie 12000x6000, pe care am gasit-o conform Notei 1 din sectiunea Note a acestui README.

*** TODO  PUNE HEADER ***

Makefile pentru compilare/rulare/stergere:

Pentru fiecare executabil pune un caz generic de rulare si precizeaza ca rularile
sunt disponibile si in rezultate_rulari.txt

*** TODO  PUNE HEADER ***
Concluzii:

***TODO fa-le frumos sub forma de lista***
In general:
    timp la omp mai bun decat la pthreads.
    timp la mpi mai slab decat omp/pthreads


Note:
1) Go to images.google.com and enter the search terms as before. Then append imagesize:WIDTHxHEIGHT to your query and hit Enter.

2) Testele au fost realizate pe poze cu dimensiune latime x inaltime, unde latime este multiplu de 4, intrucat padding-ul necesar in cazul latimii daca nu este multiplu de 4 nu este neaparat 0 si ar necesita adaptare de la imagine la imagine (sursa: https://stackoverflow.com/questions/2601365/padding-in-24-bits-rgb-bitmap)

3) Nu am verificat mereu alocarea sau deschiderea/inchiderea fisierelor, intrucat nu acesta era scopul temei.

4) Pentru masurarea timpului serial am folosit:
https://www.geeksforgeeks.org/how-to-measure-time-taken-by-a-program-in-c/