#include <stdlib.h>

#include "manipulacao_arquivos.h"
#include "funcionarios.h"
#include "math.h"

int tamanho_arquivo(FILE *arq) {
    fseek(arq, 0, SEEK_END);
    //arredonda para cima: o ultimo registro gravado campo a campo nao inclui o padding do struct
    int tam = (int) ((ftell(arq) + tamanho_registro() - 1) / tamanho_registro());
    return tam;
}