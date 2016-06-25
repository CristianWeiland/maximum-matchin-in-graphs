#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <graphviz/cgraph.h>
#include "grafo.h"

//------------------------------------------------------------------------------
// Definições extras:
#define TAM_NOME 64
#define VIZIN_SAIDA 1
#define VIZIN_COMPL 0
#define VIZIN_ENTRA -1
#define PESO_DEFAULT 0
#define BRAN 0 // Estados (cores) de vertices
#define VERM 1
#define AZUL 2
#define FDR -1 // Fim de Rotulo

//---------------------------------------------------------------------------
// nó de lista encadeada cujo conteúdo é um void *
struct no {
  void *conteudo;
  no proximo;
};

//---------------------------------------------------------------------------
// lista encadeada

struct lista {
  unsigned int tamanho;
  int padding; // só pra evitar warning
  no primeiro;
};

//------------------------------------------------------------------------------
// Estruturas Auxiliares Criadas:

// Lista v = lista de vertices do grafo.
// Nome = nome do grafo.
// Int direcao = 1 se o grafo for direcionado, 0 caso contrario.
// Int ponderado = 1 se o grafo possui peso nas arestas, 0 caso contrario.
struct grafo {
	lista v;
	char* nome;
	int direcao;
	int ponderado;
};

//------------------------------------------------------------------------------
// Estrutura de dados que representa um vértice do grafo.
// Cada vértice tem um nome, que é uma "string"
// Nome = nome do vertice
// Lista saida e entrada sao listas para arestas de saida e de entrada.
// Em grafos nao direcionados, as arestas soh sao inseridas uma vez em cada vertice!
// Por exemplo, uma aresta a--b aparece na lista de saida do vertice a e na lista de
// entrada do vertice b..
struct vertice {
	char* nome;
	lista saida, entrada;
    int *rotulo;
    int estado, atributo, coberto, visitado, na_arvore, padding;
};

//------------------------------------------------------------------------------
// Cada aresta aparecera tanto no vertice de entrada como no de saida.
// Por exemplo, caso existam dois vertices a e b e um grafo nao direcionado
// que possua a aresta a--b, tanto o vertice a quanto o vertice b apontarao
// para a aresta a--b. O mesmo vale para grafos direcionados.
// Vertice vs = vertice de saida.
// Vertice vc = vertice de chegada.
// Peso = peso da aresta (0 é o default).
typedef struct aresta *aresta;
struct aresta {
	vertice vs, vc;
	long int peso;
    int coberta, padding;
};

//------------------------------------------------------------------------------
// Protótipos de Funções Auxiliares Criadas:

//------------------------------------------------------------------------------
// Alloca memoria para um grafo
grafo constroi_grafo(void);

//------------------------------------------------------------------------------
// Alloca memoria para um vertice
vertice constroi_vertice(void);

//------------------------------------------------------------------------------
// Alloca memoria para uma aresta
aresta constroi_aresta(void);

//------------------------------------------------------------------------------
// Dado um vertice, procura um apontador pra aresta a e remove ele
void remove_duplicada(void* a, vertice v);

//------------------------------------------------------------------------------
// Cria um vertice, atribui a ele um nome e insere na lista de vertices do grafo
vertice insere_vertice(grafo g, char* nome);

//------------------------------------------------------------------------------
// Cria uma aresta com atributos vs, vc e peso sendo, respectivamente,
// saida, chegada e peso, e insere nas 2 listas (do vertice de entrada e do de saida).
aresta insere_aresta(vertice saida, vertice chegada, long int peso);

//------------------------------------------------------------------------------
// Cria uma copia da aresta e retorna um apontador para a nova aresta
aresta copia_aresta(aresta a, grafo g);

//------------------------------------------------------------------------------
// Cria uma copia do vertice e retorna um apontador para o novo vertice
vertice copia_vertice(vertice v);

//------------------------------------------------------------------------------
// Faz uma copia de um grafo, mas nao insere elementos que aparecem na lista
// de vertices excecoes, tambem garantindo que as arestas que tinham uma ponta
// em algum desses vertices tambem nao sao inseridas.
grafo copia_subgrafo(grafo g, lista excecoes);

//------------------------------------------------------------------------------
// Desalloca memoria de uma aresta (argumento eh void* para a funcao poder ser
// usada como parametro da funcao destroi_lista)
int destroi_aresta(void *v);

//------------------------------------------------------------------------------
// Desalloca memoria de um vertice (argumento eh void* para a funcao poder ser
// usada como parametro da funcao destroi_lista)
int destroi_vertice(void *v);

//------------------------------------------------------------------------------
// Procura na lista de vertices do grafo um vertice com o nome do parametro
// retorna NULL em caso de erro ou caso nao ache o vertice
// retorna o vertice caso ache ele
vertice procura_vertice(grafo g, char* nome);

//------------------------------------------------------------------------------
// Devolve 1, se o vertice v2 é adjacente (ligado por uma aresta) a v, ou
//         0, caso contrário
int adjacente(vertice v, vertice v2);

//------------------------------------------------------------------------------
// Devolve 1, se o vertice é uma clique em g, ou
//         0, caso contrário
//
// Um vertice V de um conjunto de vertices C de um grafo G é uma clique em G
// se todo vértice no conjunto C é vizinho de V em G
int vertice_clique(vertice v, lista l);

