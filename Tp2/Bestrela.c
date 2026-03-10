#include "Bestrela.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

// Macro auxiliar para obter a chave do registro
#define Chave(reg) (reg).chave

void InicializaEstrela(TipoApEst *Arvore) {
    // Cria a primeira página como externa (folha)
    *Arvore = (TipoApEst)malloc(sizeof(TipoPaginaEstrela));
    (*Arvore)->Pt = Externa;
    (*Arvore)->UU.U1.ne = 0;
    (*Arvore)->UU.U1.prox_folha = NULL;
}

// Pesquisa na árvore B*
void PesquisaEstrela(regs *x, TipoApEst Ap, conts* performance) {
    if (Ap == NULL) {
        x->chave = -1; 
        return;
    }
    
    int i = 1;
    
    if (Ap->Pt == Interna) {
        // Busca na página interna
        while (i < Ap->UU.U0.ni && x->chave > Ap->UU.U0.ri[i - 1]) {
            performance->ncomp++;
            i++;
        }
        
        performance->ncomp++;
        if (x->chave <= Ap->UU.U0.ri[i - 1]) {
            PesquisaEstrela(x, Ap->UU.U0.pi[i - 1], performance);
        } else {
            PesquisaEstrela(x, Ap->UU.U0.pi[i], performance);
        }
        
    } else { // Página externa (folha)
        // Busca sequencial na folha
        while (i < Ap->UU.U1.ne && x->chave > Chave(Ap->UU.U1.re[i - 1])) {
            performance->ncomp++;
            i++;
        }
        
        // Verifica se encontrou
        performance->ncomp++;
        if (x->chave == Chave(Ap->UU.U1.re[i - 1])) {
            *x = Ap->UU.U1.re[i - 1];
            performance->ntranf++;
        } else {
            x->chave = -1;
        }
    }
}

// Insere registro em uma página folha
void InsereNaPaginaEstrela(TipoApEst Ap, regs Reg, conts* performance) {
    int k = Ap->UU.U1.ne;
    
    short NaoAchouPosicao = (k > 0);
    
    while (NaoAchouPosicao) {
        performance->ncomp++;
        if (Reg.chave >= Chave(Ap->UU.U1.re[k - 1])) {
            NaoAchouPosicao = false;
            break;
        }
        Ap->UU.U1.re[k] = Ap->UU.U1.re[k - 1];
        k--;
        if (k < 1) NaoAchouPosicao = false;
    }
    
    Ap->UU.U1.re[k] = Reg;
    Ap->UU.U1.ne++;
    performance->ntranf++;
}

// Insere chave/ponteiro em uma página interna
void InsereNaPaginaInterna(TipoApEst Ap, int Chave, TipoApEst ApDir, conts* performance) {
    int k = Ap->UU.U0.ni;
    
    short NaoAchouPosicao = (k > 0);
    
    while (NaoAchouPosicao) {
        performance->ncomp++;
        if (Chave >= Ap->UU.U0.ri[k - 1]) {
            NaoAchouPosicao = false;
            break;
        }
        Ap->UU.U0.ri[k] = Ap->UU.U0.ri[k - 1];
        Ap->UU.U0.pi[k + 1] = Ap->UU.U0.pi[k];
        k--;
        if (k < 1) NaoAchouPosicao = false;
    }
    
    Ap->UU.U0.ri[k] = Chave;
    Ap->UU.U0.pi[k + 1] = ApDir;
    Ap->UU.U0.ni++;
    performance->ntranf++;
}

