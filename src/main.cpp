#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <format>
#include <iomanip>
#include <ios>
#include <iostream>
#include <chrono>
#include <span>
#include <stdlib.h>
#include <cmath>
#include <opencv2/opencv.hpp>
// #include <tuple>
#include <utility>
#include <vector>
// #include <getopt.h>
#include <omp.h>
#include "assignment.hpp"
#include "images.hpp"
#define SQSIZE 30
#define SQWIDTH 64
#define SQHEIGHT 36

const static int AREA = SQWIDTH*SQHEIGHT;
const static int AREA_S = AREA*8;

int cost(cv::Mat, cv::Mat, char method = ' ');
int diff(cv::Scalar, cv::Scalar, char method = ' ');
cv::Mat transform(cv::Mat, bool reflect = 0, int rotate = 0);
void disect(cv::Mat, cv::Mat[], bool);
void optimize(int score[AREA][AREA_S], int output[AREA], char mode = 'm');
void print_matrix(const int rows, const int cols, std::vector<int>& matrix, std::pair<std::vector<int>, const std::vector<int>> lines = {{},{}});
void hungarian_algorithm(const int rows, const int cols, std::vector<int>& matrix, std::vector<std::pair<int, int>>& output);
int draw_lines(const int rows, const int cols, std::vector<int>& matrix, std::pair<std::vector<int>, std::vector<int>>& lines);
std::vector<int> graph_hungarian(const int rows, const int cols, const std::vector<int>& matrix);
int evaluate_matching(std::vector<int>& matrix, std::vector<int>& matching);
int brute_forcer(const int rows, const int cols, std::vector<int>& matrix);
void print_matching(std::vector<int>& matching);
std::vector<int> auction_algorithm(const int rows, const int cols, std::vector<int> matrix);


int main (int argc, char *argv[]){
  
  // const auto start = std::chrono::high_resolution_clock::now(); //Start timer
  // //Parse command line arguments
  // if(argc <= 2) std::cout << "ERROR: expected 2 arguments" << std::endl;
  // else return -1;
  //
  // std::string file = argv[1];
  // std::string file2 = argv[2];
  // std::cout<<"reading "<<file<< " " <<file2<<'\n';
  //
  // //Read files. img is source and img2 is targt TODO: rename vars to reflect this
  // cv::Mat img = cv::imread(file);
  // cv::Mat img2 = cv::imread(file2);
  // if (img.empty()||img2.empty()) {
  //     std::cerr << "Image not loaded!" << std::endl;
  //     return -1;
  // }
  //
  // //Extract into array
  // cv::Mat squares[AREA_S];
  // cv::Mat tiles[AREA];
  // disect(img, squares, true);
  // disect(img2, tiles, false);
  //
  // //Generate cost matrix for each pairing of input and output:
  // int table[AREA][AREA_S];
  // #pragma omp parallel for collapse(2)
  // for(int i = AREA-1; i>=0;i--){
  //   for(int j = AREA_S-1;j>=0;j--){
  //     table[i][j]=cost(squares[j],tiles[i]);
  //   }
  // }
  //
  // //Find optimal solution and output squares in corresponding order (need to be constructed externally with imagemagick)
  // //TODO: find a way to stitch images back together without outputting them
  // int out[AREA];
  // optimize(table, out, 'g');
  // cv::Mat new_img = img2.clone();
  // for(int i = 0; i<AREA; i++){
  //   //std::cout << i << ":  " <<(i/SQWIDTH)*SQSIZE << "  " <<(i/SQWIDTH)*SQSIZE+119 << "  " << (i%SQWIDTH)*SQSIZE << "  " <<(i%SQWIDTH)*SQSIZE+119 << '\n';
  //   cv::Mat roi = new_img(cv::Range((i/SQWIDTH)*SQSIZE,(i/SQWIDTH)*SQSIZE+SQSIZE-1),cv::Range((i%SQWIDTH)*SQSIZE,(i%SQWIDTH)*SQSIZE+SQSIZE-1));
  //   squares[out[i]].copyTo(roi);
  //   cv::imwrite("output/"+ std::to_string(i)+"out.bmp", squares[out[i]]);
  // }
  //
  // cv::imwrite("test.bmp", new_img);
  //
  //
  //
  // //img.at<cv::Vec3b>(1000,0) = {0,0,255};
  // //cv::putText(img, "Graphic design is my passion", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
  // //cv::imwrite("test.bmp", transform(img,1,3));
  //
  // /*for(int i=0; i<(16*9*8*16*9); i++){
  // std::cout << "pixel: " << cost(squares[1][1], squares[5][9]);
  // std::cout << "\n avg:"<< cost(squares[1][1], squares[5][9], 'a');
  // std::cout << '\n' << i << std::endl;
  // }*/
  //
  // const auto end = std::chrono::high_resolution_clock::now(); //End timer
  // const std::chrono::duration<double> elapsed_seconds{end-start};
  // std::cout << "run in " << elapsed_seconds.count() <<"s\n";
  // //system("feh test.bmp");
  // //system("feh -i *out.bmp");
  // return 0;
  //
  
  const int rows = 1000;
  const int columns = 1000;


  std::vector<int> vec(rows*columns, 0);
  

  srand(10);
  for(int i = 0; i < rows*columns; i++){
    vec[i] = rand() % 1000;
    // std::cout << vec[i] << ' ';
  }


  const auto h_start = std::chrono::high_resolution_clock::now();
  std::vector<int> h_matching = graph_hungarian(rows, columns, vec);
  const auto h_end = std::chrono::high_resolution_clock::now(); //End timer
  const std::chrono::duration<double> h_elapsed_seconds{h_end-h_start};
  std::cout << "Hungarian run in " << h_elapsed_seconds.count() <<"s\n";

  const auto a_start = std::chrono::high_resolution_clock::now();
  std::vector<int> a_matching = auction_algorithm(rows,columns,vec);
  const auto a_end = std::chrono::high_resolution_clock::now(); //End timer
  const std::chrono::duration<double> a_elapsed_seconds{a_end-a_start};
  std::cout << "Auction run in " << a_elapsed_seconds.count() <<"s\n";

  int h_val = evaluate_matching(vec, h_matching);
  int a_val = evaluate_matching(vec, a_matching);
  std::cout << "Hungarian: " << h_val
            << "\nAuction: " << a_val
            << std::endl;
  // assert(h_val == a_val);


}

