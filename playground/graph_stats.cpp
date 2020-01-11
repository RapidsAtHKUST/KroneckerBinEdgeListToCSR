//
// Created by yche on 1/11/20.
//

#include <cassert>
#include <fstream>

#include <omp.h>

#include <util/program_options/popl.h>
#include <util/timer.h>
#include <util/util.h>
#include <util/stat.h>
#include <util/serialization/histogram_stats.h>
#include <util/md5.h>
#include <util/serialization/pretty_print.h>

#include <util/graph/graph.h>
#include <reordering_utils/reorder_utils.h>

using namespace popl;
using namespace std;

void PKC_refactor(string file_name, Graph &yche_graph) {
    // Core-Decomposition.
    graph_t g;
    g.adj = yche_graph.edge_dst;
    g.num_edges = yche_graph.node_off;
    g.n = yche_graph.nodemax;
    g.m = yche_graph.edgemax;
    long n = g.n;
    /* Contains the core number for each vertex */
    int *core = (int *) malloc(n * sizeof(int));
    assert(core != NULL);

    log_info("Computing k-core decomposition ...");

    Timer timer;
    /* If the graph has at least 1000 vertices use PKC  */
    PKC(&g, core, omp_get_max_threads());
    auto histogram = core_val_histogram(g.n, core);

    stringstream ss;
    if (histogram.size() < 300) {
        ss << histogram;
        log_info("All: %s", ss.str().c_str());
    } else {
        ss << pretty_print_array(&histogram.front(), 300);
        log_info("Top 300: %s", ss.str().c_str());
    }
    {
        stringstream ss;
        ss << histogram << "\n";
        log_info("Md5sum of histogram: %s", md5(ss.str()).c_str());
    }

    auto output_dir = file_name + "/" + string("kcore") + ".histogram";
    log_info("Output Dir: %s", output_dir.c_str());
    ofstream ofs(output_dir);
    ofs << histogram << endl;
    free(core);
    log_info("PKC time: %.6lf s\n", timer.elapsed());
/*******************************************************************************************/
}

int main(int argc, char *argv[]) {
    OptionParser op("Allowed options");
    auto input_dir_option = op.add<Value<std::string >>("i", "i-file-path", "the graph input bin directory path");
    log_info("%s", op.description().c_str());
    op.parse(argc, argv);

    // |V|, |E|, avg-deg, max-deg, dodg-max-deg
    // |TC|, core-info-his
    if (input_dir_option->is_set()) {
        auto file_name = input_dir_option->value(0);
        Graph yche_graph(const_cast<char *>(file_name.c_str()));

        // max deg
        uint32_t dodg_max_deg = 0;
        uint32_t org_max_deg = 0;
        auto directed_out_deg = (uint32_t *) malloc(sizeof(uint32_t) * yche_graph.nodemax);
#pragma omp parallel for schedule(dynamic, 1) reduction(max:dodg_max_deg) reduction(max:org_max_deg)
        for (auto u = 0; u < yche_graph.nodemax; u++) {
            auto dodg_deg = 0u;
            for (auto iter = yche_graph.node_off[u]; iter < yche_graph.node_off[u + 1]; iter++) {
                auto v = yche_graph.edge_dst[iter];
                if (yche_graph.degree[v] > yche_graph.degree[u] ||
                    ((yche_graph.degree[v] == yche_graph.degree[u]) && (u < v))) {
                    dodg_deg++;
                }
            }
            directed_out_deg[u] = dodg_deg;
            dodg_max_deg = max(dodg_max_deg, dodg_deg);
            org_max_deg = max(org_max_deg, yche_graph.node_off[u + 1] - yche_graph.node_off[u]);
        }

        log_info("|V|: %s", FormatWithCommas(yche_graph.nodemax).c_str());
        log_info("|E|: %s", FormatWithCommas(yche_graph.edgemax / 2).c_str());
        log_info("avg-deg: %.6lf", static_cast<double >(yche_graph.edgemax) / yche_graph.nodemax);
        log_info("max-deg: %s", FormatWithCommas(org_max_deg).c_str());
        log_info("dodg-max-deg: %s", FormatWithCommas(dodg_max_deg).c_str());

        auto output_dir = file_name + "/" + string("basic_stats") + ".txt";
        ofstream ofs(output_dir);
        ofs << "|V|: " << FormatWithCommas(yche_graph.nodemax) << endl;
        ofs << "|E|: " << FormatWithCommas(yche_graph.edgemax / 2) << endl;
        ofs << "avg-deg: " << static_cast<double >(yche_graph.edgemax) / yche_graph.nodemax << endl;
        ofs << "max-deg: " << FormatWithCommas(org_max_deg) << endl;
        ofs << "dodg-max-deg: " << FormatWithCommas(dodg_max_deg) << endl;

        PKC_refactor(file_name, yche_graph);
    } else {
        log_error("No valid options");
    }
}