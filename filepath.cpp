#include<iostream>
#include "filepath.h"


std::pair<std::string, std::string> get_cenario(std::string cenario, std::string sc){
    if (cenario == "1") {
        return {"scenario1", "sc1"};
    } else if (cenario == "2") {
        return {"scenario2", "sc2"};
    } else if (cenario == "3") {
        return {"scenario3", "sc3"};
    } else if (cenario == "4") {
        return {"scenario4", "sc4"};
    } else {
        return {"erro", sc};
    }
}

std::string get_tipo(std::string tipo){
    if (tipo == "1") {
        return {"correlated"};
    } else if (tipo == "2") {
        return {"fully_correlated"};
    } else if (tipo == "3") {
        return {"not_correlated"};
    } else {
        return {"erro"};
    }
}

std::string get_tamanho(std::string tamanho) {
    if (tamanho == "1") {
        return "300";
    } else if (tamanho == "2") {
        return "500";
    } else if (tamanho == "3") {
        return "700";
    } else if (tamanho == "4") {
        return "800";
    } else if (tamanho == "5") {
        return "1000";
    } else {
        return "erro"; // or any other appropriate error handling
    }
}
