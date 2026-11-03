#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <semaphore.h>
#include <pthread.h>
#include <mutex>
#include <atomic>
#include <queue>
#include <csignal>


using namespace std;
struct Mesa {
    string tablero[7][5];
    int perrosCap = 0;
    atomic<bool>play{true};

    pthread_mutex_t mutexProtect;
    pthread_cond_t condicion;

    Mesa() {
        pthread_mutex_init(&mutexProtect, NULL);
        pthread_cond_init(&condicion, NULL);
    }

    ~Mesa() {
        pthread_mutex_destroy(&mutexProtect);
        pthread_cond_destroy(&condicion);
    }
};
struct Jugador {
    int id;
    int pareja;
    bool asignado = false;
    pthread_cond_t condicion;
    Jugador(int numero) {
        id = numero;
        pareja = -1;
        pthread_cond_init(&condicion, NULL);
    }
    ~Jugador() {
        pthread_cond_destroy(&condicion);
    }
};
atomic<bool>stop(false);
queue<Jugador*> colaJugadores;
sem_t mesasdispo;
pthread_mutex_t mutexTorneo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t jugadoresListos = PTHREAD_COND_INITIALIZER;
int jugadoresEsperando = 0;

void manejarSenal(int senal) {
    if (senal == SIGINT) {
        stop= 1;
    }
}

int randomtime() {
    return 50+rand()%251;
}
bool emparejar(Jugador &jugador) {
    pthread_mutex_lock(&mutexTorneo);
    if(stop){
    pthread_mutex_unlock(&mutexTorneo);
    return false;
    }
    if (colaJugadores.empty()) {
        colaJugadores.push(&jugador);
        while (!jugador.asignado && !stop) {
            pthread_cond_wait(&jugador.condicion, &mutexTorneo);
        }
        pthread_mutex_unlock(&mutexTorneo);
        return false;
    } else {
        Jugador* pareja = colaJugadores.front();
        colaJugadores.pop();
        jugador.pareja = pareja->id;
        pareja->pareja = jugador.id;
        jugador.asignado = true;
        pareja->asignado = true;
        pthread_cond_signal(&pareja->condicion);
        pthread_mutex_unlock(&mutexTorneo);
        return true;
    }
}
void CtrlC(sigset_t conjunto) {
    int senal;
    sigwait(&conjunto, &senal);
    stop = true;
    pthread_mutex_lock(&mutexTorneo);
    while (!colaJugadores.empty()) {
        Jugador* jugador = colaJugadores.front();
        colaJugadores.pop();
        pthread_cond_broadcast(&jugador->condicion);
    }
    pthread_mutex_unlock(&mutexTorneo);
}
void Config(string arch) { // saca las lineas de texto
    ifstream config(arch);
    string linea;
    int semilla;
    int min;
    int max;
    int Fpuma;
    int Cpuma;
    string palabra1;
    string caso;
    int numcaso;
    int casos= 0;
    string tipoevento;
    string pieza;
    string palabra;
    int id;
    int filaOrigen;
    int columnaOrigen;
    int filaDestino;
    int columnaDestino;
    int numeroFinal;

    while (getline(config, linea)) {
        stringstream ss(linea);
        ss >> palabra1;
        if (palabra1 == "SUITE_START") {
            ss >> casos;
        }
        if (palabra1 == "TEST_CASE"){
            ss >> numcaso;
            getline(ss, caso);
        }
        if (palabra1 == "CONFIG") {
          string palabra;
          ss >> palabra >> semilla;
          ss >> palabra >> min;
          ss >> palabra >> max;
          ss >> palabra >> Fpuma >> Cpuma;
        }
        if (palabra1 == "EVENT") {
          ss >> tipoevento;
          ss >> pieza >> id;
          ss >> palabra >> filaOrigen >> columnaOrigen;
          ss >> palabra >> filaDestino >> columnaDestino;
      }
      if (palabra1 == "TEST_END") {
          ss >> numeroFinal;
      }
    }
    config.close();
}
bool Adyacente(int filaOrigen, int columnaOrigen, int filaDestino, int columnaDestino) { //verificar si se puede mover
    int difFila = abs(filaDestino - filaOrigen);
    int difColumna = abs(columnaDestino - columnaOrigen);

    if(difFila==0&& difColumna==1){//horizontal
    return true;
    }
    if(difFila==1&& difColumna==0){ //vertical
    return true;
    }
    return false;
}
bool PerrosWin(Mesa &mesa) {
    pthread_mutex_lock(&mesa.mutexProtect);
    int filaPuma = -1;
    int columnaPuma = -1;
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (mesa.tablero[fila][columna] == "P") {
                filaPuma = fila;
                columnaPuma = columna;
            }
        }
    }
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (mesa.tablero[fila][columna] == "." &&
                Adyacente(filaPuma, columnaPuma, fila, columna)) {
                pthread_mutex_unlock(&mesa.mutexProtect);
                return false;
            }
        }
    }
    pthread_mutex_unlock(&mesa.mutexProtect);
    return true;
}
void Movimiento(Mesa &mesa, int filaOrigen, int columnaOrigen, int filaDestino, int columnaDestino) {
    pthread_mutex_lock(&mesa.mutexProtect);
    if (mesa.tablero[filaOrigen][columnaOrigen] == ".") {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    if (!Adyacente(filaOrigen, columnaOrigen, filaDestino, columnaDestino)) {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    if (mesa.tablero[filaDestino][columnaDestino] != ".") {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    mesa.tablero[filaDestino][columnaDestino] =
        mesa.tablero[filaOrigen][columnaOrigen];
    mesa.tablero[filaOrigen][columnaOrigen] = ".";
    pthread_mutex_unlock(&mesa.mutexProtect);
    if(mesa.play && PerrosWin(mesa)){
      mesa.play =false;
      cout<<"GANAN LOS PERROS"<<endl;
    }
}


void Tablero(Mesa &mesa) {
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            mesa.tablero[fila][columna] = '.';
        }
    }
    mesa.tablero[0][0] = ' ';
    mesa.tablero[0][1] = ' ';
    mesa.tablero[0][3] = ' ';
    mesa.tablero[0][4] = ' ';
    mesa.tablero[1][0] = ' ';
    mesa.tablero[1][4] = ' ';
    mesa.tablero[4][2] = "P";
    mesa.tablero[2][0] = "E";
    mesa.tablero[2][2] = "E";
    mesa.tablero[2][4] = "E";
    mesa.tablero[3][1] = "E";
    mesa.tablero[3][3] = "E";
    mesa.tablero[4][0] = "E";
    mesa.tablero[4][4] = "E";
    mesa.tablero[5][1] = "E";
    mesa.tablero[5][3] = "E";
    mesa.tablero[6][0] = "E";
    mesa.tablero[6][2] = "E";
    mesa.tablero[6][4] = "E";
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            cout << mesa.tablero[fila][columna] << "   ";
        }
        cout << endl;
    }
}
bool Pumawin(int perrosCap){
    if(perrosCap>5){
      return true;
    }
  return false;
}

