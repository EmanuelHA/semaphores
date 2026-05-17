#include <semaphore.h>  // sem_t, sem_init, sem_wait, sem_post
#include <pthread.h>    // pthread_t, pthread_create, pthread_join
#include <unistd.h>     // usleep
#include <stdlib.h>     // rand, srand
#include <time.h>       // time
#include <stdio.h>      // printf

#define N_CARRILES  4   // Numero de carriles (NORTE, SUR, ESTE, OESTE)
#define N_HILOS     N_CARRILES

#define N_VEHICULOS 10  // Total de vehiculos generados por carril


/* === VARIABLES GLOBALES (compartidas entre hilos) === */
int vehiculos_cruzados = 0; // total de vehiculos que han cruzado
int accidentes = 0;         // cruces simultaneos detectados 
int en_cruce = 0;           // flag para vehiculos en el cruce 
//int cola[4];                // vehiculos esperando por carril
int cruzados_por_carril[4]; // vehiculos que cruzaron por carril
sem_t semaforo_cruce;       // controla acceso al cruce central
sem_t mutex_contadores;     // protege vehiculos_cruzados y accidentes

enum CARRIL {NORTE, SUR, ESTE, OESTE};
const char* NOMBRE_CARRIL[] = {"Norte", "Sur", "Este", "Oeste"};

void* cruzar_carril(void* arg) {
    int id_carril = *(int*)arg;

    for (int i = 1; i <= N_VEHICULOS; i++) {
        if (en_cruce == 1) {
            printf("[%s-%03d] entrando al cruce <- ACCIDENTE\n", NOMBRE_CARRIL[id_carril], i);
            accidentes++; 
        } else {
            printf("[%s-%03d] entrando al cruce\n", NOMBRE_CARRIL[id_carril], i);
        }

        #ifdef FASE_2
        sem_wait(&semaforo_cruce);
        #endif
        en_cruce = 1;
        usleep(2500 + (rand() % (5000 - 2500 + 1)));  // USAR AQUÍ LA GENERACION DE TIEMPO ALEATORIO ENTRE 2500 y 5000 us
        en_cruce = 0;
        
        #ifdef FASE_2
        sem_post(&semaforo_cruce); 
        #endif

        #ifdef FASE_2
        sem_wait(&mutex_contadores);
        #endif

        vehiculos_cruzados++;
        cruzados_por_carril[id_carril]++;

        #ifdef FASE_2
        sem_post(&mutex_contadores);
        #endif

    }
    return NULL;
}

void print_reporte() {
    printf("========================================================\n");
    printf("     SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117\n");
    printf("         Carriles: %d | Vehiculos por carril: %d\n", N_CARRILES, N_VEHICULOS);
    printf("========================================================\n");
}

int main() {
    pthread_t hilos[N_CARRILES];
    int id_c[N_CARRILES];
    #ifdef FASE_2
    sem_init(&semaforo_cruce, 0, 1);
    sem_init(&mutex_contadores, 0, 1);
    #endif
    
    srand((unsigned)time(NULL));

    print_reporte();
    
    for(int i = 0; i < N_CARRILES; i++) {
        id_c[i] = i;
        pthread_create(&hilos[i], NULL, cruzar_carril, &id_c[i]);
    }
    
    // Esperar a que todos terminen
    for(int i = 0; i < N_CARRILES; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    #ifdef FASE_2
    sem_destroy(&semaforo_cruce);
    sem_destroy(&mutex_contadores);
    #endif

    return 0;
}


/* === REFERENCIAS ===
 * - "Chapter 70: Pthreads: Semaphores for Resource Counting & Synchronization" (https://circuitlabs.net/pthreads-semaphores-for-resource-counting-synchronization/)
*/