void print_matching(std::vector<int>& matching){
  std::cout << "Matching:\n";
  for(int i = 0; i< matching.size(); i++){
    std::cout << "Worker " << i << " is assigned to job " << matching[i] << std::endl;
  }
  std::cout << std::endl;
}

//Calculate difference score between two squares using average or the summed pixelwise difference
int cost(cv::Mat source, cv::Mat target, char method){
  switch(method){
    case 'a':{ // average pixel value
      cv::Scalar s_mean = cv::mean(source);
      cv::Scalar t_mean = cv::mean(target);
      return diff(s_mean, t_mean);
      break;}
    case 'e': // edge mask (canny) TODO
      return -1;
      break;
    default:{  // summed pixelwise difference
      int out = 0;
      for(int i=SQSIZE-1; i>=0; i--){
        for(int j=SQSIZE-1; j>=0;j--){
          out+=diff(source.at<cv::Vec3b>(i,j),target.at<cv::Vec3b>(i,j));
        }
      }
      return out;
      break;}
  }
  return -100;
}

//calculate the difference between two pixels using brightness or manhatten distance
int diff(cv::Scalar source, cv::Scalar target, char method){
  switch(method){
    case 'b': //brightness
      return std::abs((source[0]+source[1]+source[2])-(target[0]+target[1]+target[2]));
      break;
    default: //manhatten distance
      return std::abs(source[0]-target[0])+abs(source[1]-target[1])+abs(source[2]-target[2]);
      break;
  }
}


//Reflect and/or rotate an image:
//if reflect == true, reflect over the x axis
//rotate rotates 90, 180 or 270 degrees for 1, 2, and 3 respectively
//reflection is applied before rotation
cv::Mat transform(cv::Mat img, bool reflect, int rotate){
  if(reflect){
    // reflect over x axis
    cv::flip(img, img, 0);
  }
  switch(rotate){
    case 1:
      cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
      break;
    case 2:
      cv::rotate(img, img, cv::ROTATE_180);
      break;
    case 3:
      cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);
      break;
  }
  return img.clone();
}

