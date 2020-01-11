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
#include <util/md5.h>

#include <util/serialization/histogram_stats.h>
#include <util/serialization/pretty_print.h>

#include <util/graph/graph.h>
#include <reordering_utils/reorder_utils.h>
#include <util/graph/triangle_counting.h>
#include <util/graph/pre_processing.h>
#include <util/graph/pre_processing_dodg.h>

#define NO_ATOMIC
#ifdef NO_ATOMIC

#include <util/sort/parasort_cmp.h>

#endif

using namespace popl;
using namespace std;

graph_t YcheGraphSimplify(Graph &yche_graph) {
    graph_t g;
    g.adj = yche_graph.edge_dst;
    g.num_edges = yche_graph.node_off;
    g.n = yche_graph.nodemax;
    g.m = yche_graph.edgemax;
    return g;
}

size_t TC(Graph &yche_graph) {
    vector<size_t> tc_cnts;
    graph_t g = YcheGraphSimplify(yche_graph);

    vector<int32_t> new_dict;
    vector<int32_t> old_dict;
    auto prev = g.adj;
    auto tmp = (int32_t *) malloc(g.m * sizeof(int32_t));
    ReorderDegDescending(g, new_dict, old_dict, tmp);
    free(prev);
    size_t whole_g_tc_cnt = CountTriBMPAndMergeWithPack(g, omp_get_max_threads());
    log_debug("");

    // To DODG.
    auto edge_lst = (pair<int32_t, int32_t> *) malloc(g.m * sizeof(pair<int32_t, int32_t>));
#pragma omp parallel for schedule(dynamic, 1000)
    for (auto u = 0; u < g.n; u++) {
        for (auto off = g.num_edges[u]; off < g.num_edges[u + 1]; off++) {
            if (u < g.adj[off]) {
                // no duplicates
                edge_lst[off].first = u;
                edge_lst[off].second = g.adj[off];
            }
        }
    }

    {
        // 1st: Remove Multi-Edges and Self-Loops.
        Timer global_timer;
        Timer sort_timer;
        int32_t max_node_id = 0;
        auto num_edges = g.m;
#pragma omp parallel for reduction(max: max_node_id) schedule(dynamic, 32*1024)
        for (size_t i = 0u; i < num_edges; i++) {
            if (edge_lst[i].first > edge_lst[i].second) {
                swap(edge_lst[i].first, edge_lst[i].second);
            }
            max_node_id = max(max_node_id, max(edge_lst[i].first, edge_lst[i].second));
        }
        log_info("Populate File Time: %.9lfs", global_timer.elapsed());
#ifdef NO_ATOMIC
        parasort(num_edges, edge_lst, [](auto l, auto r) {
            if (l.first == r.first) {
                return l.second < r.second;
            }
            return l.first < r.first;
        });
#else
        // In-Place Parallel Sort.
        ips4o::parallel::sort(edge_lst, edge_lst + num_edges, [](auto l, auto r) {
            if (l.first == r.first) {
                return l.second < r.second;
            }
            return l.first < r.first;
        });
#endif
        log_info("Sort Time: %.9lfs", sort_timer.elapsed());
        auto num_vertices = static_cast<uint32_t >(max_node_id) + 1;
        log_info("Pre-Process Edge List Time: %.9lf s", global_timer.elapsed());

        // 2nd: Convert Edge List to CSR.
        graph_t g{.n=num_vertices, .m = 0, .adj=nullptr, .num_edges=nullptr};
        uint32_t *deg_lst;
        g.adj = nullptr;
        auto max_omp_threads = omp_get_max_threads();
        ConvertEdgeListToDODGCSR((eid_t) num_edges, edge_lst, num_vertices, deg_lst, g.num_edges, g.adj,
                                 max_omp_threads, [&](size_t it) {
                    return !(edge_lst[it].first == edge_lst[it].second
                             || (it > 0 && edge_lst[it - 1] == edge_lst[it]));
                });
        g.m = g.num_edges[num_vertices];
        log_info("Undirected Graph G = (|V|, |E|): %lld, %lld", g.n, g.m);
        log_info("Mem Usage: %s KB", FormatWithCommas(getValue()).c_str());

        // 3rd: Reordering.
        vector<int32_t> new_dict;
        vector<int32_t> old_dict;

        auto *tmp_mem_blocks = (int32_t *) malloc(g.m * sizeof(int32_t));
        auto *org = g.adj;
        ReorderDegDescendingDODG(g, new_dict, old_dict, tmp_mem_blocks, deg_lst);
        free(org);
        free(deg_lst);

        // 4th: Triangle Counting.
        log_info("Mem Usage: %s KB", FormatWithCommas(getValue()).c_str());
        size_t tc_cnt = 0;
        tc_cnt = CountTriBMPAndMergeWithPackDODG(g, max_omp_threads);
        log_info("Mem Usage: %s KB", FormatWithCommas(getValue()).c_str());
        log_info("There are %s triangles in the input graph.", FormatWithCommas(tc_cnt).c_str());
        assert(whole_g_tc_cnt == tc_cnt);
    }
    return whole_g_tc_cnt;
}