void capturarPerro(Mesa &mesa, int filaOrigen, int columnaOrigen,
                   int filaPerro, int columnaPerro,
                   int filaDestino, int columnaDestino) {
    pthread_mutex_lock(&mesa.mutexProtect);
    if (mesa.tablero[filaDestino][columnaDestino] != ".") {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    if (mesa.tablero[filaOrigen][columnaOrigen] != "P") {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    if (mesa.tablero[filaPerro][columnaPerro] != "E") {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    if (!Adyacente(filaOrigen, columnaOrigen, filaPerro, columnaPerro) ||
        !Adyacente(filaPerro, columnaPerro, filaDestino, columnaDestino)) {
        pthread_mutex_unlock(&mesa.mutexProtect);
        return;
    }
    mesa.tablero[filaDestino][columnaDestino] = "P";
    mesa.tablero[filaOrigen][columnaOrigen] = ".";
    mesa.tablero[filaPerro][columnaPerro] = ".";
    mesa.perrosCap++;
    if (mesa.play && Pumawin(mesa.perrosCap)) {
        mesa.play = false;
        cout << "GANA EL PUMA" << endl;
    }
    pthread_mutex_unlock(&mesa.mutexProtect);
}

void pumamov(Mesa &mesa) {
    while (mesa.play && !stop) {
        this_thread::sleep_for(chrono::milliseconds(randomtime()));
        int filapuma = -1;
        int columnapuma = -1;
        pthread_mutex_lock(&mesa.mutexProtect);
        for (int fila = 0; fila < 7; fila++) {
            for (int columna = 0; columna < 5; columna++) {
                if (mesa.tablero[fila][columna] == "P") {
                    filapuma = fila;
                    columnapuma = columna;
                }
            }
        }
        pthread_mutex_unlock(&mesa.mutexProtect);
        if (!mesa.play) break;
        if (filapuma == -1) break;
        int filaPerro = -1;
        int columnaPerro = -1;
        int filaDestino = -1;
        int columnaDestino = -1;
        bool hayCaptura = false;
        pthread_mutex_lock(&mesa.mutexProtect);
        for (int f = 0; f < 7 && !hayCaptura; f++) {
            for (int c = 0; c < 5 && !hayCaptura; c++) {
                if (mesa.tablero[f][c] == "E" &&
                    Adyacente(filapuma, columnapuma, f, c)) {
                    for (int x = 0; x < 7 && !hayCaptura; x++) {
                        for (int y = 0; y < 5; y++) {
                            if (mesa.tablero[x][y] == "." &&
                                Adyacente(f, c, x, y)) {
                                filaPerro = f;
                                columnaPerro = c;
                                filaDestino = x;
                                columnaDestino = y;
                                hayCaptura = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(&mesa.mutexProtect);
        if (hayCaptura) {
            capturarPerro(mesa, filapuma, columnapuma,
                          filaPerro, columnaPerro,
                          filaDestino, columnaDestino);
            continue;
        }

        bool encontrar = false;
        pthread_mutex_lock(&mesa.mutexProtect);
        for (int fila = 0; fila < 7 && !encontrar; fila++) {
            for (int columna = 0; columna < 5; columna++) {
                if (mesa.tablero[fila][columna] == "." &&
                    Adyacente(filapuma, columnapuma, fila, columna)) {

                    filaDestino = fila;
                    columnaDestino = columna;
                    encontrar = true;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&mesa.mutexProtect);
        if (encontrar) {
            Movimiento(mesa, filapuma, columnapuma,
                       filaDestino, columnaDestino);
        }
    }
}

void perromov(Mesa &mesa, int num) {
    while (mesa.play && !stop) {
        this_thread::sleep_for(chrono::milliseconds(randomtime()));
        int contador = 0;
        int filaPerro = -1;
        int columnaPerro = -1;
        bool encontrado = false;
        pthread_mutex_lock(&mesa.mutexProtect);
        for (int fila = 0; fila < 7 && !encontrado; fila++) {
            for (int columna = 0; columna < 5; columna++) {
                if (mesa.tablero[fila][columna] == "E") {
                    contador++;

                    if (contador == num) {
                        filaPerro = fila;
                        columnaPerro = columna;
                        encontrado = true;
                        break;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mesa.mutexProtect);
        if (!encontrado) continue;
        int filaDestino = -1;
        int columnaDestino = -1;
        bool hayMovimiento = false;
        pthread_mutex_lock(&mesa.mutexProtect);
        for (int fila = 0; fila < 7 && !hayMovimiento; fila++) {
            for (int columna = 0; columna < 5; columna++) {
                if (mesa.tablero[filaPerro][columnaPerro] == "E" &&
                    mesa.tablero[fila][columna] == "." &&
                    Adyacente(filaPerro, columnaPerro, fila, columna)) {
                    filaDestino = fila;
                    columnaDestino = columna;
                    hayMovimiento = true;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&mesa.mutexProtect);
        if (hayMovimiento) {
            Movimiento(mesa, filaPerro, columnaPerro,
                       filaDestino, columnaDestino);
        }
    }
}
void jugarPartida() {
    if(sem_wait(&mesasdispo)!=0){return;};
    Mesa mesa;
    Tablero(mesa);
    thread hilos[13];
    hilos[0] = thread(pumamov, ref(mesa));
    for (int i = 1; i < 13; i++) {
        hilos[i] = thread(perromov, ref(mesa), i);
    }
    for (int i = 0; i < 13; i++) {
        hilos[i].join();
    }
    if(stop){
    mesa.play=false;
    }
    sem_post(&mesasdispo);
}
void cliente(Jugador* jugador) {
    bool inicia = emparejar(*jugador);
    if (inicia) {
        jugarPartida();
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    sigset_t conjunto;

    sigemptyset(&conjunto);
    sigaddset(&conjunto, SIGINT);
    pthread_sigmask(SIG_BLOCK, &conjunto, NULL);

    if (argc < 3 || argc > 4) {
        return 1;
    }
    int N = stoi(argv[1]);
    int M = stoi(argv[2]);
    if (argc == 4) {
        Config(argv[3]);
    }
    sem_init(&mesasdispo, 0, M);
    thread hiloCtrlC(CtrlC, conjunto);
    Jugador** jugadores = new Jugador*[N];
    thread *hilos = new thread[N];
    for (int i = 0; i < N; i++) {
        jugadores[i] = new Jugador(i + 1);
    }
    for (int i = 0; i < N; i++) {
        hilos[i] = thread(cliente, jugadores[i]);
    }
    for (int i = 0; i < N; i++) {
        hilos[i].join();
    }
    if (!stop) {
    pthread_kill(hiloCtrlC.native_handle(), SIGINT);
    }

    hiloCtrlC.join();
    for (int i = 0; i < N; i++) {
        delete jugadores[i];
    }
    delete[] jugadores;
    delete[] hilos;
    sem_destroy(&mesasdispo);
    pthread_mutex_destroy(&mutexTorneo);
    pthread_cond_destroy(&jugadoresListos);
    cout << "Torneo Komikan finalizado" << endl;
    return 0;
}
