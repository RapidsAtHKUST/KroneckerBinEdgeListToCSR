//
// Created by yche on 1/12/20.
//

#include <util/primitives/primitives.h>

using eid_t = uint32_t;
eid_t *tmp_histogram;

int main() {
    auto reduceN = 19840;
    tmp_histogram = (eid_t *) malloc((reduceN + 1) * sizeof(eid_t));
    MemSetOMP(tmp_histogram, 0, reduceN + 1);
    log_info("ok");
}
