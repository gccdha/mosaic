#pragma once

#include <vector>

std::vector<int> greedy(const int rows, const int cols, const std::vector<int>& matrix);
void optimize(std::vector<int>score, std::vector<int>& output, const int cols, const int rows, char mode = 'm');
std::vector<int> graph_hungarian(const int rows, const int cols, const std::vector<int>& matrix);
int brute_forcer(const int rows, const int cols, std::vector<int>& matrix);
std::vector<int> auction_algorithm(const int rows, const int cols, std::vector<int> matrix);
