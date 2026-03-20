#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <format>
#include <iostream>
#include <chrono>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <span>
#include <sstream>
#include <stdlib.h>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <utility>
#include <vector>
#include <omp.h>
#include "assignment.hpp"
#include "images.hpp"
#include "compare.hpp"

void print_matrix(const int rows, const int cols, std::vector<int>& matrix, std::pair<std::vector<int>, const std::vector<int>> lines = {{},{}});
int evaluate_matching(std::vector<int>& matrix, std::vector<int>& matching);
void print_matching(std::vector<int>& matching);


int main (int argc, char *argv[]){



  const int tile_height = 120; //WARN: be cautious with tile_height > ~1500 because costs could approach INT_MAX
  const int tile_width = tile_height; 
  const int height = 9;
  const int width = 16;

  // const auto start = std::chrono::high_resolution_clock::now(); //Start timer

  //Parse command line arguments
  if(argc <= 2){
    std::cout << "ERROR: expected 2 arguments" << std::endl;
    return -1; 
  }

  //Read files. Return if one can't be loaded
  std::string source_file = argv[1];
  std::string target_file = argv[2];
  cv::Mat source, target;

  if(!(read_image(source_file, source) && read_image(target_file, target)))
    return -1;

  //Crop images to be the same size and be tile aligned
  // crop_to_match( source, target, tile_width, tile_height);

  //Extract images into vectors of tiles
  std::vector<cv::Mat> source_tiles, target_tiles;
  bool all_rotations[8] = {1,0,0,0,0,0,0,0};
  bool no_rotations[8] = {1,0,0,0,0,0,0,0};
  disect(source, source_tiles, all_rotations, height, width, tile_height, tile_width);
  disect(target, target_tiles, no_rotations, height, width, tile_height, tile_width);

  const int rows = target_tiles.size();
  const int cols = source_tiles.size();

  for(int i = 0; i < rows; i++){
    std::stringstream ss;
    ss << "testoutput/t/" << i << ".bmp";
    cv::imwrite(ss.str(), target_tiles[i]);
  }

  for(int i = 0; i < cols; i++){
    std::stringstream ss;
    ss << "testoutput/s/" << i << ".bmp";
    cv::imwrite(ss.str(), source_tiles[i]);
  }


  //Generate the cost matrix
  std::vector<int> matrix = generate_matrix(source_tiles, target_tiles, true);

  print_matrix(rows, cols, matrix);

  //Find optimal matching
  std::vector<int> matching = graph_hungarian(rows, cols, matrix);


  //Get corresponding tiles 
  std::vector<cv::Mat> tiles = select_images(matching, source_tiles, rows);

  //Stitch together final image and make visualization
  cv::Mat final_image = stitch(tiles, width, height);
  cv::Mat visualization = visualize_matching(matching, height, width, tile_height, tile_width);
  
  std::cout << "Writing files\n";

  //write files
  cv::imwrite("output.bmp", final_image);
  cv::imwrite("visual.bmp", visualization);

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
  
  // const int rows = 1000;
  // const int columns = 1000;
  //
  //
  // std::vector<int> vec(rows*columns, 0);
  //
  //
  // srand(10);
  // for(int i = 0; i < rows*columns; i++){
  //   vec[i] = rand() % 1000;
  //   // std::cout << vec[i] << ' ';
  // }
  //
  //
  // const auto h_start = std::chrono::high_resolution_clock::now();
  // std::vector<int> h_matching = graph_hungarian(rows, columns, vec);
  // const auto h_end = std::chrono::high_resolution_clock::now(); //End timer
  // const std::chrono::duration<double> h_elapsed_seconds{h_end-h_start};
  // std::cout << "Hungarian run in " << h_elapsed_seconds.count() <<"s\n";
  //
  // const auto a_start = std::chrono::high_resolution_clock::now();
  // std::vector<int> a_matching = auction_algorithm(rows,columns,vec);
  // const auto a_end = std::chrono::high_resolution_clock::now(); //End timer
  // const std::chrono::duration<double> a_elapsed_seconds{a_end-a_start};
  // std::cout << "Auction run in " << a_elapsed_seconds.count() <<"s\n";
  //
  // int h_val = evaluate_matching(vec, h_matching);
  // int a_val = evaluate_matching(vec, a_matching);
  // std::cout << "Hungarian: " << h_val
  //           << "\nAuction: " << a_val
  //           << std::endl;
  // // assert(h_val == a_val);
  //
  // cv::Mat new_img = visualize_matching(h_matching, 25, 40, 60, 60);
  // cv::imwrite("test.bmp", new_img);

}

void print_matching(std::vector<int>& matching){
  std::cout << "Matching:\n";
  for(int i = 0; i< matching.size(); i++){
    std::cout << "Worker " << i << " is assigned to job " << matching[i] << std::endl;
  }
  std::cout << std::endl;
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


/* TODO:
 * 4. figure out how to restitch the image back together
 * 5. implement the canny edge finding
 * 6. replace this : montage $(seq -f "%gout.bmp" 0 2303) -tile 64x36 -geometry +0+0 -font DejaVu-Sans output.jpg
 * 7. implement in GPU for real time (maybe reararange previous frame to get current frame?)
 * 8. Do the same thing but for sound; reararange song into itself or into another song
 * TESTS: Does it correctly rearrange the image into itself? Can it correctly recreate the target image if the input
 * includes every color, the square size is 1px, and repeats are allowed
 * */
