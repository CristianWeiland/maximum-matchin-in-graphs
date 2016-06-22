#include <stdio.h>
#include "grafo.h"

//------------------------------------------------------------------------------

int main(void) {

  grafo g = le_grafo(stdin);

  if ( !g )

    return 1;

  printf("nome: %s\n", nome_grafo(g));
  int d = direcionado(g);
  printf("%sdirecionado\n", d ? "" : "não ");
  printf("%sponderado\n", ponderado(g) ? "" : "não ");
  printf("%d vértices\n", n_vertices(g));
  printf("%d arestas\n", n_arestas(g));

  escreve_grafo(stdout,g);

  grafo emparelhamento = emparelhamento_maximo(g);
  escreve_grafo(stdout,emparelhamento);
  printf("Tamanho do Emparelhamento: %d\n", n_arestas(emparelhamento));

  return ! (destroi_grafo(emparelhamento) && destroi_grafo(g));
}