//------------------------------------------------------------------------------
// percorre a lista e retorna 1 se o parametro conteudo estiver dentro dessa
// lista, retorna 0 caso contrario.
int na_lista(lista l, void* conteudo);

//------------------------------------------------------------------------------
// Retorna a lista de vertices de um grafo g
lista vertices_grafo(grafo g);

//------------------------------------------------------------------------------
// Imprime o rotulo de um vertice v.
void imprime_rotulo(vertice v);

//------------------------------------------------------------------------------
// Imprime um vertice
void imprime_vertice(void* param);

//------------------------------------------------------------------------------
// Imprime uma lista de vertices.
void imprime_lista_vertices(lista l);

//------------------------------------------------------------------------------
// Imprime uma lista de arestas.
void imprime_lista_arestas(lista l);

//------------------------------------------------------------------------------
// Faz a comparação lexicográfica entre dois vetores de inteiros.
// Devolve 1, se b > a lexicograficamente.
//         0, caso contrário.
int lexcmp(int *a, int *b);

//------------------------------------------------------------------------------
// Procura o vértice de maior rótulo em uma lista de vértices.
no v_rotulo_maximo(lista l);

//------------------------------------------------------------------------------
// Efetua a busca em largura lexicografica para um vértice específico e adiciona
// o resultado na lista ordem.
lista busca_largura_lexicografica_vertice(grafo g, vertice r, lista ordem);


//------------------------------------------------------------------------------
// Retorna 1 se a aresta estiver coberta, 0 caso contrario.
int aresta_coberta(aresta a);

//------------------------------------------------------------------------------
// Copia os vertices cobertos de g2 para g1, criando novos vertices no processo.
void copia_vertices(grafo g1, grafo g2);

//------------------------------------------------------------------------------
// Copia as arestas cobertas de g2 para g1, criando arestas entre os vertices
// de g1.
void copia_arestas_cobertas(grafo g1, grafo g2);

//------------------------------------------------------------------------------
// Efetua a operação xor numa lista, basicamente fazendo:
// - Se a aresta estava coberta, ela fica não coberta.
// - Se a aresta não estava coberta, ela fica coberta.
void xor(lista l);

//------------------------------------------------------------------------------
// Função recursiva que busca um camminho aumentante. Caso eles eja encontrado,
// retorna 1 e o caminho aumentante é representado por uma lista de arestas em l.
int busca_caminho(vertice v, lista l, int last);

//------------------------------------------------------------------------------
// Procura um caminho aumentante (se existir) no grafo g.
lista caminho_aumentante(grafo g);

//------------------------------------------------------------------------------
// Implementação das Funções:
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Funções do Lista.c

//---------------------------------------------------------------------------
// devolve o número de nós da lista l

unsigned int tamanho_lista(lista l) { return l->tamanho; }

// Imprime uma lista
void imprime_lista(lista l,void imprime_elemento(void *));

void imprime_lista(lista l,void imprime_elemento(void *)) {
    no elem;
    for(elem = l->primeiro; elem; elem = elem->proximo) {
        imprime_elemento(elem->conteudo);
    }
    return ;
}

//---------------------------------------------------------------------------
// devolve o primeiro nó da lista l,
//      ou NULL, se l é vazia

no primeiro_no(lista l) { return l->primeiro; }

//---------------------------------------------------------------------------
// devolve o conteúdo do nó n
//      ou NULL se n = NULL 

void *conteudo(no n) { return n->conteudo; }

//---------------------------------------------------------------------------
// devolve o sucessor do nó n,
//      ou NULL, se n for o último nó da lista

no proximo_no(no n) { return n->proximo; }

//---------------------------------------------------------------------------
// cria uma lista vazia e a devolve
//
// devolve NULL em caso de falha

lista constroi_lista(void) {

  lista l = malloc(sizeof(struct lista));

  if ( ! l ) 
    return NULL;

  l->primeiro = NULL;
  l->tamanho = 0;

  return l;
}
//---------------------------------------------------------------------------
// desaloca a lista l e todos os seus nós
// 
// se destroi != NULL invoca
//
//     destroi(conteudo(n)) 
//
// para cada nó n da lista. 
//
// devolve 1 em caso de sucesso,
//      ou 0 em caso de falha

int destroi_lista(lista l, int destroi(void *)) {

  no p;
  int ok=1;

  while ( (p = primeiro_no(l)) ) {

    l->primeiro = proximo_no(p);

    if ( destroi )
      ok &= destroi(conteudo(p));

    free(p);
  }

  free(l);

  return ok;
}
//---------------------------------------------------------------------------
// insere um novo nó na lista l cujo conteúdo é p
//
// devolve o no recém-criado
//      ou NULL em caso de falha

no insere_lista(void *conteudo, lista l) {

  no novo = malloc(sizeof(struct no));

  if ( ! novo )
    return NULL;

  novo->conteudo = conteudo;
  novo->proximo = primeiro_no(l);
  ++l->tamanho;

  return l->primeiro = novo;
}

//------------------------------------------------------------------------------
// Remove um nó de uma lista e executa a função destroi no conteúdo de cada nó.

