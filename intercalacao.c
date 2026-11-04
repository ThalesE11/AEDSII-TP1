#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

/* ------------------------------------------------------------------
   INTERCALACAO COM ARVORE BINARIA DE VENCEDORES
   ------------------------------------------------------------------
   A arvore e guardada em um vetor (como um heap):
     - raiz no indice 1
     - filhos do no i: 2*i (esq) e 2*i+1 (dir)
     - folhas nos indices [folhas, 2*folhas-1]  (folhas = potencia de 2 >= num_p)
   Cada no e um TPilha {info, p}: info = registro vencedor, p = particao de origem.
   info == NULL significa HIGH VALUE (particao esgotada ou folha de enchimento). */

/* devolve o vencedor (menor chave) entre dois nos */
static TPilha vencedor(TPilha a, TPilha b) {
    if (a.info == NULL) return b;
    if (b.info == NULL) return a;
    return (a.info->cod <= b.info->cod) ? a : b;
}

void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p){
    if (pilha == NULL || vetTop == NULL || num_p <= 0) {
        printf("Nao ha pilhas para intercalar.\n");
        return;
    }

    FILE *out = fopen(nome_arquivo_saida, "wb");
    if (out == NULL) {
        printf("Erro ao abrir arquivo de saida\n");
        return;
    }

    //1. numero de folhas = menor potencia de 2 que comporta as num_p particoes
    int folhas = 1;
    while (folhas < num_p) folhas *= 2;

    TPilha *arv = (TPilha *) malloc(sizeof(TPilha) * 2 * folhas);
    if (arv == NULL) {
        printf("Erro de alocacao da arvore de vencedores\n");
        fclose(out);
        return;
    }

    //2. preenche as folhas com o primeiro registro (topo) de cada pilha
    for (int i = 0; i < folhas; i++) {
        TPilha *folha = &arv[folhas + i];
        folha->info = NULL;      //HIGH VALUE
        folha->p = -1;
        if (i < num_p) {
            folha->info = pop(pilha[i], 0, &vetTop[i]); //NULL se a pilha estiver vazia
            folha->p = i;
        }
    }

    //3. monta os nos internos de baixo para cima (cada no guarda o vencedor dos filhos)
    for (int i = folhas - 1; i >= 1; i--)
        arv[i] = vencedor(arv[2 * i], arv[2 * i + 1]);

    //4. enquanto a raiz tiver registro, ele e o menor de todos
    int gravados = 0;   //posicao (em registros) do proximo registro no arquivo de saida
    while (arv[1].info != NULL) {
        TFunc *menor = arv[1].info;
        int p = arv[1].p;

        //salva_funcionario grava campo a campo (sem o padding do struct), por isso
        //posiciona com fseek usando tamanho_registro(), igual ao restante do projeto
        fseek(out, gravados * tamanho_registro(), SEEK_SET);
        salva_funcionario(menor, out);
        gravados++;
        free(menor);

        //repoe a folha da particao vencedora com o proximo registro da pilha
        int pos = folhas + p;
        arv[pos].info = pop(pilha[p], 0, &vetTop[p]); //NULL se esgotou

        //refaz somente o caminho dessa folha ate a raiz (log2 das folhas comparacoes)
        for (pos = pos / 2; pos >= 1; pos /= 2)
            arv[pos] = vencedor(arv[2 * pos], arv[2 * pos + 1]);
    }

    free(arv);
    fclose(out);
}