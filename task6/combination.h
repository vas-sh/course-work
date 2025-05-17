#pragma once
#include "sector.h"
#include "cell.h"
#include <thread>
#include <atomic>
#include <vector>
#include <functional>


std::thread Combs(
    std::atomic<bool>& cancel,
    const std::vector<Sector>& sectors,
    const std::vector<Cell> cells,
    std::function<void(const std::vector<Cell>)> resultHandler
);