int remove_no(struct lista *l, struct no *rno, int destroi(void *)) {
    int r = 1;
    if (l->primeiro == rno) {
        l->primeiro = rno->proximo;
        if (destroi != NULL) {
            r = destroi(conteudo(rno));
        }
        free(rno);
        l->tamanho--;
        return r;
    }
    for (no n = primeiro_no(l); n->proximo; n = proximo_no(n)) {
        if (n->proximo == rno) {
            n->proximo = rno->proximo;
            if (destroi != NULL) {
                r = destroi(conteudo(rno));
            }
            free(rno);
            l->tamanho--;
            return r;
        }
    }
    return 0;
}







//------------------------------------------------------------------------------
// Funções que não são mais do lista.c

int na_lista(lista l, void* content) {
    no elem;
    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        if(content == conteudo(elem))
            return 1;
    }
    return 0;
}

grafo constroi_grafo(void) {
    grafo g;
    g = (grafo) malloc(sizeof(struct grafo));
    if(g == NULL) {
        perror("(constroi_grafo) Erro ao allocar memoria para o grafo.");
        return NULL;
    }
    g->v = constroi_lista();
    g->nome = malloc(sizeof(char) * TAM_NOME);
    if(g->nome == NULL) {
        perror("(constroi_grafo) Erro ao allocar memoria para nome.");
        return NULL;
    }
    g->direcao = 0;
    g->ponderado = 0;
    return g;
}

vertice constroi_vertice(void) {
    vertice v = malloc(sizeof(struct vertice));
    if(v == NULL) {
        perror("(constroi_vertice) Erro ao allocar memoria para o vertice.");
        return NULL;
    }
    if(!(v->saida = constroi_lista()))
        puts("Erro ao construir lista de saida.");
    if(!(v->entrada = constroi_lista()))
        puts("Erro ao construir lista de saida.");
    v->nome = malloc(sizeof(char) * TAM_NOME);
    if(v->nome == NULL) {
        perror("(constroi_vertice) Erro ao allocar memoria para nome.");
        return NULL;
    }
    return v;
}

aresta constroi_aresta(void) {
    aresta a = malloc(sizeof(struct aresta));
    if(a == NULL) {
        perror("(constroi_aresta) Erro ao allocar memoria para aresta.");
        return NULL;
    }
    a->peso = PESO_DEFAULT;
    return a;
}

void imprime_lista_arestas(lista l) {
    no elem;
    aresta a;
    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        printf("Aresta %s -- %s\n", a->vs->nome, a->vc->nome);
    }
    return ;
}

int destroi_aresta(void* param) {
    // Nao damos free em a->vs ou a->vc, porque sao os vertices. Eles estao sendo apontados por outro lugar.
    // Destroi_aresta soh destroi a propria aresta.
    aresta a = (aresta) param;
    if(a == NULL) return 1; // Jah foi destruido
    free(a);
    return 1;
}

int destroi_vertice(void* param) { // Soh funciona pra ser chamado dentro de destroi_grafo.
    vertice v = (vertice) param;
    if(v == NULL) {
        return 1; // Jah foi destruido
    }
    if(v->nome) {
        free(v->nome);
    }
    if(!destroi_lista(v->saida, destroi_aresta)) {
        perror("(destroi_vertice) Erro ao destruir lista.");
        return 0;
    }
    if(!destroi_lista(v->entrada, NULL)) {
        perror("(destroi_vertice) Erro ao destruir lista.");
        return 0;
    }
    free(v);
    return 1;
}

int destroi_grafo(void* param) {
    grafo g = (grafo) param;
    if(g == NULL)
        return 1;
    if(g->nome) {
        free(g->nome);
    }

    if(!destroi_lista(g->v, destroi_vertice)) {
        perror("(destroi_grafo) Erro ao destruir grafo.");
        return 0;
    }

    free(g);
    return 1;
}

vertice insere_vertice(grafo g, char* nome) {
    void* content = constroi_vertice();
    no novo = insere_lista(content, g->v);
    if(novo == NULL) {
        perror("(insere_vertice) Erro ao inserir vertice no grafo.");
        return NULL;
    }
    vertice v = conteudo(novo);
    v->nome = strncpy(v->nome,nome,TAM_NOME);
    return v;
}

aresta insere_aresta(vertice saida, vertice chegada, long int peso) {
    aresta a = constroi_aresta();
    a->vs = saida;
    a->vc = chegada;
    a->peso = peso;
    // Insere a aresta na lista de arestas do vertice de saida
    if(insere_lista((void*)a, saida->saida) == NULL) {
        perror("(insere_aresta) Erro ao inserir aresta no vertice.");
        return NULL;
    }
    // Insere a aresta na lista de arestas do vertice de chegada
    if(insere_lista((void*)a, chegada->entrada) == NULL) {
        perror("(insere_aresta) Erro ao inserir aresta no vertice.");
        return NULL;
    }
    return a;
}

vertice procura_vertice(grafo g, char* nome) {
    vertice v;
    lista vertices = g->v;
    no elem;
    for(elem = primeiro_no(vertices); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        if(strcmp(v->nome, nome) == 0)
            return v;
    }
    return NULL;
}

