#include "arvoreB.h"
#include <stdio.h>
#include <stdlib.h>

void Inicializa(TipoApontador *Arvore) {
    *Arvore = NULL;
}


bool Pesquisa(regs *x, TipoApontador Ap, conts* performance, int medir_tempo) {
    clock_t inicio, fim;
    if (medir_tempo) {
        inicio = clock();
    }

    long i = 1;
    if (!Ap) {
        if (medir_tempo) {
            fim = clock();
            calcTemp(inicio, fim, performance);
        }
        return false;
    }

    while (i < Ap->n && x->chave > Ap->r[i - 1].chave) {
        performance->ncomp++;
        i++;
    }

    performance->ncomp++;
    if (x->chave == Ap->r[i - 1].chave) {
        *x = Ap->r[i - 1];
        performance->ntranf++;
        if (medir_tempo) {
            fim = clock();
            calcTemp(inicio, fim, performance);
        }
        return true;
    }

    bool achou;
    performance->ncomp++;
    if (x->chave < Ap->r[i - 1].chave)
        achou = Pesquisa(x, Ap->p[i - 1], performance, 0);
    else
        achou = Pesquisa(x, Ap->p[i], performance, 0);

    if (medir_tempo) {
        fim = clock();
        calcTemp(inicio, fim, performance);
    }

    return achou;
}

void InsereNaPagina(TipoApontador Ap, regs Reg, TipoApontador ApDir, conts* performance) {
    short NaoAchouPosicao;
    int k;
    k = Ap->n; NaoAchouPosicao = (k > 0);

    while (NaoAchouPosicao)
    {
        if (Reg.chave >= Ap->r[k - 1].chave)
        {
            NaoAchouPosicao = false;
            break;
        }
        Ap->r[k] = Ap->r[k - 1];
        Ap->p[k + 1] = Ap->p[k];
        k--;
        if (k < 1)
            NaoAchouPosicao = false;
    }

    Ap->r[k] = Reg;
    Ap->p[k + 1] = ApDir;
    Ap->n++;
    performance->ntranf++;
}

void Insere(regs Reg, TipoApontador *Ap, conts* performance) {
    short Cresceu;
    regs RegRetorno;
    TipoApontador ApRetorno, ApTemp;
    
    InsereRec(Reg, *Ap, &Cresceu, &RegRetorno, &ApRetorno, performance);
    
    if (Cresceu) {
        ApTemp = (TipoApontador)malloc(sizeof(TipoPagina));
        ApTemp->n = 1;
        ApTemp->r[0] = RegRetorno;
        ApTemp->p[1] = ApRetorno;
        ApTemp->p[0] = *Ap;
        *Ap = ApTemp;
        performance->ntranf++;
    }
}

void InsereRec(regs Reg, TipoApontador Ap, short *Cresceu, regs *RegRetorno, TipoApontador *ApRetorno, conts* performance) {
    long i = 1;
    long j;
    TipoApontador ApTemp;
    
    if (Ap == NULL) {
        *Cresceu = true;
        *RegRetorno = Reg;
        *ApRetorno = NULL;
        return;
    }
    
    while (i < Ap->n && Reg.chave > Ap->r[i-1].chave) {
        performance->ncomp++;
        i++;
    }
    
    performance->ncomp++;
    if (Reg.chave == Ap->r[i-1].chave) {
        printf("Erro: Registro ja existe\n");
        *Cresceu = false;
        return;
    }
    
    performance->ncomp++;
    if (Reg.chave < Ap->r[i-1].chave) i--;
    
    InsereRec(Reg, Ap->p[i], Cresceu, RegRetorno, ApRetorno, performance);
    
    if (!*Cresceu) return;
    
    if (Ap->n < MM) {
        InsereNaPagina(Ap, *RegRetorno, *ApRetorno, performance);
        *Cresceu = false;
        return;
    }
    
    // Overflow - Split da página
    ApTemp = (TipoApontador)malloc(sizeof(TipoPagina));
    ApTemp->n = 0;
    ApTemp->p[0] = NULL;
    
    int meio = MM / 2;
    
    if (i <= meio) {
        for (j = meio; j < MM; j++) {
            ApTemp->r[ApTemp->n] = Ap->r[j];
            ApTemp->p[ApTemp->n + 1] = Ap->p[j + 1];
            ApTemp->n++;
        }
        Ap->n = meio;
        InsereNaPagina(Ap, *RegRetorno, *ApRetorno, performance);
    } else {
        for (j = meio + 1; j < MM; j++) {
            ApTemp->r[ApTemp->n] = Ap->r[j];
            ApTemp->p[ApTemp->n + 1] = Ap->p[j + 1];
            ApTemp->n++;
        }
        Ap->n = meio + 1;
        InsereNaPagina(ApTemp, *RegRetorno, *ApRetorno, performance);
    }
    
    *RegRetorno = Ap->r[Ap->n - 1];
    ApTemp->p[0] = Ap->p[Ap->n];
    Ap->n--;
    *ApRetorno = ApTemp;
    performance->ntranf += 2;
}

bool ArvoreB(regs *registro, FILE *arq, conts* performance) {
    static TipoApontador arvore = NULL;
    static bool inicializada = false;
    
    if (!inicializada) {
        // Fase de preparação: construção da árvore
        clock_t comeco = clock();
        
        Inicializa(&arvore);
        
        regs temp;
        fseek(arq, 0, SEEK_SET);
        
        while (fread(&temp, sizeof(regs), 1, arq) == 1) {
            Insere(temp, &arvore, &performance[0]);
        }
        
        // Atualiza tempo de preparação
        clock_t final = clock();
        performance[0].tempo = (double)(final - comeco) / CLOCKS_PER_SEC;
        inicializada = true;
    }
    
    // Fase de pesquisa
    clock_t comeco = clock();
    
    regs regBusca = *registro;
    bool encontrado = Pesquisa(&regBusca, arvore, &performance[1], 1);
    
    if (encontrado) {
        *registro = regBusca;
    }
    
    // Atualiza tempo de pesquisa
    clock_t final = clock();
    performance[1].tempo = (double)(final - comeco) / CLOCKS_PER_SEC;
    
    return encontrado;
}
