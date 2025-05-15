#include <limits>
#include "cell.cc"
#include <vector>
using namespace  std;

struct Shape {
	int MinI = 0, MinJ = 0;
	int MaxI = 0, MaxJ = 0;
};

Shape getShape(const vector<Cell>& cells) {
	Shape res;
    res.MinI = numeric_limits<int>::max();
    res.MinJ = numeric_limits<int>::max();
    res.MaxI = numeric_limits<int>::min();
    res.MaxJ = numeric_limits<int>::min();

    if (cells.empty()) {
        res.MinI = 0; res.MinJ = 0; res.MaxI = 0; res.MaxJ = 0;
        return res;
    }

	for (size_t i = 0; i < cells.size(); ++i) {
		res.MinI = min(cells[i].i, res.MinI);
		res.MinJ = min(cells[i].j, res.MinJ);
		res.MaxI = max(cells[i].i, res.MaxI);
		res.MaxJ = max(cells[i].j, res.MaxJ);
	}
	return res;
}