aresta copia_aresta(aresta a, grafo g) {
    aresta copia = constroi_aresta();
    // Eh necessario usar procura_vertice porque nao queremos copiar o apontador do grafo base,
    // mas sim apontar para o vertice adicionado na copia do grafo.
    copia->vs = procura_vertice(g, a->vs->nome);
    copia->vc = procura_vertice(g, a->vc->nome);
    if( (void*)copia->vs == (void*)a->vs->nome || copia->vs == NULL) {
        perror("O vertice encontrado eh o do grafo errado!");
        return NULL;
    }
    if( (void*)copia->vc == (void*)a->vc->nome || copia->vc == NULL) {
        perror("O vertice encontrado eh o do grafo errado!");
        return NULL;
    }
    copia->peso = a->peso;
    return copia;
}

vertice copia_vertice(vertice v) {
    vertice copia;

    copia = constroi_vertice();
    copia->nome = strncpy(copia->nome,v->nome,TAM_NOME);

    return copia;
}

grafo copia_grafo(grafo g) {
    no elem, childElem;
    void* content;
    vertice v;

    grafo g2 = constroi_grafo();

    g2->ponderado = g->ponderado;
    g2->direcao = g->direcao;
    g2->nome = strncpy(g2->nome,g->nome,TAM_NOME);

    // Copia vertices
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        content = (void*) copia_vertice(conteudo(elem));
        if(insere_lista(content, g2->v) == NULL) {
            perror("(copia_grafo) Erro ao inserir vertice no grafo copia.");
            return NULL;
        }
    }

    // Percorre todos os vertices originais...
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = conteudo(elem);
        // ... pra percorrer todas as arestas
        for(childElem = primeiro_no(v->saida); childElem; childElem = proximo_no(childElem)) {
            // cria uma aresta nova e insere no grafo novo
            content = (void*) copia_aresta(conteudo(childElem), g2);
            if(insere_lista(content, ((aresta)content)->vc->saida) == NULL) {
                perror("(copia_grafo) Erro ao inserir aresta no vertice copia.");
                return NULL;
            }
            if(insere_lista(content, ((aresta)content)->vs->entrada) == NULL) {
                perror("(copia_grafo) Erro ao inserir aresta no vertice copia.");
                return NULL;
            }
        }
    }

    return g2;
}

grafo copia_subgrafo(grafo g, lista excecoes) {
    no elem, childElem;
    void* content;
    aresta a;
    vertice v;

    grafo g2 = constroi_grafo();

    g2->ponderado = g->ponderado;
    g2->direcao = g->direcao;
    g2->nome = strncpy(g2->nome,g->nome,TAM_NOME);

    // Copia vertices
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        if(na_lista(excecoes, conteudo(elem))) {
            continue;
        }
        content = (void*) copia_vertice(conteudo(elem));
        if(insere_lista(content, g2->v) == NULL) {
            perror("(copia_grafo) Erro ao inserir vertice no grafo copia.");
            return NULL;
        }
    }

    // Vou percorrer todas as arestas na seguinte forma: percorre todos os vertices, e todas as listas de entrada de todos os vertices
    // Percorre todos os vertices originais...
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = conteudo(elem);
        // ... pra percorrer todas as arestas
        for(childElem = primeiro_no(v->entrada); childElem; childElem = proximo_no(childElem)) {
            a = conteudo(childElem);
            if(na_lista(excecoes, a->vs) || na_lista(excecoes, a->vc)) {
                continue;
            }
            // Cria uma aresta nova e insere no grafo novo
            content = (void*) copia_aresta(conteudo(childElem), g2);
            if(insere_lista(content, ((aresta)content)->vc->entrada) == NULL) {
                perror("(copia_grafo) Erro ao inserir aresta no vertice copia.");
                return NULL;
            }
            if(insere_lista(content, ((aresta)content)->vs->saida) == NULL) {
                perror("(copia_grafo) Erro ao inserir aresta no vertice copia.");
                return NULL;
            }
        }
    }

    return g2;
}

lista vertices_grafo(grafo g) {
    return g->v;
}

char* nome_grafo(grafo g) {
    return g->nome;
}

int direcionado(grafo g) {
    return g->direcao;
}

int ponderado(grafo g) {
    return g->ponderado;
}

unsigned int grau(vertice v, int direcao, grafo g) {
    if(g == NULL) {
        return 0;
    }
    no elem;
    aresta a;
    unsigned int count = 0;
    switch (direcao) {
        case -1:
            for(elem = primeiro_no(v->entrada); elem; elem = proximo_no(elem)) {
                a = (aresta) conteudo(elem);
                if(a->vs == v)
                    ++count;
            }
            return count;
            break;
        case 0:
            return (unsigned int) 2 * tamanho_lista(v->entrada);
            break;
        case 1:
            for(elem = primeiro_no(v->entrada); elem; elem = proximo_no(elem)) {
                a = (aresta) conteudo(elem);
                if(a->vc == v)
                    ++count;
            }
            return count;
            break;
    }
    return count;
}

unsigned int n_vertices(grafo g) {
    return tamanho_lista(g->v);
}

unsigned int n_arestas(grafo g) {
    unsigned int count = 0;
    no elem;
    vertice v;
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        count += tamanho_lista(v->entrada);
    }
    return count;
}

