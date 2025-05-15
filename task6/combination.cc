#include "cell.cc"
#include "sector.cc" 
#include "validator.cc"
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
using namespace std;

static thread Combs(atomic<bool>& cancel, const vector<Sector>& sectors, const vector<Cell> cells, function<void(const vector<Cell>)> resultHandler) {
    thread generator_thread([&cancel, sectors, resultHandler]() {
        try {
            vector<vector<vector<Cell>>> groups;
            for (size_t i = 0; i < sectors.size(); ++i) {
                 if (cancel.load()) return; 
                if (sectors[i].Number != nullptr && *sectors[i].Number > 0) {
                    groups.push_back(sectors[i].Combs());
                }
            }

            sort(groups.begin(), groups.end(), [](const vector<vector<Cell>>& a, const vector<vector<Cell>>& b) {
                return a.size() < b.size();
            });

            long long m = 0;
            size_t limit_idx = groups.size();
            for (size_t i = 0; i < groups.size(); ++i) {
                 if (cancel.load()) return;
                if (groups[i].empty() && (sectors[i].Number && *sectors[i].Number > 0)) {
                    
                     return; 
                }
                if (m == 0) {
                    if (!groups[i].empty()) {
                       m = groups[i].size();
                    }
                    continue;
                }
                if (groups[i].empty()) continue; 

                long long current_size = groups[i].size();
                if (current_size > 0 && m > 100'000'000LL / current_size) {
                    limit_idx = i;
                    break;
                }
                m *= current_size;
                if (m > 100'000'000LL) {
                    limit_idx = i;
                    break;
                }
            }
            if (limit_idx < groups.size()) {
                groups.resize(limit_idx);
            }


            function<void(int, vector<Cell>)> backtrack;
            backtrack = [&](int index, vector<Cell>path) {
                if (cancel.load()) return; 

                if (index == static_cast<int>(groups.size())) {
                    if (validComb(path)) {
                      
                        resultHandler(path);
                        if (cancel.load()) return;
                    }
                    return;
                }

                if (groups[index].empty()) {
                     backtrack(index + 1, path);
                     return;
                }


                for (const auto& option : groups[index]) {
                    if (cancel.load()) return; 

                    vector<Cell> next_path = path;
                    next_path.insert(next_path.end(), option.begin(), option.end());

                    if (validComb(next_path)) {
                        backtrack(index + 1, next_path);
                        if (cancel.load()) return;
                    }
                }
            };

            backtrack(0, vector<Cell>{});

        } catch (const exception& e) {
            cerr << "Exception in Combs generator thread: " << e.what() << endl;
        } catch (...) {
            cerr << "Unknown exception in Combs generator thread." << endl;
        }
    }); 

    return generator_thread;
}