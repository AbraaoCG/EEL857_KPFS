# EEL857_KPFS
Solver para o Problema da Mochila com Conjuntos de Penalidade (KPFS), uma extensão do problema clássico da mochila que incorpora penalidades por excesso de seleção em grupos específicos de itens. O repositório implementa diversas metaheurísticas e testes realizados em múltiplos cenários.


## Exemplo de execução - Linux

### Main

"g++ -O2 -fopenmp main.cpp -o main.out"

"./main.out scenario1/correlated_sc1/300/kpfs_1.txt"

### Main_benchmark (executa todos os casos)

"g++ -O2 -fopenmp main_benchmark.cpp -o main_b.out"

"./main_b.out"

Em seguida selecionar o algorítimo desejado.