lista vizinhanca(vertice v, int direcao, grafo g) {
    if(g == NULL) {
        return NULL;
    }
    lista l = constroi_lista();
    no elem;
    aresta a;

    if(direcao >= 0) {
        for(elem = primeiro_no(v->saida); elem; elem = proximo_no(elem)) {
            a = (aresta) conteudo(elem);
            if(insere_lista((void*)a->vc, l) == NULL) {
                perror("(vizinhanca) Erro ao inserir vertice na lista de vizinhanca de saida");
                return NULL;
            }
        }
    }
    if(direcao <= 0) {
        for(elem = primeiro_no(v->entrada); elem; elem = proximo_no(elem)) {
            a = (aresta) conteudo(elem);
            if(insere_lista((void*)a->vs, l) == NULL) {
                perror("(vizinhanca) Erro ao inserir vertice na lista de vizinhanca de saida");
                return NULL;
            }
        }
    }
    return l;
}

inline int adjacente(vertice v, vertice v2) {
    no elem;
    aresta a;

    if(v2 == v)
        return 0;

    // Procura por todas as arestas de v se o vertice v2 eh adjacente.
    for(elem = primeiro_no(v->entrada); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        if(a->vs == v2 || a->vc == v2) { // CONFERIR DEPOIS! acho que dah pra comparar soh com vs aqui, e vc ali na frente.
            // Um dos vertices eh igual a v2 e o outro tem que ser v, portanto, eh adjacente
            return 1;
        }
    }
    for(elem = primeiro_no(v->saida); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        if(a->vs == v2 || a->vc == v2) {
            // Um dos vertices eh igual a v2 e o outro tem que ser v, portanto, eh adjacente
            return 1;
        }
    }
    return 0;
}

int vertice_clique(vertice v, lista l) {
    no elem;
    vertice v_aux;

    // Estou descobrindo se o vertice eh um clique. Para isso, preciso
    // percorrer todos os vertices da lista e ver se eles estao ligados
    // ao vertice escolhido por uma aresta.
    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        v_aux = (vertice) conteudo(elem);
        if(v == v_aux)
            continue;
        if(!adjacente(v, v_aux)) {
            return 0;
        }
    }

    return 1;
}

int clique(lista l, grafo g) {
    if(!g)
        return -1;
    no elem;
    vertice v;

    // Percorre todos os elementos vendo se eles sao cliques.
    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        if(!vertice_clique(v,l)) {
            return 0;
        }
    }

    return 1;
}

int simplicial(vertice v, grafo g) {
    if(g->direcao)
        return 0;

    lista l = vizinhanca(v,VIZIN_COMPL,g);
    if(!l) {
        perror("(simplicial) Nao foi possivel encontrar a vizinhanca do vertice.");
        return 0;
    }
    if(tamanho_lista(l) == 0) {
    // Vizinhanca eh o conjunto vazio. O vertice eh simplicial.
        return 1;
    }

    imprime_lista_vertices(l);

    int retVal = clique(l,g);
    destroi_lista(l, NULL);

    return retVal;
}

int cordal(grafo g) {
    if(g->direcao) // Grafos direcionados nao sao cordais
        return 0;

    lista l = busca_largura_lexicografica(g);
    int ret = ordem_perfeita_eliminacao(l,g);
    destroi_lista(l, NULL);
    return ret;
}

int lexcmp(int *a, int *b) {
/* Retorna positivo se b > a */
    int i = 0;
    while (a[i] != FDR && b[i] != FDR) {
        if(a[i] - b[i] == 0) {
            ++i;
        } else { // b > a, retorna positivo.
            return b[i] - a[i];
        }
    }
    if(a[i] == FDR) {
        if(b[i] == FDR) {
            return 0;
        } else {
            return 1;
        }
    }
    return -1;
}

no v_rotulo_maximo(lista l) {
    if(l == NULL || tamanho_lista(l) == 0)
        return NULL;

    no elem, maior;
    vertice v, v_max;

    if(!(elem = primeiro_no(l))) {
        puts("Lista vazia e tamanho != 0. Erro?"); // Em todo caso, o maior elemento eh nulo.
        return NULL;
    }
    maior = elem;
    v_max = (vertice) conteudo(maior);
    for(; elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        if(lexcmp(v_max->rotulo, v->rotulo) > 0) {
            maior = elem;
            v_max = v;
        }
    }
    return maior;
}