// Função recursiva de inserção
void InsereRecEstrela(regs Reg, TipoApEst Ap, short *Cresceu, 
                     regs *RegRetorno, TipoApEst *ApRetorno, conts* performance) {
    
    if (Ap == NULL) {
        *Cresceu = true;
        *RegRetorno = Reg;
        *ApRetorno = NULL;
        return;
    }
    
    if (Ap->Pt == Interna) {
        int i = 1;
        
        while (i < Ap->UU.U0.ni && Reg.chave > Ap->UU.U0.ri[i - 1]) {
            performance->ncomp++;
            i++;
        }
        
        performance->ncomp++;
        if (Reg.chave == Ap->UU.U0.ri[i - 1]) {
            printf("Erro: Registro ja existe\n");
            *Cresceu = false;
            return;
        }
        
        performance->ncomp++;
        if (Reg.chave <= Ap->UU.U0.ri[i - 1]) i--;
        
        InsereRecEstrela(Reg, Ap->UU.U0.pi[i], Cresceu, RegRetorno, ApRetorno, performance);
        
        if (!*Cresceu) return;
        
        // Trata crescimento no nó interno
        if (Ap->UU.U0.ni < M) {
            InsereNaPaginaInterna(Ap, RegRetorno->chave, *ApRetorno, performance);
            *Cresceu = false;
        } else {
            // Overflow - split do nó interno (2/3)
            TipoApEst novaPagina = (TipoApEst)malloc(sizeof(TipoPaginaEstrela));
            if (!novaPagina) {
                printf("Erro de alocação de memória\n");
                exit(1);
            }
            novaPagina->Pt = Interna;
            novaPagina->UU.U0.ni = 0;
            novaPagina->UU.U0.pi[0] = NULL;
            performance->ntranf++;
            
            // Split 2/3 - página esquerda fica com ~67% dos elementos
            int dois_tercos = (2 * M + 2) / 3;
            
            if (i < dois_tercos) {
                InsereNaPaginaInterna(novaPagina, Ap->UU.U0.ri[M - 1], Ap->UU.U0.pi[M], performance);
                Ap->UU.U0.ni--;
                InsereNaPaginaInterna(Ap, RegRetorno->chave, *ApRetorno, performance);
            } else {
                InsereNaPaginaInterna(novaPagina, RegRetorno->chave, *ApRetorno, performance);
            }
            
            // Move elementos para a nova página
            for (int j = dois_tercos + 1; j <= M; j++) {
                InsereNaPaginaInterna(novaPagina, Ap->UU.U0.ri[j - 1], Ap->UU.U0.pi[j], performance);
            }
            
            Ap->UU.U0.ni = dois_tercos - 1;
            novaPagina->UU.U0.pi[0] = Ap->UU.U0.pi[dois_tercos];
            
            RegRetorno->chave = Ap->UU.U0.ri[dois_tercos - 1];
            *ApRetorno = novaPagina;
            *Cresceu = true;
        }
        
    } else { // Página folha - SPLIT 2/3 IMPLEMENTADO
        if (Ap->UU.U1.ne < M) {
            InsereNaPaginaEstrela(Ap, Reg, performance);
            *Cresceu = false;
        } else {
            // Overflow na folha - split 2/3 (característica da B*)
            TipoApEst novaPagina = (TipoApEst)malloc(sizeof(TipoPaginaEstrela));
            if (!novaPagina) {
                printf("Erro de alocação de memória\n");
                exit(1);
            }
            novaPagina->Pt = Externa;
            novaPagina->UU.U1.ne = 0;
            performance->ntranf++;
            
            // Split 2/3: primeira página fica com ~67% dos elementos
            // M+1 elementos totais (M na página + 1 novo)
            // Primeira página: 2*(M+1)/3 elementos
            // Segunda página: (M+1)/3 elementos
            int dois_tercos = (2 * (M + 1)) / 3;
            
            // Cria buffer temporário com todos os M+1 elementos ordenados
            regs buffer[M + 1];
            int buf_count = 0;
            int pos_inserido = -1;
            
            // Copia elementos existentes e insere o novo na posição correta
            for (int j = 0; j < M; j++) {
                if (pos_inserido == -1 && Reg.chave < Chave(Ap->UU.U1.re[j])) {
                    buffer[buf_count++] = Reg;
                    pos_inserido = buf_count - 1;
                }
                buffer[buf_count++] = Ap->UU.U1.re[j];
            }
            if (pos_inserido == -1) {
                buffer[buf_count++] = Reg;
            }
            
            // Primeira página fica com 2/3 dos elementos
            Ap->UU.U1.ne = dois_tercos;
            for (int j = 0; j < dois_tercos; j++) {
                Ap->UU.U1.re[j] = buffer[j];
            }
            
            // Segunda página fica com o restante (~1/3)
            novaPagina->UU.U1.ne = (M + 1) - dois_tercos;
            for (int j = 0; j < novaPagina->UU.U1.ne; j++) {
                novaPagina->UU.U1.re[j] = buffer[dois_tercos + j];
            }
            
            // Encadeamento de folhas
            novaPagina->UU.U1.prox_folha = Ap->UU.U1.prox_folha;
            Ap->UU.U1.prox_folha = novaPagina;
            
            // A chave que sobe é a primeira da nova página (direita)
            *RegRetorno = novaPagina->UU.U1.re[0];
            *ApRetorno = novaPagina;
            *Cresceu = true;
        }
    }
}

// Função pública de inserção
void InsereEstrela(regs Reg, TipoApEst *Ap, conts* performance) {
    short Cresceu;
    regs RegRetorno;
    TipoApEst ApRetorno;
    
    InsereRecEstrela(Reg, *Ap, &Cresceu, &RegRetorno, &ApRetorno, performance);
    
    if (Cresceu) {
        TipoApEst novaRaiz = (TipoApEst)malloc(sizeof(TipoPaginaEstrela));
        if (!novaRaiz) {
            printf("Erro de alocação de memória\n");
            exit(1);
        }
        novaRaiz->Pt = Interna;
        novaRaiz->UU.U0.ni = 1;
        novaRaiz->UU.U0.ri[0] = RegRetorno.chave;
        novaRaiz->UU.U0.pi[0] = *Ap;
        novaRaiz->UU.U0.pi[1] = ApRetorno;
        *Ap = novaRaiz;
        performance->ntranf++;
    }
}

// Função principal
bool ArvoreBEstrela(regs *registro, FILE *arq, conts performance[2]) {
    static TipoApEst arvore = NULL;
    static bool inicializada = false;
    
    conts *perf_prep = &performance[0];
    conts *perf_search = &performance[1];
    
    if (!inicializada) {
        clock_t comeco = clock();
        
        InicializaEstrela(&arvore);
        
        regs temp_reg;
        fseek(arq, 0, SEEK_SET);
        
        while (fread(&temp_reg, sizeof(regs), 1, arq) == 1) {
            InsereEstrela(temp_reg, &arvore, perf_prep);
        }
        
        clock_t final = clock();
        perf_prep->tempo = (double)(final - comeco) / CLOCKS_PER_SEC;
        inicializada = true;
    }
    
    clock_t comeco = clock();
    
    regs resultado;
    resultado.chave = registro->chave;
    
    PesquisaEstrela(&resultado, arvore, perf_search);
    
    clock_t final = clock();
    perf_search->tempo = (double)(final - comeco) / CLOCKS_PER_SEC;
    
    if (resultado.chave != -1) {
        *registro = resultado;
        return true;
    }
    
    return false;
}
