#include "graph.h"


void free_graph(graph_t *g) {
    if (g->adj != nullptr)
        free(g->adj);

    if (g->num_edges != nullptr)
        free(g->num_edges);

    if (g->eid != nullptr)
        free(g->eid);
}

