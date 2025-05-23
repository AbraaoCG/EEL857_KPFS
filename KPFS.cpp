#include<iostream> 
#include<fstream> 


using namespace std;

struct Item {
    int id;
    int peso;
    int profit;
    int solv;

    Item (int _id, int p, int l , bool sol=0): id(_id), peso(p), profit(l), solv(sol) {}
};


// class item {
//     int id;
//     int peso;
//     int profit;
//     int solv;

//     public:
//         item(int id);

//         void add_peso(int p);

//         void add_profit(int l);
// };

// item::item(int id){
//     this->id = id;
// }


int main(){
    int nI = 0; //número de itens
    int nP = 0; //número de conjuntos de penalidade
    int kS = 0; //Capacidade de mochila
    fstream newfile;
    

    newfile.open("instances\\scenario1\\correlated_sc1\\300\\kpfs_1.txt", ios_base::in);
    
    if(newfile.is_open()){
        cout << "Arquivo aberto com sucesso." << endl;
        if (newfile >> nI) {
            // Leitura de nI bem-sucedida
        } else {
            cerr << "Erro ao ler o número de itens (nI) do arquivo." << endl;
            newfile.close(); // Fecha o arquivo antes de sair em caso de erro parcial
            return 1;        // Retorna um código de erro
        }

        if (newfile >> nP) {
            // Leitura de nP bem-sucedida
        } else {
            cerr << "Erro ao ler o número de conjuntos de penalidade (nP) do arquivo." << endl;
            newfile.close();
            return 1;
        }

        if (newfile >> kS) {
            // Leitura de kS bem-sucedida
        } else {
            cerr << "Erro ao ler a capacidade da mochila (kS) do arquivo." << endl;
            newfile.close();
            return 1;
        }

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