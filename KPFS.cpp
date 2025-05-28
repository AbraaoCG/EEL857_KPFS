#include<iostream> 
#include<fstream>
#include <vector>
#include "filepath.h"


using namespace std;

struct Item {
    int id;
    int peso;
    int profit;
    bool solv;

    Item(int _id, int p, int l , bool sol=false): id(_id), peso(p), profit(l), solv(sol) {}
};

struct ConjuntoPenalidade{
    int id;
    int limiteItens; //Limite de itens que podem ser escolhidos deste conjunto sem penalidade.
    int penalidade; //Valor da penalidade associada a este conjunto
    int cardinalidade; //Cardinalidade do conjunto de penalidade
    vector<int> itens; //IDs dos itens que pertencem a este conjunto de penalidade

    ConjuntoPenalidade(int _id, int _limiteItens, int _penalidade, int _cardinalidade, vector<int> _itens): id(_id), limiteItens(_limiteItens), penalidade(_penalidade), cardinalidade(_cardinalidade), itens(_itens) {}
};

//TODO criar a estrutura dos conjuntos de penalidade (nP)

int main(){
    int nI = 0; //número de itens
    int nP = 0; //número de conjuntos de penalidade
    int kS = 0; //Capacidade de mochila
    vector<Item> items;
    string filepath, cenario, sc, tipo, tamanho, caso;
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
    
    if (cenario == "erro") {
        cerr << "Erro: Informe valores entre 1 e 4!";
        return 1;
    }
    sc = par_string.second;

    //Escolha do tipo de cenário
    cout << "Escolha o tipo do cenário:" << endl;
    cout << "1: correlated" << endl;
    cout << "2: fully correlated" << endl;
    cout << "3: not correlated" << endl;
    cin >> tipo;

    tipo = get_tipo(tipo);
    if (tipo == "erro") {
        cerr << "Erro: Informe valores entre 1 e 3!";
        return 1;
    }
    
    // TODO: Implementar a escolha do tamanho(300 até 1000) e do arquivo txt
    cout << "Escolha o tamanho do problema em número de itens:" << endl;
    cout << "1: 300" << endl;
    cout << "2: 500" << endl;
    cout << "3: 700" << endl;
    cout << "4: 800" << endl;
    cout << "5: 1000" << endl;
    cin >> tamanho;

    tamanho = get_tamanho(tamanho);
    if (tamanho == "erro") {
        cerr << "Erro: Informe valores entre 1 e 5!";
        return 1;
    }

    cout << "Escolha o caso desejado de 1 a 10:" << endl;
    cin >> caso;
    
    #ifdef _WIN32
        filepath = "instances\\" + cenario + "\\" + tipo + sc + "\\"+ tamanho+"\\kpfs_"+caso+".txt";
    #else
        filepath = "instances/" + cenario + "/" + tipo + sc + "/"+tamanho+"/kpfs_"+caso+".txt";
    #endif
    
    cout << filepath << endl;

    newfile.open(filepath, ios_base::in);
    
    if(newfile.is_open()) {
        cout << "Arquivo aberto com sucesso." << endl;
        newfile >> nI >> nP >> kS;


        vector<int> profits_temp(nI);
        vector<int> pesos_temp(nI);
        items.reserve(nI);

        //Lendo os valores de cada item
        for (int i = 0; i < nI; i++) {
            newfile >> profits_temp[i];
        }
        //Lendo os pesos de cada item
        for (int i = 0; i < nI; i++) {
            newfile >> pesos_temp[i];
        }

        //Criando os objetos Item
        for (int i = 0; i < nI; ++i) {
            items.emplace_back(i, pesos_temp[i], profits_temp[i]);
        }

        //TODO: Implementar lógica para ler e processar dados dos conjuntos de penalidade (nP)
        



        newfile.close(); // Fecha o arquivo após terminar de usá-lo
        cout << "Arquivo fechado." << endl;
    } else {
        cerr << "Erro: O arquivo não pôde ser aberto!" << endl;
        return 1; // Retorna um código de erro indicando falha
    }

    cout << nI << endl;
    cout << nP << endl;
    cout << kS << endl;

    // Checar se os itens foram lidos corretamente.
    for(const auto& item : items) {
        std::cout << "ID: " << item.id << ", Peso: " << item.peso << ", Profit: " << item.profit << std::endl;
    }
    return 0;
}