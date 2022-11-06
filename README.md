<h1> 344C1
PROIECT SM - SHARPEN FILTER
- PETRUC RARES
</h1>

Am realizat urmatoarele tipuri de implementari ale filtrului:
  
- seriala
- OpenMP
- pthreads
- MPI
- MPI_OpenMP
- MPI_pthreads

<h2> Despre Tema </h2>

Am vazut o implementare seriala de unde m-am inspirat la [1] (sectiunea *Referinte*).
O idee importanta pe care am preluat-o tot de aici a fost modul in care consideram pixelii vecini ai pixelilor din margine (randul 0, coloana 0, randul h-1, coloana w-1, unde h si w reprezinta inaltime, respectiv latimea matricei). Am folosit metoda ***wrap around***, astfel ca pixeli ca cei cu coordonatele (-1, -1) ajung sa fie (w-1, h-1). 


Pentru testarea si masurarea finala, ca sa avem rezultate cat de cat relevante din punct de vedere al ***speedup***-ului am folosit o poza mare, rezolutie 12000x6000, pe care am gasit-o conform ***Notei 1*** din sectiunea *Note* a acestui README.

<h2> Compilare/Rulare/Stergere Tema </h2>

Am dezvoltat un Makefile pentru un mod mai facil de lucru cu tema.

Dupa ce se dezarhiveaza/cloneaza tema local, rulati:
```makefile
    make
```
iar asta ar trebui sa va genereze cele 6 executabile, numite:

- serial
- omp
- pthreads
- mpi
- mpi_omp
- mpi_pthreads

Pentru rularea acestora puteti folosi alte reguli din Makefile, ca:

```makefile
    run_serial
    run_omp
    ...
    run_mpi_pthreads
```

si sa le executati in linia decomanda folosind de exemplu:

```makefile
    make run_serial
```

Aceste reguli dau ca argumente executabilelor aferente ca:
- numele pozei: ex: 12000height_6000width.bmp (situate in folderul ***photos***)
- numarul de procesoare: ex: "mpirun -np ***8*** mpi_pthreads ..." -> 8 procesoare
- numar de threaduri: ex: "mpirun -np 4 mpi_pthreads 12000height_6000width.bmp ***4*** -> 4 threaduri"

Dupa caz, unele ***argumente nu sunt existente*** in unele *executabile*, ca de exemplu ***numarul de threaduri*** in cazul executabilului ***serial***, sau ***numarul de procesoare*** in cazul executabilului ***omp*** sau ***pthreads***.

- Exemple de rulari sunt puse si in fisierul ***rezultate_rulari.txt*** , alaturi de timpii obtinuti de ele.

Daca se doreste ***rularea*** pe o alta poza decat cele existente in photos, formatul pozei trebuie sa fie ***"24-bit bitmap"*** si sa respecte ***Nota 2*** din sectiunea *Note*. Pentru convertirea pozei la formatul specificat, recomand tool-ul prezent in sectiune *Referinte*, punctul 2.

Pozele obtinute in urma rularii executabilelor pot fi vazute in folderul ***sharpened_photos***, si comparate fata de implementarea seriala folosind utilitare ca si ***cmp*** sau ***diff***, pentru a valida *determinismul* programelor.

<h1>Concluzii</h1>

Nu am intampinat probleme majore in rezolvarea temei propuse, insa am aprofundat notiuni legate de comunicarea dintre procese si modul de lucru in MPI si nu numai.

Asupra performantelor as face doar o serie de precizari, ce vor fi ulterior elaborate in cadrul prezentarii din etapa urmatoare a proiectului si anume:

- Se observa in general o rulare mai lenta pentru ***MPI*** fata de ***Pthreads/OpenMP***, datorat poate overheadului crearii proceselor fata de threaduri

- Se observa in general o rulare mai rapida pentru ***Pthreads*** fata de ***OpenMP***, probabil intrucat *OpenMP* utilizeaza *Pthreads*.

- ***Rularile hibride*** dau niste rezultate mai dificil de explicat si asta datorat *dublei sincronizari* (la nivel de threaduri si la nivel de procese).

<h2> Note: </h2>

1) Go to images.google.com and enter the search terms as before. Then append imagesize:WIDTHxHEIGHT to your query and hit Enter.

2) Testele au fost realizate pe poze cu dimensiune latime x inaltime, unde latime este multiplu de 4, intrucat padding-ul necesar in cazul latimii daca nu este multiplu de 4 nu este neaparat 0 si ar necesita adaptare de la imagine la imagine (sursa: https://stackoverflow.com/questions/2601365/padding-in-24-bits-rgb-bitmap)

3) Nu am verificat mereu alocarea sau deschiderea/inchiderea fisierelor, intrucat nu acesta era scopul temei.

4) Pentru masurarea timpului serial am folosit:
https://www.geeksforgeeks.org/how-to-measure-time-taken-by-a-program-in-c/

<h2> Referinte: </h2>

1) https://lodev.org/cgtutor/filtering.html
2) https://online-converting.com/image/convert2bmp/