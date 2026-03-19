#include "assignment.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <span>
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

//generate output image based on cost matrix using greedy with or without repeats
//TODO: Why do both m and g use cols - 2 in the second nested loop?
void optimize(std::vector<int> score, std::vector<int>& output, const int cols, const int rows, char mode){
  switch(mode){
    case 'm':{ // Pure minimum without repeat protection
      for(int i=rows-1; i>=0; i--){
        int min = score[i*cols+cols-1];
        output[i] = cols-1;
        for(int j=cols-2; j>=0; j--){
          if(score[i*cols+j]<min){
            min = score[i*cols+j];
            output[i] = j;
          }
        }
      }
      return;
      break;
    }
    case 'g':{ // greedy
      bool used[cols];
      for (int i=0; i<cols;i++) used[i]=0;

      for(int i=rows-1; i>=0; i--){
        int min = score[i*cols+cols-1];
        output[i] = cols-1;
        
        //find minimally different square
        for(int j=cols-2; j>=0; j--){
          if(score[i*cols+j]<min && !used[j]){
            min = score[i*cols+j];
            output[i] = j;
          }
        }
        //set used for the square and all its symetries
        for(int k=0; k<8; k++){
          used[output[i]%rows+k*rows]=1;
        }
      }
      return;
      break;
    }
    case 'h':{ //TODO: hungarian algorithm (optimal no-repeat solution)
      break;
    }
  }
}

std::vector<int> graph_hungarian(const int rows, const int cols, const std::vector<int>& matrix){
  //make it clearer what these mean (J is jobs, W is workers)
  const int J = rows;
  const int W = cols;
  assert(J <= W);

  // std::vector<int> jobs; //list of jobs that have been assigned (S_j)
  std::vector<int> matching(W, -1); // matching[worker] = -1 if not assigned, or job if assigned (M)
  
  //potential function
  std::vector<int> w_potential(W, 0); //for worker nodes
  std::vector<int> j_potential(J, 0); //for job nodes

  for(int current_job = 0; current_job < J; current_job++){
    //std::cout << "Assigning job " << current_job << '\n';
    // jobs.push_back(current_job); //add the current node to get S_j from S_{j-1}
    
    ///keeps track of which nodes can do what for the min and the path back to j(Z)
    std::vector<int> j_parent(J, -1); //job nodes
    std::vector<int> w_parent(W, -1);  //worker nodes
    j_parent[current_job] = W+1; //mark starting job as non existant worker so that it is > -1 but not assigned
   
    while(true){
      int min = INT_MAX;    //Min of the diff between the edge weight and potential for the endponits (Delta)
      int next_worker = -1; //A worker that has one of the edges that satisfy the above
      int parent_job = -1; //the job that the above worker was discovered from
      
      //min only includes reachable jobs and unreachable workers
      for(int j = 0; j < J; j++){
        if(j_parent[j] != -1){
          for(int w = 0; w < W; w++){
            if(w_parent[w] == -1){
              int oldmin = min;
              int v = matrix[j*W+w] - j_potential[j] - w_potential[w];
              if(v < min){ //PERF: this is the hottest part of the function. There is a way to get rid of one of these nested loops.
                min = v;
                next_worker = w;
                parent_job = j;
              }
            }
          }
        }
      }

      //update potential function
      for(int j = 0; j < J; j++){
        if(j_parent[j] != -1) j_potential[j] += min;
      }
      for(int w = 0; w < W; w++){
        if(w_parent[w] != -1) w_potential[w] -= min;
      }

      int assigned_job = matching[next_worker];

      if(assigned_job == -1){
        //follow path backwards from next_worker, changing matchings until the current job is reached (swapping which edges in the augmenting path are in or out of the matching)
        int w_traversal = next_worker;
        int j_traversal = parent_job;
        matching[w_traversal] = j_traversal;

        while(j_traversal != current_job){
          w_traversal = j_parent[j_traversal];
          j_traversal = w_parent[w_traversal];
          matching[w_traversal] = j_traversal;
        }
        break; //TODO: make a propper condition in the while loop instead of using break (somthing about assigned job == -1 and move this logic outside the loop?)
        }
      else{
        //add next_workel and its assigned job to the reachable nodes and go again
        j_parent[assigned_job] = next_worker;
        w_parent[next_worker] = parent_job;
      }
    }
  }
  return matching;
}


