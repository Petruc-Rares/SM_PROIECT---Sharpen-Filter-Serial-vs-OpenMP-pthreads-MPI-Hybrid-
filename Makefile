build:
	gcc serial.c -o serial
	gcc omp.c -o omp -fopenmp
	mpicc mpi.c -o mpi
	mpicc mpi_omp.c -o mpi_omp -fopenmp
	gcc pthreads.c -o pthreads -lpthread
	mpicc mpi_pthreads.c -o mpi_pthreads -lpthread

run_serial:
	./serial 12000height_6000width.bmp

run_omp:
	./omp 12000height_6000width.bmp 4

run_mpi:
	mpirun -np 4 mpi 12000height_6000width.bmp

run_pthreads:
	./pthreads 12000height_6000width.bmp 2

run_mpi_omp:
	mpirun -np 4 mpi_omp photo_big_resolution.bmp 8

run_mpi_pthreads:
	mpirun -np 4 mpi_pthreads photo_big_resolution.bmp 4

clean:
	rm serial omp mpi mpi_omp mpi_pthreads pthreads sharpened_photos/*_sharpen.bmp