#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>

int Sudoku[8][8];
int comp[9] = {1,2,3,4,5,6,7,8,9};

void Leer(char *name){
	FILE* fd;
	char c[82];
	int i, len, z, x, y;
	//abrir archivo
	fd = fopen(name,"rt");
	fgets(c,82,fd);
	fclose(fd);
	int file = open(name, O_RDONLY, S_IRUSR | S_IWUSR);
	//printf(c);
	char *content = mmap(NULL, 82, PROT_READ, MAP_PRIVATE, file, 0);
	//printf("\n");
	int pointer = 0;
	for( x = 0; x <=8 ; x++){
		for( y = 0; y <=8 ; y++){
			z = c[pointer];
			//printf("%d \n", z-'0');
			Sudoku[x][y] = z-'0';
			pointer++;
		}	
	}
	//printf("%d \n", Sudoku[8][8]);
}

void RevFilas(){
	int fila[9];
	int i,y;
	int cont = 0;
	for (i = 0; i <= 8; ++i)
	{
		for (y = 0; y <= 8; ++y)
		{
			fila[y] = Sudoku[i][y];
			if(y == 8){
				int ret = 0;
				ret = memcmp(comp, fila, 9);

				if(ret == -1){
					//printf("la fila esta correcta\n");
					cont++;
				} else {
					//printf("la fila no esta correcta\n");
   				}
   				//fila = {};
			}
		}
	}
	if (cont == 9)
	{
		printf("todas las filas estan correctas\n");
	}
}

void RevColum(){
	omp_set_nested(1);
	omp_set_num_threads(9);
	int fila[9];
	int i,y;
	int cont = 0;
	#pragma omp parallel for private(fila)
	for (i = 0; i <= 8; ++i)
	{
		printf("En la revision de columnas el siguiente es un thread en ejecuion: %d\n", syscall(SYS_gettid));
		#pragma omp parallel for schedule(dynamic)
		for (y = 0; y <= 8; ++y)
		{
			fila[y] = Sudoku[y][i];
			//printf("%d",fila[y]);
			//printf("%d\n",Sudoku[y][i]);
			if(y == 8){
				int ret = 0;
				
				ret = memcmp(comp, fila, 9);
				//printf("%d <---\n",ret);
				if(ret == -1) {
					//printf("la columna esta correcta\n");
					cont++;
				} else {
					//printf("la columna no esta correcta\n");
   				}
   				//fila = {};
			}
		}
	}
	if (cont == 9)
	{
		printf("todas las columnas estan correctas\n");
	}
}

void SubArreglos(){
	int i,j,k,w, cont ,cont1,cont2,cont3 = 0;
	int fila[9];
	for ( i = 0; i <=2 ; ++i)
	{
		for (j = 0; j <=2 ; ++j)
		{
			for (k = 0; k <=2 + cont2 ; ++k)
			{
				for (w = 0; w <=2 + cont1 ; ++w)
				{	
					fila[k] = Sudoku[k][w];
				}
				cont1 += 3;
				if(k == 2){
					int ret = 0;
					
					ret = memcmp(comp, fila, 9);
					//printf("%d <---\n",ret);
					if(ret == -1) {
						//printf("la columna esta correcta\n");
						
					} else {
						//printf("la columna no esta correcta\n");
	   				}
	   				//fila = {};
				}
			}
			cont2 += 3;
		}
		cont++;
	}
	if (cont >= 9)
		{
			printf("todos los subarreglos estan correctas\n");
		}
}

void *ejecRevColum(){
	printf("EL thread que ejecuta el metodo para ejecutar el metodo de revision de columnas es: %d\n", syscall(SYS_gettid));
	RevColum();
	pthread_exit(0);
}

void *ejecRevFilas(){
	printf("EL thread que ejecuta el metodo para ejecutar el metodo de revision de filas es: %d\n", syscall(SYS_gettid));
	RevFilas();
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	omp_set_num_threads(1);
	Leer(argv[1]);
	//RevColum();
	//RevFilas();
	
	pid_t pid1 = getpid();
	int nProc = (int)pid1;

	int fork1 = 0;
	fork1 = fork();
	
	if (fork1 < 0)
	{
		printf("error en crear fork");
		return 1;
	}

	if (fork1 == 0)
	{
		char pid[6];
		sprintf(pid, "%d", nProc);
		execlp("ps", "ps", "-p", pid, "-1Lf", (char *)NULL);
	}
	else{
		pthread_t tRevCols;
		if (pthread_create(&tRevCols, NULL, ejecRevColum, NULL))
		{
			fprintf(stderr, "error al crear thread\n");
			return 1;
		}

		if (pthread_join(tRevCols, NULL))
		{
			fprintf(stderr, "error al unirse el thread\n");
			return 2;
		}

		printf("el thread que se esta ejecutando es: %d\n", syscall(SYS_gettid));

		usleep(1000);

		pthread_t tRevFils;
		if (pthread_create(&tRevFils, NULL, ejecRevFilas, NULL))
		{
			fprintf(stderr, "error al crear thread\n");
			return 1;
		}

		if (pthread_join(tRevFils, NULL))
		{
			fprintf(stderr, "error al unirse el thread\n");
			return 2;
		}

		int fork2 = 0;
		fork2 = fork();

		if (fork2 < 0)
		{
			printf("error al crear fork\n");
			return 1;
		}

		if (fork2 == 0)
		{
			char pid[6];
			sprintf(pid, "%d", nProc);
			execlp("ps", "ps", "-p", pid, "-1Lf", (char *)NULL);
		}
		else{
			printf("espera\n");
			usleep(2000);
			printf("termino la espera\n");
		}
	}
	return 0;
}
