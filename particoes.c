
#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>

#include "particoes.h"

void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

/* Selecao Natural: gera particoes ordenadas usando um reservatorio (arquivo em disco).
   M = tamanho da memoria, n = tamanho do reservatorio.
   Retorna quantas particoes foram geradas. */
int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n) {
    if (n > M) n = M;
    if (n < 1) n = 1;
    rewind(arq);

    FILE *res = fopen("reservatorio.tmp", "wb+");
    if (res == NULL) {
        printf("Erro ao criar o reservatorio\n");
        return 0;
    }

    TFunc *v[M];            //memoria
    int tam_mem = 0;        //registros na memoria
    int tam_res = 0;        //registros no reservatorio
    int lidos = 0;          //registros ja lidos da entrada
    int num_particoes = 0;

    while (1) {
        //completa a memoria com registros da entrada
        while (tam_mem < M && lidos < nFunc) {
            fseek(arq, lidos * tamanho_registro(), SEEK_SET);
            v[tam_mem] = le_funcionario(arq);
            tam_mem++;
            lidos++;
        }
        if (tam_mem == 0) break; //nao sobrou nada: terminou

        //abre nova particao
        if (nome_arquivos_saida == NULL) {
            printf("Faltam nomes para as particoes\n");
            break;
        }
        FILE *p = fopen(nome_arquivos_saida->nome, "wb+");
        if (p == NULL) {
            printf("Erro criar arquivo de saida\n");
            break;
        }
        printf("\n%s\n", nome_arquivos_saida->nome);
        nome_arquivos_saida = nome_arquivos_saida->prox;
        num_particoes++;

        int cheio = 0; //fica 1 quando o reservatorio enche
        int reg_particao = 0;

        while (tam_mem > 0) {
            //acha o menor da memoria
            int pos = 0;
            for (int i = 1; i < tam_mem; i++) {
                if (v[i]->cod < v[pos]->cod)
                    pos = i;
            }
            TFunc *r = v[pos];

            //grava na particao
            fseek(p, reg_particao * tamanho_registro(), SEEK_SET);
            salva_funcionario(r, p);
            imprime_funcionario(r);
            reg_particao++;

            //substitui r pelo proximo da entrada
            int substituiu = 0;
            while (!substituiu) {
                if (cheio || lidos == nFunc) {
                    //nao da para repor: tira r da memoria
                    v[pos] = v[tam_mem - 1];
                    tam_mem--;
                    substituiu = 1;
                } else {
                    fseek(arq, lidos * tamanho_registro(), SEEK_SET);
                    TFunc *novo = le_funcionario(arq);
                    lidos++;
                    if (novo->cod >= r->cod) {
                        v[pos] = novo;      //ainda cabe nesta particao
                        substituiu = 1;
                    } else {
                        //menor que o ultimo gravado: vai para o reservatorio
                        fseek(res, tam_res * tamanho_registro(), SEEK_SET);
                        salva_funcionario(novo, res);
                        tam_res++;
                        free(novo);
                        if (tam_res == n)
                            cheio = 1;
                    }
                }
            }
            free(r);
        }
        fclose(p);

        //passa o reservatorio para a memoria e esvazia o reservatorio
        if (tam_res > 0) {
            for (int i = 0; i < tam_res; i++) {
                fseek(res, i * tamanho_registro(), SEEK_SET);
                v[i] = le_funcionario(res);
            }
            tam_mem = tam_res;
            tam_res = 0;
            fclose(res);
            res = fopen("reservatorio.tmp", "wb+");
        }
    }

    for (int i = 0; i < tam_mem; i++) free(v[i]);
    fclose(res);
    remove("reservatorio.tmp");
    return num_particoes;
}
