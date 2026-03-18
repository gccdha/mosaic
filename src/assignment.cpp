#include "assignment.hpp"
#include <climits>
#include <vector>



std::vector<int> greedy(const int rows, const int cols, const std::vector<int>& matrix){
  std::vector<int> chosen(cols, -1);
  for(int i = 0; i < rows; i++){
    int min = INT_MAX;
    int min_element = -1;
    for(int j = 0; j < cols; j++){
      int score = matrix[i*cols+j];
      if((chosen[j] == -1) && (score < min)){
        min = score;
        min_element = j;
      }
    }
    chosen[min_element] = i;
  }
  return chosen;
}
