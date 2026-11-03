# Tarea-2---Sistemas-Operativos

El juego enfrenta a un Puma contra doce Perros en un tablero de 5 × 5 posiciones conectado a un triángulo o trono.
Las partidas funcionan sin turnos estrictos. Cada pieza intenta realizar movimientos después de un retardo aleatorio, simulando una competencia en tiempo real.

Para compilar y ejecutar el proyecto se necesita el Sistema operativo Linux o Unix, Compilador G++, Soporte para C++17, Biblioteca POSIX Threads.
se compila con el siguiente codigo: g++ -Wall -Wextra -std=c++17 -pthread main.cpp -o torneo
El programa recibe dos parámetros obligatorios y uno opcional:
./torneo N M [config.txt]

El proyecto utiliza primitivas POSIX para coordinar los hilos y proteger los recursos compartidos:
- pthread_mutex_t: protege el acceso al tablero y a la cola de jugadores.
- pthread_cond_t: coordina la espera y notificación durante el emparejamiento.
- sem_t: controla la cantidad máxima de mesas simultáneas.
Cada mesa posee su propio tablero y estado de partida, permitiendo que diferentes partidas se ejecuten independientemente.
Los movimientos se protegen mediante exclusión mutua para evitar que dos hilos modifiquen simultáneamente el mismo tablero.
Los retardos aleatorios se encuentran entre 50 y 300 milisegundos.
El programa contempla la recepción de la señal SIGINT, correspondiente a Ctrl+C, mediante sigwait().
