#include<iostream> 
#include<fstream>
#include "filepath.h"


using namespace std;

struct Item {
    int id;
    int peso;
    int profit;
    bool solv;

    Item (int _id, int p, int l , bool sol=false): id(_id), peso(p), profit(l), solv(sol) {}
};


int main(){
    int nI = 0; //número de itens
    int nP = 0; //número de conjuntos de penalidade
    int kS = 0; //Capacidade de mochila
    string filepath, cenario, sc, tipo;
    pair<string, string> par_string;
    fstream newfile;


    //Escolha do cenário
    cout << "Escolha o cenário:" << endl;
    cout << "1: scenario1" << endl;
    cout << "2: scenario2" << endl;
    cout << "3: scenario3" << endl;
    cout << "4: scenario4" << endl;
    cin >> cenario;

    par_string = get_cenario(cenario, sc);
    cenario = par_string.first;
    
    if (cenario == "erro"){
        cerr << "Erro: Informe valores entre 1 e 4!";
        return 1;
    }
    sc = par_string.second;
   
    cout << "Escolha o tipo do cenário:" << endl;
    cout << "1: correlated" << endl;
    cout << "2: fully correlated" << endl;
    cout << "3: not correlated" << endl;
    cin >> tipo;

    tipo = get_tipo(tipo);
    if (tipo == "erro"){
        cerr << "Erro: Informe valores entre 1 e 3!";
        return 1;
    }
    
    filepath = "instances\\" + cenario + "\\" + tipo + sc + "\\300\\kpfs_1.txt";
    cout << filepath << endl;

    newfile.open(filepath, ios_base::in);
    
    if(newfile.is_open()){
        cout << "Arquivo aberto com sucesso." << endl;
        newfile >> nI;
        newfile >> nP;
        newfile >> kS;

        newfile.close(); // Fecha o arquivo após terminar de usá-lo
        cout << "Arquivo fechado." << endl;
    } else{
        cerr << "Erro: O arquivo não pôde ser aberto!" << endl;
        return 1; // Retorna um código de erro indicando falha
    }

    cout << nI << endl;
    cout << nP << endl;
    cout << kS << endl;
    return 0;
}