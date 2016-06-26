Trabalho referente à disciplina Algoritmos e Teoria dos Grafos, lecionada pelo professor Renato Carmo.
Este trabalho foi realizado pelo aluno Cristian Weiland (cw14) - GRR20142235.

Considerações sobre a implementação:

Explicação dos novos atributos nas estruturas:
Vértice:
- Adicionados dois atributos do tipo int chamados 'coberto' e 'visitado'.
- O atributo 'coberto' tem como função indicar se um vértice faz parte do emparelhamento ou não. Caso ele faça parte, o atributo vale 1, caso contrário, 0.
- O atributo 'visitado' tem como função auxiliar a busca por algum caminho aumentante. Caso o vértice já tenha sido visitado naquela busca por um caminho aumentante, ele terá valor 1, caso contrário, ele terá valor 0.

Aresta:
- Adicionado um atributo do tipo int chamado 'coberta'. Ele indica se a aresta faz parte do emparelhamento. Caso faça parte, seu valor é 1, caso contrário, 0.



Explicação das funções que descobrem o tamanho do emparelhamento máximo:

grafo emparelhamento_maximo(grafo g): É a função que organiza a busca pelo emparelhamento máximo. A ideia principal é buscar caminhos aumentantes até que não existam outros, e assim será descoberto o emparelhamento máximo. Depois de achar o emparelhamento máximo, são chamadas as funções que vão efetivamente criar o novo grafo que contém o emparelhamento. Isto só é feito no final para evitar a criação e destruição repetidas de vértices e arestas.

void copia_vertices(grafo g1, grafo g2): Esta função copia apenas os vértices de g2 que estão cobertos pelo emparelhamento para um novo grafo g1. Os vértices de g1 são novos vértices (isto é, não são apenas apontadores para o vértice original, mas sim uma nova estrutura vértice), mas com o mesmo nome.

void copia_arestas_cobertas(grafo g1, grafo g2): Da mesma forma, são criadas novas arestas entre os vértices do grafo que representa o emparelhamento.

void xor(lista l): Esta função serve para inverter as arestas cobertas encontradas pelo caminho aumentante, isto é, caso a aresta esteja coberta, ela é retirada do emparelhamento, caso ela não esteja coberta, ela é inserida. Desta forma, o tamanho do emparelhamento é aumentado em 1.

lista caminho_aumentante(grafo g): Função que, ao ser chamada, retorna um caminho aumentante (em forma de uma lista de arestas) para o emparelhamento do grafo g. Ela tem a função de fazer inicializações e de chamar a função busca_caminho (que é quem realmente se encarrega de formar o caminho aumentante).

int busca_caminho(vertice v, lista l, int last): Esta função é quem realmente acha o caminho aumentante. A ideia do seu algoritmo é a seguinte: começando a partir de um vértice não coberto, eu devo achar um caminho que só passe por vértices que pertencem ao emparelhamento alternatadamente e que não foram visitado por essa função antes (na busca deste caminho aumentante) até achar outro vértice não coberto. Desta forma, sei que o caminho achado é um caminho aumentante (começa em um vértice que não faz parte dele, passa por ele e termina em outor vértice não coberto). Durante a execução são inseridas estas arestas na lista, e caso o caminho seja encontrado, a função retorna TRUE.

