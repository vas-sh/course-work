#include "cell.h"
#include "validator.h"
#include <vector>


bool validComb(const std::vector<Cell>& comb) {
    if (comb.size() <= 1) {
        return true;
    }
    for (size_t i = 0; i < comb.size(); ++i) {
        for (size_t j = i + 1; j < comb.size(); ++j) {
            if (comb[i].NextTo(comb[j]) || comb[i].Equal(comb[j])) {
                return false;
            }
        }
    }
    return true;
}