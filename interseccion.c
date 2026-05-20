#include <semaphore.h>  // sem_t, sem_init, sem_wait, sem_post
#include <pthread.h>    // pthread_t, pthread_create, pthread_join
#include <unistd.h>     // usleep
#include <stdlib.h>     // rand, srand
#include <time.h>       // time, clock_gettime, CLOCK_MONOTONIC
#include <stdio.h>      // printf

#define N_CARRILES  4
#define N_HILOS     N_CARRILES
#define N_VEHICULOS 50
#define MIN_DELAY 2000
#define MAX_DELAY 5000

/* === VARIABLES GLOBALES === */
int fase_actual = 1;
int vehiculos_cruzados = 0;
int accidentes = 0;
int en_cruce = 0;
int cruzados_por_carril[N_CARRILES];
sem_t semaforo_cruce;
sem_t mutex_contadores;

// Para guardar resultados de cada fase
int vehiculos_cruzados_f1 = 0, accidentes_f1 = 0, cruzados_por_carril_f1[N_CARRILES];
int vehiculos_cruzados_f2 = 0, accidentes_f2 = 0, cruzados_por_carril_f2[N_CARRILES];
double tiempo_f1 = 0, tiempo_f2 = 0;

enum CARRIL {NORTE, SUR, ESTE, OESTE};
const char* NOMBRE_CARRIL[] = {"Norte", "Sur", "Este", "Oeste"};


void reiniciar_contador() {
    vehiculos_cruzados = 0; accidentes = 0; en_cruce = 0;
    for (int i = 0; i < N_CARRILES; i++) cruzados_por_carril[i] = 0;
}

void* cruzar_carril(void* arg) {
    int id_carril = *(int*)arg;
    unsigned int semilla = time(NULL) + id_carril + fase_actual * 1000;
    
    for (int i = 1; i <= N_VEHICULOS; i++) {
        if (fase_actual == 2) {
            sem_wait(&semaforo_cruce);
            printf("[%s-%03d] sem_wait() -> cruzando\n", NOMBRE_CARRIL[id_carril], i);
        }
        
        if (en_cruce == 1) {
            if (fase_actual == 1) {
                printf("[%s-%03d] entrando al cruce <- ACCIDENTE\n", NOMBRE_CARRIL[id_carril], i);
            }
            accidentes++;
        } else {
            if (fase_actual == 1) {
                printf("[%s-%03d] entrando al cruce\n", NOMBRE_CARRIL[id_carril], i);
            }
        }
        
        en_cruce = 1;
        usleep(MIN_DELAY + (rand_r(&semilla) % (MAX_DELAY - MIN_DELAY + 1)));
        en_cruce = 0;
        
        if (fase_actual == 2) {
            printf("[%s-%03d] sem_post() -> cruce libre\n", NOMBRE_CARRIL[id_carril], i);
        }
        
        if (fase_actual == 2) {
            sem_post(&semaforo_cruce);
            sem_wait(&mutex_contadores);
        }

        vehiculos_cruzados++;
        cruzados_por_carril[id_carril]++;

        if (fase_actual == 2) {
            sem_post(&mutex_contadores);
        } else {
            printf("[%s-%03d] cruce completado\n", NOMBRE_CARRIL[id_carril], i);
        }
        
        usleep(MIN_DELAY + (rand_r(&semilla) % (MAX_DELAY - MIN_DELAY + 1)));
    }
    return NULL;
}

/*Correr simulacion */
void simulacion(double *elapsed) {
    pthread_t hilos[N_CARRILES];
    int id_c[N_CARRILES];
    struct timespec start, end;
    
    if (fase_actual == 2) {
        sem_init(&semaforo_cruce, 0, 1);
        sem_init(&mutex_contadores, 0, 1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for(int i = 0; i < N_CARRILES; i++) {
        id_c[i] = i;
        pthread_create(&hilos[i], NULL, cruzar_carril, &id_c[i]);
    }
    
    for(int i = 0; i < N_CARRILES; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    *elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    
    if (fase_actual == 1) {
        vehiculos_cruzados_f1 = vehiculos_cruzados;
        accidentes_f1 = accidentes;
        for (int i = 0; i < N_CARRILES; i++) cruzados_por_carril_f1[i] = cruzados_por_carril[i];
        tiempo_f1 = *elapsed;
    } else {
        vehiculos_cruzados_f2 = vehiculos_cruzados;
        accidentes_f2 = accidentes;
        for (int i = 0; i < N_CARRILES; i++) cruzados_por_carril_f2[i] = cruzados_por_carril[i];
        tiempo_f2 = *elapsed;
    }
    
    if (fase_actual == 2) {
        sem_destroy(&semaforo_cruce);
        sem_destroy(&mutex_contadores);
    }
}

void correr_fase(int fase) {
    reiniciar_contador();
    // Fase 1
    if (fase == 1){
        fase_actual = 1;
        printf("--- Fase 1: Sin sincronizacion ---\n");
        simulacion(&tiempo_f1);
    }
    // Fase 2
    if (fase == 2) {
        fase_actual = 2;
        printf("\n--- Fase 2: Con semaforos ---\n");
        simulacion(&tiempo_f2);
    }
}

void print_header() {
    printf("========================================================\n");
    printf("     SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117\n");
    printf("         Carriles: %d | Vehiculos por carril: %d\n", N_CARRILES, N_VEHICULOS);
    printf("========================================================\n");
}

void print_reporte() {
    printf("\n======== REPORTE FINAL ========\n");
    
    printf("FASE 1 (sin sincronizacion):\n");
    printf("Total vehiculos: %d\n", vehiculos_cruzados_f1);
    printf("Accidentes: %d <- debe ser > 0\n", accidentes_f1);
    printf("Vehiculos/carril: %s=%d %s=%d %s=%d %s=%d\n",
           NOMBRE_CARRIL[NORTE],    cruzados_por_carril_f1[NORTE],
           NOMBRE_CARRIL[SUR],      cruzados_por_carril_f1[SUR],
           NOMBRE_CARRIL[ESTE],     cruzados_por_carril_f1[ESTE],
           NOMBRE_CARRIL[OESTE],    cruzados_por_carril_f1[OESTE]);
    printf("Tiempo simulacion: %.3f segundos\n", tiempo_f1);
    
    printf("\nFASE 2 (con semaforos):\n");
    printf("Total vehiculos: %d\n", vehiculos_cruzados_f2);
    printf("Accidentes: %d <- debe ser exactamente 0\n", accidentes_f2);
    printf("Vehiculos/carril: %s=%d %s=%d %s=%d %s=%d\n",
           NOMBRE_CARRIL[NORTE],    cruzados_por_carril_f2[NORTE],
           NOMBRE_CARRIL[SUR],      cruzados_por_carril_f2[SUR],
           NOMBRE_CARRIL[ESTE],     cruzados_por_carril_f2[ESTE],
           NOMBRE_CARRIL[OESTE],    cruzados_por_carril_f2[OESTE]);
    printf("Tiempo simulacion: %.3f segundos\n", tiempo_f2);

    printf("\nANALISIS:\n");
    double overhead = tiempo_f2 - tiempo_f1;
    double percentage = (tiempo_f1 > 0) ? (overhead / tiempo_f1) * 100 : 0;
    printf("Overhead de sincronizacion: +%.3f seg (+%.1f%%)\n", overhead, percentage);
    printf("========================================================\n");
}

int main() {
    print_header();

    correr_fase(1);
    correr_fase(2);

    print_reporte();
    
    return 0;
}