int evaluate_matching(std::vector<int>& matrix, std::vector<int>& matching){
  const int cols = matching.size();
  const int rows = matrix.size()/cols;
  int total = 0;
  for(int w = 0; w < cols; w++){
    int j = matching[w];
    if(j != -1) total += matrix[j*cols+w];
    // std::cout  << "Worker " << w << " did job " << j << " with cost " << matrix[j*cols+w] << std::endl;
  }
  return total;
}

void brute_force_helper(std::vector<int>& matrix, std::vector<int>& vec, int i, int len, int& counter, int& min, std::vector<int>& matching){
  if(i == len){ //base case. this means we have a full permutation
    counter++;

    int score = evaluate_matching(matrix, vec);
  
    if(score < min){
      matching = vec;
      min = score;
    }

    min = std::min(min, evaluate_matching(matrix, vec));

  }
  for(int j = i; j < len; j++){ //j=i gives the "identity" swap case 
    //swap with every element. This allows any element to start the list, and inductively that means
    //that this gets every permutation
    std::swap(vec[i], vec[j]);
    brute_force_helper(matrix, vec, i+1, len, counter, min, matching);
    std::swap(vec[i], vec[j]);
  }
}

int brute_forcer(const int rows, const int cols, std::vector<int>& matrix){ //TODO: right now this permutes workers but it should permute jobs instead since jobs <= workers
  std::vector<int> possible_assignments(cols, -1);
  for(int i = 0; i < rows; i++){
    possible_assignments[i] = i;
  }
  int counter = 0;
  int min = INT_MAX;
  std::vector<int> matching;
  brute_force_helper(matrix, possible_assignments, 0, possible_assignments.size(), counter, min, matching);
  //std::cout  << "Brute forcer value (checked " << counter << " combinations): " << min << std::endl;
  //print_matching(matching); 
  return min;
}


//dual? idk. this function sets the new value in the matrix to the max value minus the original value
void positive_inverse(std::vector<int>& matrix){
  std::span<int> matspan = std::span(matrix);
  int max = *std::max_element(matspan.begin(), matspan.end());
  std::transform(matrix.begin(), matrix.end(), matrix.begin(),
                 [max](int e) -> int{return max-e;});
}


std::vector<int> auction_algorithm(const int rows, const int cols, std::vector<int> matrix){
  const int J = rows;
  const int W = cols;
  const double epsilon = 1.0/(rows-1.0);

  positive_inverse(matrix);

  std::vector<int> assignments(W, -1);
  std::vector<int> j_assignments(J, -1);
  std::vector<double> prices(J, 0.0);

  while(true){
    //find all unassigned workers
    std::vector<int> unassigned;
    for(int w = 0; w < W; w++){
      if(assignments[w] == -1) unassigned.push_back(w);
    }
    if(unassigned.empty()) break;

    // std::vector<int> bid_items(J, -1); //best bidding worker for each item
    std::vector<double> bids(W, -1); //bid from each parallel worker

    while(!unassigned.empty()){
      const int bidder = unassigned.back();
      unassigned.pop_back();

      double primary_bid = -2e100;
      double secondary_bid = -2e100;
      int best_item = -1;

      // go through every worker and calculate their best bid
      for(int j = 0; j < J; j++){
        // find the item that gives the best value (matrix contains how much an item is "worth", and the prices are how much it "costs")
        // and the value of the second best item
        double value = matrix[j*W+bidder] - prices[j];


        if(value > primary_bid){
          secondary_bid = primary_bid;
          primary_bid = value;
          best_item = j;
        } else if(value > secondary_bid){
          secondary_bid = value;
        }
      }

      double increment = primary_bid - secondary_bid + epsilon; 

      prices[best_item] += increment;

      int previous = j_assignments[best_item];

      if(previous != -1) assignments[previous] = -1;

      assignments[bidder] = best_item;
      j_assignments[best_item] = bidder;
      
      if(assignments[previous] == -1) unassigned.push_back(previous);
    }
  }
return assignments;
}