//disect the input image into a number of squares based on the global parameters
//can choose to include or not include rotations and reflections
//the first AREA elements in squares will be the normal images. this will be followed by 90, 180, 270
// and reflected 0, 90, 180, and 270 rotated squares.
void disect(cv::Mat img, cv::Mat squares[], bool symetries){
  //first, loop over all the macro-squares
  for(int i=SQHEIGHT-1; i>=0; i--){
    int yindex = i*SQSIZE;
    for(int j=SQWIDTH-1; j>=0; j--){
      int xindex = j*SQSIZE;
      cv::Mat tmp(cv::Size(SQSIZE,SQSIZE),CV_8UC3,cv::Scalar(0,0,0));
      int square = i*SQWIDTH+j;
      squares[square]=tmp;
      //then, loop over all pixels in the macro-square
      for(int k=SQSIZE-1; k>=0; k--){
        for(int l=SQSIZE-1;l>=0; l--){
          squares[square].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(yindex+k, xindex+l); // TODO: Is in neccesary to copy each pixel likes this?
          //squares[i][j].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(i, j);
        }
      }
    }
  }
  if(!symetries) return;

  //add reflections and rotations
  for(int i = 1; i<8;i++){
    int k=i*AREA;
    int ref = (i)/4;
    int rot = (i)%4;
    for(int j=0; j<AREA; j++){
      squares[j+k]=transform(squares[j],ref,rot); 
    }
  }
  return;
}

