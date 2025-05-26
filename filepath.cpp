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
        return {"correlated_"};
    } else if (tipo == "2") {
        return {"fully_correlated_"};
    } else if (tipo == "3") {
        return {"not_correlated_"};
    } else {
        return {"erro"};
    }
}