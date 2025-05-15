#pragma once
#include "cell.cc"
#include <vector>
#include <functional>
using namespace std;

struct Sector {
	vector<Cell> Cells;
	shared_ptr<int> Number = nullptr;

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

    vector<vector<Cell>> Combs() const {
        vector<vector<Cell>> res;
        if (Number == nullptr || *Number == 0) {
            return res;
        }

        std::function<void(int, vector<Cell>)> backtrack;
        backtrack = [&](int start, vector<Cell> path) {
            if (path.size() == static_cast<size_t>(*Number)) {
                vector<Cell> comb = path; 
                extern bool validComb(const vector<Cell>& comb);
                if (validComb(comb)) {
                    res.push_back(comb);
                }
                return;
            }
            for (size_t i = start; i < Cells.size(); ++i) {
                vector<Cell> next_path = path;
                next_path.push_back(Cells[i]);
                backtrack(i + 1, next_path);
            }
        };
        backtrack(0, vector<Cell>{});
        return res;
    }
};