//generate output image based on cost matrix using greedy with or without repeats
//TODO: Why do both m and g use AREA_S - 2 in the second nested loop?
void optimize(int score[AREA][AREA_S], int output[AREA], char mode){
  switch(mode){
    case 'm':{ // Pure minimum without repeat protection
      for(int i=AREA-1; i>=0; i--){
        int min = score[i][AREA_S-1];
        output[i] = AREA_S-1;
        for(int j=AREA_S-2; j>=0; j--){
          if(score[i][j]<min){
            min = score[i][j];
            output[i] = j;
          }
        }
      }
      return;
      break;
    }
    case 'g':{ // greedy
      bool used[AREA_S];
      for (int i=0; i<AREA_S;i++) used[i]=0;

      for(int i=AREA-1; i>=0; i--){
        int min = score[i][AREA_S-1];
        output[i] = AREA_S-1;
        
        //find minimally different square
        for(int j=AREA_S-2; j>=0; j--){
          if(score[i][j]<min && !used[j]){
            min = score[i][j];
            output[i] = j;
          }
        }
        //set used for the square and all its symetries
        for(int k=0; k<8; k++){
          used[output[i]%AREA+k*AREA]=1;
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

void print_matrix(const int rows, const int cols, std::vector<int>& matrix, std::pair<std::vector<int>, const std::vector<int>> lines){
  std::span<int> matrix_span = std::span(matrix);
  int max = *std::max_element(matrix_span.begin(), matrix_span.end());
  
  int digits = log10(max)+1;
  for(int i = 0; i< rows; i++){
    for(int j = 0; j<cols; j++){
      bool col_cover = std::find(lines.second.begin(), lines.second.end(), j) != lines.second.end(); 
      std::string fill = " ";
      if (col_cover) {
        fill = "║";
      }
      std::string formatstr = std::format("{{:^{}}}", digits+1);
      std::cout << std::vformat(formatstr,std::make_format_args(fill));
    }
    std::cout << std::endl;

    for(int j = 0; j<cols; j++){
      std::string fill = "";

      //if the current cell is covered, replace it with an appropriate box drawing char
      bool row_cover = std::find(lines.first.begin(), lines.first.end(), i) != lines.first.end();
      if (row_cover) {
        fill = "═";
      }

      std::string formatstr = std::format("{{:{}>{}}}", fill, digits+1);
      
      std::cout << std::vformat(formatstr, std::make_format_args(matrix[i*cols+j]));
    }
    std::cout << std::endl;
  }
  
}

//TODO: lots of duplicated code from draw_lines, so maybe do something about that
void assign_tasks(const int rows, const int cols, std::vector<int>& matrix, int assignments[]){ //assignments has size "rows"
  int col_zeroes[cols];
  int row_zeroes[rows];


  for(int i = 0; i< cols; i++){
    col_zeroes[cols] = -1;
  }

  int assignment_count = 0;
  bool assign_arbitrary = false;
  while(assignment_count < rows)
  {   
    bool row_assigned = false;
    //assign all rows with one optimal worker
    for(int i = 0; i < rows; i++){
      int z = 0;
      int latest = 0; //gives column if there is one worker
      for(int j = 0; j < cols; j++){
        if(matrix[i*cols + j] == 0 && col_zeroes[j] != 0){
          z++;
          latest = j;
        }
      }
      //check if there is one worker or if every row and col has more than 1 worker
      if(z == 1 || (z > 1 && assign_arbitrary)){
        assignments[i] = latest;
        assignment_count++;
        row_zeroes[i] = 0;
        row_assigned = true;
        assign_arbitrary = false;
        std::cout << "assign " << latest << " to job " << i << std::endl;
        // col_zeroes[latest]--;
      } 
      else row_zeroes[i] = z;
    }

    bool col_assigned = false;
    //if no jobs have one optimal worker, we try to assign workers with one optimal jobs
    for(int j = 0; (j < cols)&&(!row_assigned); j++){
      int z = 0;
      int latest = 0;
      for(int i=0; i < rows; i++){
        if(matrix[i*cols+j] == 0 && row_zeroes[i] != 0){
          z++;
          latest = i;
        }
      }
      if(z == 1){
        assignments[latest] = j;
        assignment_count++;
        col_zeroes[j]=0;
        col_assigned = true;
        std::cout << "assign " << j << " to job " << latest << std::endl;
      }
      col_zeroes[j]=z;
    }

    if(!(row_assigned || col_assigned)) assign_arbitrary = true;
  }



  //We greedilly assign tasks with only one zero in the row or column.
  
  // std::cout << "num_lines: " << num_lines;
  // return num_lines;
  //lines has 2 vectors, one for rows with lines in them and one for columns with lines in them
 
}


//Find the optimal assignment
void hungarian_algorithm(const int rows, const int cols, std::vector<int>& matrix, std::vector<std::pair<int, int>>& output){

  assert(rows <= cols);
  
  

  //find the min in each row and then subtract it from each element in that row
  for (int i = 0; i < rows; i++){


    int min = INT_MAX;
    for (int j = 0; j < cols; j++){
      min = std::min(min, matrix[i*rows +  j]);
    }

    // std::span<int> row = std::span(matrix).subspan(i * cols, cols);
    // int min = *std::min_element(row.begin(), row.end());

    for(int j = 0; j<cols; j++){
      matrix[i*cols + j] -= min;
    }
  }
  

  //find the min in each column and then subtract it from each element in that column
  for(int j = 0; j < cols; j++){
    int min = INT_MAX;

    for (int i = 0; i < rows; i++) {
      min =  std::min(min, matrix[i*rows + j]);
    }

    for (int i = 0; i < rows; i++){
      matrix[i*rows + j] -=  min;
    }
  }


  // for(int i = 0; i < rows; i++){
  //   for(int j = 0; j< cols; j++){
  //     std::cout << matrix[i*rows + j] << ' ';
  //   }
  //   std::cout << std::endl;
  // };

  std::pair<std::vector<int>, std::vector<int>> lines;
  int numlines = 0;
  while((numlines = draw_lines(rows, cols, matrix, lines)) < rows){
    // find smallest value not in a row or column that is covered
    // for(int i = 0; i < rows; i++){
    //   if(std::find(lines.first.begin(), lines.first.end(), i) == lines.first.end()){
    //   }
    // }

    int min = INT_MAX;
    for(int i = 0; i < rows*cols; i++){
      int row = i/cols;
      int col = i%cols;

      //if the current cell isn't covered by a line, count it for the min
      if(std::find(lines.first.begin(), lines.first.end(), row) == lines.first.end()
      && std::find(lines.second.begin(), lines.second.end(), col) == lines.second.end()){
        min = std::min(min, matrix[i]);
      }
    }

    for(int i = 0; i < rows*cols; i++){
      int row = i/cols;
      int col = i%cols;

      //if the current cell is covered, do nothing, if its double covened, add the min, if its uncovered, subtract the min
      bool line_row = std::find(lines.first.begin(), lines.first.end(), row) != lines.first.end();
      bool line_col = std::find(lines.second.begin(), lines.second.end(), col) != lines.second.end();
      if(line_row && line_col){
        matrix[i] += min;
      }
      else if(!line_row && !line_col){
        matrix[i] -= min;
      }
    }

    // print_matrix(rows, cols, matrix, lines);
    lines = {{},{}};
  }

  std::cout << "rows: ";
  for(const auto& el : lines.first){
    std::cout << el << ", ";
  }
  std::cout << std::endl << "cols: ";
  for(const auto& el : lines.second){
    std::cout << el << ", ";
  }
  std::cout << std::endl;



  print_matrix(rows, cols, matrix, lines);
  std::cout << "with " << numlines << " lines\n";


  int assignments[rows];
  for(int i = 0; i<rows; i++){
    assignments[i] = -1;
  }

  assign_tasks(rows, cols, matrix, assignments);

  for(int i = 0; i<rows; i++){
    std::cout << "assignment " << i << ": " << assignments[i] << std::endl;
  }



}

//Returns the number of lines drawn and populates "lines" argument with vectors with index of rows and columns with lines on them (respectively)
int draw_lines(const int rows, const int cols, std::vector<int>& matrix, std::pair<std::vector<int>, std::vector<int>>& lines){
  int col_zeroes[cols];
  int row_zeroes[rows];


  for(int i = 0; i < rows; i++){
      int z = 0;
      for(int j = 0; j < cols; j++){
        if(matrix[i*cols + j] == 0) z++;
    }
    row_zeroes[i] = z;
  }

  for(int j = 0; j < cols; j++){
    int z = 0;
    for(int i=0; i < rows; i++){
      if(matrix[i*cols+j] == 0) z++;
    }
    col_zeroes[j]=z;
  }


  //We greedilly assign lines based on the number of zeroes in a row or column.
  int num_lines = 0;
  while(true){
    // find row/column with max number of zeroes

    // std::pair<int, int> maxrow = {0,0}; // row, value
    // std::pair<int, int> maxcol = {0,0}; // col, value
    // TODO: rearrange this into a for loop for clarity

    //index of row/col that contains maximum # of zeroes
    int maxrow = 0;
    int maxcol = 0;

    //find the maximum row and col 
    for(int i = 0; i < rows; i++){
      if(row_zeroes[i] > row_zeroes[maxrow]){
        maxrow = i;
      }
    }
    
    for(int j = 0; j < cols; j++){
      if(col_zeroes[j] > col_zeroes[maxcol]){
        maxcol = j;
      }
    }

    if(row_zeroes[maxrow] == 0) break; //this means all zeroes are covered
    num_lines++;

    //put a line on the row or column with the most zeroes (equal means row because of locality)
    //remove coverd zeroes from the count from the rows and columns that they are a part of
    //add the line to the list of lines
    if(row_zeroes[maxrow] < col_zeroes[maxcol]){
      lines.second.push_back(maxcol);
      for(int i = 0; i < rows; i++){
        if(matrix[i*cols + maxcol] == 0) row_zeroes[i]--;
      }
      col_zeroes[maxcol] = 0;
    } else {
      lines.first.push_back(maxrow);
      for(int j = 0; j < cols; j++){
        if(matrix[maxrow*cols + j] == 0) col_zeroes[j]--;
      }
      row_zeroes[maxrow] = 0;
    }
    // print_matrix(rows, cols, matrix, lines);
    // std::cout << "=================" << std::endl;
  }
  
  // std::cout << "num_lines: " << num_lines;
  return num_lines;
  //lines has 2 vectors, one for rows with lines in them and one for columns with lines in them
 
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














/* TODO:
 * 3. implement hungarian alg
 * 4. figure out how to restitch the image back together
 * 5. implement the canny edge finding
 * 6. replace this : montage $(seq -f "%gout.bmp" 0 2303) -tile 64x36 -geometry +0+0 -font DejaVu-Sans output.jpg
 * 7. implement in GPU for real time (maybe reararange previous frame to get current frame?)
 * 8. Do the same thing but for sound; reararange song into itself or into another song
 * TESTS: Does it correctly rearrange the image into itself? Can it correctly recreate the target image if the input
 * includes every color, the square size is 1px, and repeats are allowed
 * */