lista busca_largura_lexicografica(grafo g) {
    no elem;
    vertice v;
    unsigned int tamanho;
    // Para cada v em V(G)
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        // Inicialize o rótulo de v com {}
        v = (vertice) conteudo(elem);
        tamanho = tamanho_lista(v->entrada) + tamanho_lista(v->saida) + 1;
        v->rotulo = malloc(tamanho * sizeof(int));
        v->rotulo[0] = FDR;
        v->estado = BRAN;
        v->atributo = 0; // Atributo, nesta funcao, equivale ao tamanho do rotulo.
    }

    // V <-- V(G)
    // Nao vou colocar todo o V(G) de uma vez soh. Vou colocando aos poucos pelas vizinhanças.
    // Isso evita que eu tenha que comparar com vertices de outros componentes e que eu tenha
    // que comparar com vertices de rotulo vazio.
    // De vez em quando tenho que inserir um vertice, que eh um vertice de um componente que nao
    // foi processado ainda. Pra isso, usa o atributo estado (estado == 0, nao foi processado.
    // estado == 1, foi processado).
    lista ordem = constroi_lista();

    // Elem vai iterar por todo V(G). Se o vertice nao foi processado, processa ele.
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        if(v->estado == BRAN) { // Se v->estado eh branco, nao percorri nenhum vertice desse componente.
            // A lista ordem eh a que vai conter a ordem perfeita.
            ordem = busca_largura_lexicografica_vertice(g, v, ordem);
        }
    }

    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        free(v->rotulo);
    }
    return ordem;
}

lista busca_largura_lexicografica_vertice(grafo g, vertice r, lista ordem) {
    no elem, elem2;
    vertice v, w;
    lista l, V;
    aresta aux;

    unsigned int tamLista = tamanho_lista(g->v);

    // V <- fila vazia
    V = constroi_lista();
    // Processe r
    // enfile r em V
    insere_lista((void *) r, V);
    // r.estado <- 1
    r->estado = VERM;
    // Enquanto V nao esta vazia
    while(primeiro_no(V)) {
        tamLista--; // Equivale ao valor do rotulo.
        // Desenfile um vertice v de V, sendo ele o de maior rotulo
        elem = v_rotulo_maximo(V);
        v = (vertice) conteudo(elem);
        elem2 = insere_lista(v, ordem);
        remove_no(V, elem, NULL);
        // Para cada w E vizinhanca(v) em G
        l = v->saida;

        // Vou percorrer a lista de arestas de saida, portanto, quero w como vertice de chegada da aresta.
        for( elem2 = primeiro_no(l); elem2; elem2 = proximo_no(elem2) ) {
            aux = (aresta) conteudo(elem2);
            w = (vertice) aux->vc;
            // Se w.estado = 1
            if(w->estado == VERM) {
                // adiciona_rotulo
                w->rotulo[w->atributo] = (int) tamLista;
                w->atributo++;
                w->rotulo[w->atributo] = FDR;
                // Processe { v, w }
            // Senao, se w.estado = 0
            } else if(w->estado == BRAN) {
                // adiciona_rotulo
                w->rotulo[w->atributo] = (int) tamLista;
                w->atributo++;
                w->rotulo[w->atributo] = FDR;
                // Processe w e { v, w }
                // w.pai <- v
                // enfile w em V
                insere_lista(w, V);
                // w.estado <- 1
                w->estado = VERM;
            }
        }
        l = v->entrada;
        // Agora vou percorrer a lista de arestas de entrada.
        for( elem2 = primeiro_no(l); elem2; elem2 = proximo_no(elem2) ) {
            aux = (aresta) conteudo(elem2);
            w = (vertice) aux->vs;
            if(w->estado == VERM) {
                w->rotulo[w->atributo] = (int) tamLista;
                w->atributo++;
                w->rotulo[w->atributo] = FDR;
            } else if(w->estado == BRAN) {
                w->rotulo[w->atributo] = (int) tamLista;
                w->atributo++;
                w->rotulo[w->atributo] = FDR;
                insere_lista(w, V);
                w->estado = VERM;
            }
        }
        // v.estado <- 2
        v->estado = AZUL;
    }
    destroi_lista(V, NULL);
    return ordem;
}

void imprime_rotulo(vertice v) {
    printf("Imprimindo rotulo de %s\n", v->nome);
    while(*(v->rotulo) != FDR) {
        printf("%d", *(v->rotulo));
        v->rotulo++;
    }
    puts("");
}

void imprime_lista_vertices(lista l) {
    no elem;
    vertice v;
    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        puts(v->nome);
    }
}

int ordem_perfeita_eliminacao(lista l, grafo g) {
    no elem, elem2;
    vertice v, w, u;
    aresta a;
    int i, tam_vizinh, cont;

    // Percorre todos os elementos inicializando estado e atributo.
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        v->estado = BRAN; // Tá no grafo
        v->atributo = -1; // Valor inicial
    }

    for(i=0, elem = primeiro_no(l); elem; elem = proximo_no(elem), ++i) {
        cont = tam_vizinh = 0;
        v = (vertice) conteudo(elem);

        // Marca elementos da vizinhança de v com o valor i e conta tamanho da vizinhanca.
        for(elem2 = primeiro_no(v->entrada); elem2; elem2 = proximo_no(elem2)) {
            a = (aresta) conteudo(elem2);
            u = a->vs;
            u->atributo = i;
            if(u->estado == BRAN) // Vertice nao foi removido do grafo ainda.
                tam_vizinh++;
        }
        for(elem2 = primeiro_no(v->saida); elem2; elem2 = proximo_no(elem2)) {
            a = (aresta) conteudo(elem2);
            u = a->vc;
            u->atributo = i;
            if(u->estado == BRAN)
                tam_vizinh++;
        }

        // Percorre a OPE procurando o proximo vertice vizinho de v.
        for(elem2 = elem; elem2; elem2 = proximo_no(elem2)) {
            w = (vertice) conteudo(elem2);
            if(w->atributo == i) {
            // Achei um vizinho de v.
                break;
            }
        }

        // Agora que conheço w, preciso saber se ele compartilha os vizinhos de v.
        // Para isso, vou percorrer a vizinhança de w e contar quantos vertices
        // vizinhos de w tambem sao vizinhos de v (eu sei que um vertice eh
        // vizinho de v se o seu atributo é i). Se o numero de vertices vizinhos de
        // w e v for igual a |vizinh(V)|-1 (-1 porque exclui o proprio w), deu ok.
        for(elem2 = primeiro_no(w->entrada); elem2; elem2 = proximo_no(elem2)) {
            a = (aresta) conteudo(elem2);
            u = a->vs;
            if(u->atributo == i) {
                cont++;
            }
        }
        for(elem2 = primeiro_no(w->saida); elem2; elem2 = proximo_no(elem2)) {
            a = (aresta) conteudo(elem2);
            u = a->vc;
            if(u->atributo == i) {
                cont++;
            }
        }

        // -1 pra desconsiderar o proprio w.
        if(cont < tam_vizinh-1) { // A vizinhança de v nao ta contida em w.
            return 0;
        }

        v->estado = AZUL; // 'Remove' elemento do grafo
    }

    return 1;
}

