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
bool PerrosWin() {

    lock_guard<mutex> bloqueo(mutexProtect);
    int filaPuma = -1;
    int columnaPuma = -1;
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (tablero[fila][columna] == "P") {
                filaPuma = fila;
                columnaPuma = columna;
            }
          }
    }
    for (int fila = 0; fila < 7; fila++) {
        for (int columna = 0; columna < 5; columna++) {
            if (tablero[fila][columna] == "." && Adyacente(filaPuma, columnaPuma, fila, columna)) {
                return false;
          }
      }
    }

    return true;
}
void Movimiento(string tablero[7][5], int filaOrigen,int columnaOrigen, int filaDestino,int columnaDestino) {
    {lock_guard<mutex> bloqueo(mutexProtect);
    if (tablero[filaOrigen][columnaOrigen] == ".") {
        cout << "SIN PIEZA ACA" << endl;
        return;
    }
    if (!Adyacente(filaOrigen, columnaOrigen, filaDestino, columnaDestino)) {
    cout << "MOVIMIENTO NO VALIDO" << endl;
    return;
    }
    if (tablero[filaDestino][columnaDestino] != ".") {
        cout << "OCUPADO" << endl;
        return;
    }
    tablero[filaDestino][columnaDestino] =tablero[filaOrigen][columnaOrigen];
    tablero[filaOrigen][columnaOrigen] = ".";}
    if(PerrosWin()){
    play=false;
    cout<<" GANAN LOS PERROS"<<endl;
    }
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
bool Pumawin(int perrosCap){
    if(perrosCap>5){
      return true;
    }
  return false;
}

void capturarPerro(string tablero[7][5], int filaOrigen, int columnaOrigen, int filaPerro, int columnaPerro, int filaDestino, int columnaDestino) {
    lock_guard<mutex> bloqueo(mutexProtect);
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
    if (!Adyacente(filaOrigen, columnaOrigen, filaPerro, columnaPerro) || !Adyacente(filaPerro, columnaPerro, filaDestino, columnaDestino)) {
        return;
    }
    
    tablero[filaDestino][columnaDestino] = "P";
    tablero[filaOrigen][columnaOrigen] = ".";
    tablero[filaPerro][columnaPerro] = ".";
    perrosCap++;
    if(Pumawin(perrosCap)){
    play=false;
    cout<<"A GANADO EL PUMA"<<endl;
    
    }
  
}

void pumamov() {
    while (play) {
        this_thread::sleep_for(chrono::milliseconds(randomtime()));

        int filapuma = -1;
        int columnapuma = -1;
        {
            lock_guard<mutex> bloqueo(mutexProtect);
            for (int fila = 0; fila < 7; fila++) {
                for (int columna = 0; columna < 5; columna++) {
                    if (tablero[fila][columna] == "P") {
                        filapuma = fila;
                        columnapuma = columna;
                  }
             }
            }
        }

        int filaPerro = -1;
        int columnaPerro = -1;
        int filaDestino = -1;
        int columnaDestino = -1;
        bool hayCaptura = false;
        {
            lock_guard<mutex> bloqueo(mutexProtect);
            for (int f = 0; f < 7 && !hayCaptura; f++) { //para captura
                for (int c = 0; c < 5 && !hayCaptura; c++) {
                    if (tablero[f][c] == "E" &&
                        Adyacente(filapuma, columnapuma, f, c)) {
                        for (int x = 0; x < 7 && !hayCaptura; x++) {
                            for (int y = 0; y < 5; y++) {
                                if (tablero[x][y] == "." &&
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
        }
        if (hayCaptura) {
            capturarPerro(tablero, filapuma, columnapuma,filaPerro, columnaPerro,filaDestino, columnaDestino);
            continue;
        }
        int filaDestinoNormal = -1;
        int columnaDestinoNormal = -1;
        bool encontrar = false;
        {
            lock_guard<mutex> bloqueo(mutexProtect);
            for (int fila = 0; fila < 7 && !encontrar; fila++) {
                for (int columna = 0; columna < 5; columna++) {
                    if (tablero[fila][columna] == "." &&
                        Adyacente(filapuma, columnapuma, fila, columna)) {
                        filaDestinoNormal = fila;
                        columnaDestinoNormal = columna;
                        encontrar = true;
                        break;
                  }
                }
            }
        }
        if (encontrar) {
        Movimiento(tablero, filapuma, columnapuma, filaDestinoNormal, columnaDestinoNormal);
        }
    }
}

void perromov(int num) {
    while (play) {
        this_thread::sleep_for(chrono::milliseconds(randomtime()));
        int contador = 0;
        int filaPerro = -1;
        int columnaPerro = -1;
        bool encontrado = false;
        {
          lock_guard<mutex> bloqueo(mutexProtect);
            for (int fila = 0; fila < 7 && !encontrado; fila++) {
                for (int columna = 0; columna < 5; columna++) {
                    if (tablero[fila][columna] == "E") {
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
        }
        if (!encontrado) {
            continue;
        }
        int filaDestino = -1;
        int columnaDestino = -1;
        bool hayMovimiento = false;
        {
            lock_guard<mutex> bloqueo(mutexProtect);
            if (tablero[filaPerro][columnaPerro] != "E") {
                continue;
            }
            for (int fila = 0; fila < 7 && !hayMovimiento; fila++) {
                for (int columna = 0; columna < 5; columna++) {
                    if (tablero[fila][columna] == "." &&
                        Adyacente(filaPerro, columnaPerro, fila, columna)) {
                        filaDestino = fila;
                        columnaDestino = columna;
                        hayMovimiento = true;
                        break;
                    }
                }
            }
        }
        if (hayMovimiento) {
        Movimiento(tablero, filaPerro, columnaPerro, filaDestino, columnaDestino);
        }
    }
}



int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (argc < 3 || argc > 4) {
        return 1;}
    if (argc == 4) {
        cout << "archivo de configuracion: "<<argv[3] << endl;
        Config(argv[3]);
    }
    int N, M;
    N = stoi(argv[1]);
    M = stoi(argv[2]);
    
    Tablero();
    
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

   
  

    return 0;
}
