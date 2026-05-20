### Opción 1: Usando make
```bash
make        # Compila el programa
make clean  # Limpia el ejecutable
```

### Opción 2: Compilación manual
```bash
# FASE 1: Sin sincronización
make
./interseccion

# O el programa ya ejecuta ambas fases automaticamente
```

## Ejecución

El programa ejecuta **ambas fases en secuencia** y muestra:
1. FASE 1: Sin sincronización (detecta accidentes)
2. FASE 2: Con semáforos (sin accidentes)
3. Reporte final con comparación de tiempos

Simplemente ejecuta:
```bash
make
./interseccion
```

## Salida esperada

```
========================================================
     SIMULADOR DE INTERSECCION DE TRAFICO - CI-0117
         Carriles: 4 | Vehiculos por carril: 10
========================================================
--- FASE 1: Sin sincronizacion ---
[Norte-001] entrando al cruce
[Sur-001] entrando al cruce <- ACCIDENTE
[Norte-001] cruce completado
...

--- FASE 2: Con semaforos ---
[Norte-001] sem_wait() -> cruzando
[Norte-001] sem_post() -> cruce libre
[Sur-001] sem_wait() -> cruzando
...

======== REPORTE FINAL ========
FASE 1 (sin sincronizacion):
Total vehiculos: 40
Accidentes: 7 <- debe ser > 0
Vehiculos/carril: Norte=10 Sur=10 Este=10 Oeste=10
Tiempo simulacion: 0.312 segundos

FASE 2 (con semaforos):
Total vehiculos: 40
Accidentes: 0 <- debe ser exactamente 0
Vehiculos/carril: Norte=10 Sur=10 Este=10 Oeste=10
Tiempo simulacion: 0.418 segundos

ANALISIS:
Overhead de sincronizacion: +0.106 seg (+34.0%)
========================================================
```

## Configuración

Puedes modificar estos valores en `interseccion.c`:
- `N_VEHICULOS`: Número de vehículos por carril (default: 10)
- `MIN_DELAY` / `MAX_DELAY`: Tiempo de simulación entre vehículos (microsegundos)

#### a) Race Condition
Identifique exactamente cuáles líneas del código de Fase 1
constituyen una race condition. ¿Por qué la instrucción `en_cruce = 1` sola no es
suficiente para garantizar exclusión mutua?

R//: 
     
     47: if (en_cruce == 1) { ... }    // Lectura sucia
     58: en_cruce = 1;                 // Escritura sucia
     60: en_cruce = 0;                 // Escritura sucia
     71: vehiculos_cruzados++;         // Escritura sucia

`en_cruce = 1` no garantiza exclusión mutua porque no es una instrucción atómica (carga, modifica y guarda).

#### b) Invariante del semáforo
¿Qué invariante garantiza el semáforo binario en Fase 2? Explíquelo con sus propias palabras.

R//:  La invariante semáforo garantiza que se cumpla en todo el recorrido del programa que en las fases críticas cerradas por este (``sem_wait() **sección crítica** sem_wait()``) siempre haya solo un hilo (o cero en caso de no haber tránsito), lo que garantiza que no hayan condiciones de carrera en esa sección de código.

¿Por qué se inicializa en 1 y no en 0?

R//: Esto se debe a que es un semáforo binario. Los semáforos fucionan como tiquetes o llaves de entrada, cada que se hace un ``sem_wait()``, se resta un tiquete/llave hasta que este es devuelto (``sem_post()``). Por lo que al inicializarlo en 1, estamos garantizando que solo exista un tiquete, lo que consecuentemente da acceso a solo un hilo a esta sección. 

#### c) Overhead
¿Por qué la Fase 2 suele tomar más tiempo que la Fase 1? ¿Este
overhead es aceptable? ¿En qué tipo de sistema NO sería aceptable?

R//: Esto ocurre por el overhead que causan ``sem_wait()`` y ``sem_post()`` al llamar a sistema para verificar los tiquets y en el caso de no quedar, poner en espera a los demás hilos hasta que se libere el semáforo. Es aceptable y necesario porque requerimos mantener la concurrencia de los datos, lo cual se pierde por completo al no usarlos, es decir, el programa no sirve, son puros accidentes . 

#### d) Experimento
Presente los resultados del experimento de escala (N_VEHICULOS = 5, 20, 50). ¿Cómo cambia la tasa de accidentes al aumentar el número de vehículos? ¿Tiene sentido ese comportamiento?

| Vehículos | Accidentes | Tasa |
|:--:|:------:|:---:|
| 5  |  10~13 |~ 2.4|
| 20 |  36~48 |~ 2.1|
| 50 | 92~110 |~ 2.0|

Tiene todo el sentido, dado que a mayor número de vehículos se presenta proporcionalmente un mayor número de accesos al cruce y estadía de los vehículos en el mismo (tiempo de cruce del vehículo), lo que produce un crecmiento altísimo en la cantidad de accidentes a medida que aumentamos el núnero de vehículos.

#### e) Extensión (opcional, bonus +0.5)
¿Cómo modificaría el sistema para permitir que más de un vehículo cruce simultáneamente (carril de doble vía)? ¿Qué tipo de semáforo usaría?

Permitiría el paso de 2 vehículos en simultáneo por carril y duplicaría la cantidad de carriles (hilos) a ``2*4 = 8``. Además, cambiaría la condición de, Para ello emplearía ambos semáforos (``semaforo_cruce`` y ``mutex_contador``) inicializados en 2, es decir, con 2 tiquets para permitir entrada de 2 carriles.