inline int aresta_coberta(aresta a) {
    return a->coberta;
}

void copia_vertices(grafo g1, grafo g2) {
    /* Versão criando vértices novos (pois devo mudar a lista de vizinhança dele).
       Copia os vértices de g2 para g1 (g1 é um grafo vazio no início) */
    no elem;
    vertice v;

    for(elem = primeiro_no(g2->v); elem; elem = proximo_no(elem)) {
        // Cria um novo vertice.
        v = (vertice) conteudo(elem);
        if(v->coberto)
            insere_vertice(g1, v->nome );
    }
}

void copia_arestas_cobertas(grafo g1, grafo g2) {
    /* Copia arestas de g2 cobertas pelo emparalhamento para g1.
     * A ideia é parecida com a leitura do grafo: eu leio um vértice v
     * do grafo g2, que é o grafo que tem as arestas, e percorro a sua
     * vizinhança. Como os vértices do grafo g1 são diferentes dos vértices
     * de g2, eu tenho que procurar o semelhante a v em g1, que vou salvar
     * em v_saida. Depois, percorre a vizinhança de v (porque ele tem as
     * arestas) e acha o vértice de chegada. Com esse vertice de chegada
     * (a->vc), pego o semelhante a ele em g1, que salvo em v_chegada.
     * Depois, insiro em g1. */
    no elem_v, elem_a;
    vertice v, v_saida, v_chegada;
    aresta a;

    for(elem_v = primeiro_no(g2->v); elem_v; elem_v = proximo_no(elem_v)) {
        v = (vertice) conteudo(elem_v);
        v_saida = procura_vertice(g1, v->nome);
        for(elem_a = primeiro_no(v->saida); elem_a; elem_a = proximo_no(elem_a)) {
            a = (aresta) conteudo(elem_a);
            if(aresta_coberta(a)) {
                v_chegada = procura_vertice(g1, a->vc->nome);
                insere_aresta(v_saida, v_chegada, a->peso);
            }
        }
    }
}

void xor(lista l) {
/* Usando l como lista de arestas */
    aresta a;
    no elem;

    for(elem = primeiro_no(l); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        a->vc->coberto = 1;
        a->vs->coberto = 1;
        a->coberta = !a->coberta;
    }
}

int busca_caminho(vertice v, lista l, int last) {
    /* essa função é chamada pela função que tenta achar um caminho aumentante pra
     * cada vértice não coberto (e retorna assim que achar) e last é inicialmente 1,
     * pois a primeira aresta (que tenho que achar) será 0 (não coberta) */

    if (!v->coberto && !v->visitado) {
        return TRUE;
    }

    no elem;
    aresta a;
    vertice w;

    v->visitado = 1;

    for(elem = primeiro_no(v->saida); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        if(a->coberta != last) {
            w = a->vc; // w = vizinho do vértice
            if(!w->visitado && busca_caminho(w, l, !last)) {
                insere_lista(a, l);
                return TRUE;
            }
        }
    }
    for(elem = primeiro_no(v->entrada); elem; elem = proximo_no(elem)) {
        a = (aresta) conteudo(elem);
        if(a->coberta != last) {
            w = a->vs;
            if(!w->visitado && busca_caminho(w, l, !last)) {
                insere_lista(a, l);
                return TRUE;
            }
        }
    }

    return FALSE;
}

lista caminho_aumentante(grafo g) {
    no elem_v, elem_aux;
    vertice v;
    lista l;

    l = constroi_lista();

    for(elem_v = primeiro_no(g->v); elem_v; elem_v = proximo_no(elem_v)) {
        v = (vertice) conteudo(elem_v);
        v->visitado = 0;
    }

    for(elem_v = primeiro_no(g->v); elem_v; elem_v = proximo_no(elem_v)) {


        for(elem_aux = primeiro_no(g->v); elem_aux; elem_aux = proximo_no(elem_aux)) {
            v = (vertice) conteudo(elem_aux);
            v->visitado = 0;
        }

        v = (vertice) conteudo(elem_v);
        v->visitado = 1;

        if(!v->coberto) {
            if(busca_caminho(v, l, 1)) {
                if(primeiro_no(l)) { // Lista nao ta vazia.
                    return l;
                }
            }
        }
    }
    destroi_lista(l, NULL);
    return NULL;
}