vector<int32_t> PKC_refactor(string file_name, Graph &yche_graph) {
    // Core-Decomposition.
    graph_t g = YcheGraphSimplify(yche_graph);

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
    log_info("PKC time: %.6lf s", timer.elapsed());
    return histogram;
/*******************************************************************************************/
}

int main(int argc, char *argv[]) {
    OptionParser op("Allowed options");
    auto input_dir_option = op.add<Value<std::string >>("i", "i-file-path", "the graph input bin directory path");
    op.parse(argc, argv);

    // |V|, |E|, avg-deg, max-deg, dodg-max-deg
    // |TC|, core-info-his
    if (input_dir_option->is_set()) {
        auto file_name = input_dir_option->value(0);

        // V|, |E|, avg-deg, max-deg, dodg-max-deg.
        Graph yche_graph(const_cast<char *>(file_name.c_str()));
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

        // Core-Decomposition.
        auto histogram = PKC_refactor(file_name, yche_graph);

        // Triangle-Counting.
        auto tc_cnt = TC(yche_graph);

        // Output Stats.
        log_info("|V|: %s", FormatWithCommas(yche_graph.nodemax).c_str());
        log_info("|E|: %s", FormatWithCommas(yche_graph.edgemax / 2).c_str());
        log_info("|TC|: %s", FormatWithCommas(tc_cnt).c_str());
        log_info("avg-deg: %.6lf", static_cast<double >(yche_graph.edgemax) / yche_graph.nodemax);
        log_info("max-deg: %s", FormatWithCommas(org_max_deg).c_str());
        log_info("dodg-max-deg: %s", FormatWithCommas(dodg_max_deg).c_str());
        log_info("max-core-val: %s", FormatWithCommas(histogram.size() - 1).c_str());

        auto output_dir = file_name + "/" + string("basic_stats") + ".txt";
        ofstream ofs(output_dir);
        ofs << "|V|: " << FormatWithCommas(yche_graph.nodemax) << endl;
        ofs << "|E|: " << FormatWithCommas(yche_graph.edgemax / 2) << endl;
        ofs << "|TC|: " << FormatWithCommas(tc_cnt) << endl;
        ofs << "avg-deg: " << static_cast<double >(yche_graph.edgemax) / yche_graph.nodemax << endl;
        ofs << "max-deg: " << FormatWithCommas(org_max_deg) << endl;
        ofs << "dodg-max-deg: " << FormatWithCommas(dodg_max_deg) << endl;
        ofs << "max-core-val: " << FormatWithCommas(histogram.size() - 1) << endl;
    } else {
        log_error("No valid options");
    }
}