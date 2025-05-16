#include "cell.h"
#include <vector>

struct Shape {
	int MinI = 0, MinJ = 0;
	int MaxI = 0, MaxJ = 0;

    Shape getShape(const std::vector<Cell>& cells);
};