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


using namespace std;
string tablero[7][5];
int perrosCap=0;
atomic<bool> play(true); //consulta hilos
mutex mutexProtect;
int randomtime() {
    return 50+rand()%251;
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

void Tablero() {
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            tablero[fila][columna] = '.';
        }
    }
    tablero[0][0] = ' ';
    tablero[0][1] = ' ';
    tablero[0][3] = ' ';
    tablero[0][4] = ' ';
    tablero[1][0] = ' ';
    tablero[1][4] = ' ';
    tablero[4][2] = "P";
    tablero[2][0] = "E";
    tablero[2][2] = "E";
    tablero[2][4] = "E";
    tablero[3][1] = "E";
    tablero[3][3] = "E";
    tablero[4][0] = "E";
    tablero[4][4] = "E";
    tablero[5][1] = "E";
    tablero[5][3] = "E";
    tablero[6][0] = "E";
    tablero[6][2] = "E";
    tablero[6][4] = "E";
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            cout << tablero[fila][columna] << "   ";
        }
        cout << endl;
    }
}
void pumamov(){
    this_thread::sleep_for(chrono::milliseconds(randomtime())); // espera
    lock_guard<mutex> bloqueo(mutexProtect);
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (tablero[fila][columna] == "P") {
              cout << "Puma ubicado en: ["<< fila << "," << columna <<"]"<< endl;
            }
        }
    }
}
void perromov(int num){
    this_thread::sleep_for(chrono::milliseconds(randomtime()));
    lock_guard<mutex> bloqueo(mutexProtect);
    int contador = 0;
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (tablero[fila][columna] == "E") {
                contador++;
                if (contador == num) {
                    cout << "Perro " << num<< " ubicado en: ["<< fila << ", " << columna <<"]"<< endl;
                    return;
                }
            }
        }
    }
}
void Movimiento(string tablero[7][5], int filaOrigen,int columnaOrigen, int filaDestino,int columnaDestino) {
    lock_guard<mutex> bloqueo(mutexProtect);
    if (tablero[filaOrigen][columnaOrigen] == ".") {
        cout << "SIN PIEZA ACA" << endl;
        return;
    }
    if (tablero[filaDestino][columnaDestino] != ".") {
        cout << "OCUPADO" << endl;
        return;
    }
    tablero[filaDestino][columnaDestino] =tablero[filaOrigen][columnaOrigen];
    tablero[filaOrigen][columnaOrigen] = ".";
}

bool Adyacente(int filaOrigen, int columnaOrigen, int filaDestino, int columnaDestino) { //verificar si se puede mover
    int difFila = abs(filaDestino - filaOrigen);
    int difColumna = abs(columnaDestino - columnaOrigen);

    if (difFila <= 1 && difColumna <= 1) {
        if (difFila != 0 || difColumna != 0) {
            return true;
        }
    }
    return false;
}
void capturarPerro(string tablero[7][5], int filaOrigen, int columnaOrigen, int filaPerro, int columnaPerro, int filaDestino, int columnaDestino) {

    if (tablero[filaDestino][columnaDestino] != ".") {
        cout << "OCUPADO" << endl;
        return;
    }
    if (tablero[filaOrigen][columnaOrigen] != "P") {
        cout << "NO ES PUMA" << endl;
        return;
    }

    if (tablero[filaPerro][columnaPerro] != "E") {
        cout << "Sin perro" << endl;
        return;
    }
    tablero[filaDestino][columnaDestino] = "P";
    tablero[filaOrigen][columnaOrigen] = ".";
    tablero[filaPerro][columnaPerro] = ".";
    perrosCap++;
}
bool Pumawin(int perrosCap){
    if(perrosCap>5){
      return true;
    }
  return false;
}

void Config(string arch);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (argc < 3 || argc > 4) {
        return 1;}

    int N, M;
    N = stoi(argv[1]);
    M = stoi(argv[2]);
    //threads [0] puma, el resto perros
    thread hilos[13];
    hilos[0]=thread(pumamov);

    for(int i=1; i<13; i++){
      hilos[i]=thread(perromov, i);
    }
    for(int i=0; i<13; i++){
      hilos[i].join();
    }

    cout<< "Torneo Komikan"<< endl;
    cout <<"numero total de clientes: "<<N<< endl;
    cout << "Max. de mesas: "<< M << endl;

    if (argc == 4) {
        cout << "archivo de configuracion: "<<argv[3] << endl;
        Config(argv[3]);
    }
  Tablero();

    return 0;
}
