#include<stack> 
#include<iostream> 
#include<ctime> 
#include<list> 
#include<queue> 
#include <fstream> 
#include <bits/stdc++.h>

#include<iostream> // usado para "printar" no no terminal, utilizado durante os textes 
#include<ctime> // usado para ver o tempo para rodar os algoritimos
#include<list> // lista para a lista de adjacencia
#include<queue> // fila para BFS 
#include <fstream> // biblioteca usada para leitura/escrita do arquivo .txt
#include <bits/stdc++.h> // utilizado para a função sort nos algoritimos de componentes conexas

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