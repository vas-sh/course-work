#pragma once
#include "cell.h"
#include "validator.h"
#include <vector>
#include <memory>
#include <functional>

struct Sector {
    std::vector<Cell> Cells;
    std::shared_ptr<int> Number = nullptr;

    bool Contains(const Cell& cell) const {
        if (Number == nullptr) {
            return false;
        }
        for (size_t i = 0; i < Cells.size(); ++i) {
            if (Cells[i].Equal(cell)) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::vector<Cell>> Combs() const {
        std::vector<std::vector<Cell>> res;
        if (Number == nullptr || *Number == 0) {
            return res;
        }

        std::function<void(int, std::vector<Cell>)> backtrack;
        backtrack = [&](int start, std::vector<Cell> path) {
            if (path.size() == static_cast<size_t>(*Number)) {
                std::vector<Cell> comb = path;
                if (validComb(comb)) {
                    res.push_back(comb);
                }
                return;
            }
            for (size_t i = start; i < Cells.size(); ++i) {
                std::vector<Cell> next_path = path;
                next_path.push_back(Cells[i]);
                backtrack(i + 1, next_path);
            }
        };
        backtrack(0, std::vector<Cell>{});
        return res;
    }
};