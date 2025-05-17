#pragma once
#include "cell.h"
#include <vector>
#include <memory>

struct Sector {
    std::vector<Cell> Cells;
    std::shared_ptr<int> Number = nullptr;

    bool Contains(const Cell& cell) const;
    std::vector<std::vector<Cell>> Combs() const;
};