grafo emparelhamento_maximo(grafo g) {
    lista l;
    grafo e;
    no elem_v, elem_a;
    vertice v;
    aresta a;

    for(elem_v = primeiro_no(g->v); elem_v; elem_v = proximo_no(elem_v)) {
        v = (vertice) conteudo(elem_v);
        v->coberto = 0;
        for(elem_a = primeiro_no(v->saida); elem_a; elem_a = proximo_no(elem_a)) {
            a = (aresta) conteudo(elem_a);
            a->coberta = 0;
        }
    }

    while((l = caminho_aumentante(g)) != NULL) {
        xor(l);
        destroi_lista(l, NULL); // Nao destroi as arestas porque elas ainda fazem parte do grafo g.
    }

    e = constroi_grafo();
    strcpy(e->nome, "Max Matching");
    copia_vertices(e,g);
    copia_arestas_cobertas(e,g);
    return e;
}

grafo le_grafo(FILE *input) {
    Agraph_t *g = agread(input, NULL);
    Agnode_t *node;
    Agedge_t *a;
    char* aux;
    char* attr = malloc(sizeof(char) * TAM_NOME);
    strcpy(attr,"peso");
    long int peso = PESO_DEFAULT;
    int v_alterado = 0;

    grafo g2 = constroi_grafo();

    g2->direcao = agisdirected(g);
    // Nao da pra fazer g2->nome apontar pra agnameof(g), porque ele vai apontar pro nome da estrutura de grafo
    // da biblioteca, que vai ser desalocada no final desta função.
    strncpy(g2->nome, agnameof(g), TAM_NOME);

    vertice v, v_aux;

    // Insere todos os vertices.
    for(node=agfstnode(g); node; node=agnxtnode(g,node)) {
        v = insere_vertice(g2, agnameof(node));
        if(v == NULL) {
            perror("(le_grafo) Erro ao inserir vertice no grafo.");
            return NULL;
        }
    }

    // Percorre todos os vertices para inserir todas as arestas.
    for(node=agfstnode(g); node; node=agnxtnode(g,node)) {
        v = procura_vertice(g2, agnameof(node));
        if(v == NULL) {
            perror("(le_grafo) Vertice nao encontrado.");
            return NULL;
        }
        // Percorre todas as arestas da estrutura, procurando os vertices de entrada (o vertice de saida eh v)
        // e insere na nossa estrutura, ou seja, na lista de vertices do grafo g2.
        for(a = agfstedge(g,node); a; a=agnxtedge(g,a,node)) {
            // Procura o vertice de chegada da aresta.

            if(node == aghead(a)) {
                v_aux = v;
                aux = agnameof(agtail(a));
                v = procura_vertice(g2, aux);
                v_alterado = 1;
                if(v == NULL) {
                    perror("(le_grafo) Vertice nao encontrado.");
                    return NULL;
                }
                char* aux2 = agget((void*)a,attr);

                if(aux2 != NULL)
                    peso = atol(aux2);
                else
                    peso = 0;

                if(peso != PESO_DEFAULT) // Se alguma aresta tem peso, o grafo virou ponderado.
                    g2->ponderado = 1;

                // Insere ele como aresta do vertice v.
                if(insere_aresta(v,v_aux,peso) == NULL) {
                    perror("(le_grafo) Erro ao inserir aresta.");
                    return NULL;
                }

                if(v_alterado)
                    v = v_aux;
                v_alterado = 0;
            }
        }
    }

    agclose(g);
    free(attr);

    return g2;
}

grafo escreve_grafo(FILE *output, grafo g) {
    if(g == NULL)
        return NULL;

    char direcao = g->direcao? '>' : '-';
    if(g->direcao)
        fprintf(output,"strict digraph \"%s\" {\n\n", g->nome);
    else
        fprintf(output,"strict graph \"%s\" {\n\n", g->nome);

    no elem, childElem;
    vertice v;
    aresta a;

    // Imprime todos os vertices
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        fprintf(output,"   \"%s\"\n",v->nome);
    }
    fprintf(output,"\n");

    // Imprime todas as arestas, percorrendo todas as listas de entrada dos vertices
    for(elem = primeiro_no(g->v); elem; elem = proximo_no(elem)) {
        v = (vertice) conteudo(elem);
        for(childElem = primeiro_no(v->entrada); childElem; childElem = proximo_no(childElem)) {
            a = (aresta) conteudo(childElem);
            fprintf(output,"   \"%s\" -%c \"%s\"", a->vs->nome, direcao, a->vc->nome);
            if(a->peso != PESO_DEFAULT)
                fprintf(output," [peso=%ld]", a->peso);
            fprintf(output,"\n");
        }
    }

    fprintf(output,"}\n");

    return g;
}

void imprime_vertice(void* param) {
    vertice v = (vertice) param;
    printf("%s, ",v->nome);
    return ;
}