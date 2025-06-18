#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio> // Include thecstdio library for the remove function
using namespace std;

void removeFile(const std::string& filePath) {
    // Use the remove function to delete the file
    if (std::remove(filePath.c_str()) != 0) {
        // Handle the error if the file cannot be removed
        perror("Error deleting file");
    } else {
        // File removed successfully
        // std::cout << "File removed successfully: " << filePath << std::endl;
    }
}
typedef pair<int,int> parValorPeso;

void writeAndPlotExecution(const vector<parValorPeso> data, const std::string& filePath) {
    // Abre o arquivo de script do Gnuplot
    std::ofstream gnuplotScript(filePath + ".gnuplot");
    cout << filePath + ".gnuplot" << endl;
    if (!gnuplotScript.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de script do Gnuplot." << std::endl;
        return;
    }

    // Configurações do terminal e arquivo de saída
    gnuplotScript << "set terminal pngcairo enhanced font 'Verdana,10' size 800,600\n";
    gnuplotScript << "set output '" << filePath << ".png'\n";
    gnuplotScript << "set title 'Evolução Temporal do Algorítimo genético - Valor e Peso'\n";
    gnuplotScript << "set xlabel 'Geração'\n";
    gnuplotScript << "set ylabel 'Valor'\n";
    gnuplotScript << "set grid\n";

    // Adiciona o comando de plotagem
    gnuplotScript << "plot '"+filePath + ".txt"+"' using 1:2 with points title 'Valor na mochila' lc rgb 'blue' ps 1.5, \\\n";
    gnuplotScript << "     '"+filePath + ".txt"+"' using 1:3 with points title 'Peso da mochila' lc rgb 'red' ps 1.5\n";

    gnuplotScript.close();

    // Abre o arquivo .txt para inserir os dados
    std::ofstream dataFile(filePath + ".txt");
    if (!dataFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo .txt." << std::endl;
        return;
    }
    // Insere os dados da coluna Fitness
    for (size_t i = 0; i < data.size(); ++i) {
        dataFile << i << " " << data[i].first << " " << data[i].second << "\n";
    }
    dataFile << "e\n";
    // Fecha o arquivo .txt
    dataFile.close();
    

    // Executa o script do Gnuplot
    std::string command = "gnuplot " + filePath + ".gnuplot";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Erro ao executar o Gnuplot. Certifique-se de que está instalado." << std::endl;
    }
    removeFile(filePath + ".gnuplot");

}



