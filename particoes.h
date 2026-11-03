#ifndef PARTICOES_H
#define PARTICOES_H

#include <stdio.h>

#include "lista.h"

/* Algoritmo de geracao de particoes por Classificacao Interna, recebe como parâmetro
o arquivo de dados de entrada, a lista contendo os nomes dos arquivos de saída das partições,
o númer de elementos M a ser armazenado em cada partição e o número de registro total do arquivo.*/
void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc);

/* Geracao de particoes por Selecao Natural. Recebe o arquivo de dados de entrada, a lista
com os nomes dos arquivos de saida das particoes, o numero de elementos M da memoria,
o numero total de registros do arquivo e o tamanho n do reservatorio.
RETORNA o numero de particoes que foram realmente geradas (necessario para a intercalacao,
pois a selecao natural gera menos particoes do que a classificacao interna). */
int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n);


#endif
