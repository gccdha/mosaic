#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <cmath>
#include <opencv2/opencv.hpp>
// #include <getopt.h>
// #include <omp.h>
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

int main (int argc, char *argv[]){
  
  const auto start = std::chrono::high_resolution_clock::now(); //Start timer
  //Parse command line arguments
  if(argc <= 2) std::cout << "ERROR: expected 2 arguments" << std::endl;
  else return -1;

  std::string file = argv[1];
  std::string file2 = argv[2];
  std::cout<<"reading "<<file<< " " <<file2<<'\n';

  //Read files. img is source and img2 is targt TODO: rename vars to reflect this
  cv::Mat img = cv::imread(file);
  cv::Mat img2 = cv::imread(file2);
  if (img.empty()||img2.empty()) {
      std::cerr << "Image not loaded!" << std::endl;
      return -1;
  }

  //Extract into array
  cv::Mat squares[AREA_S];
  cv::Mat tiles[AREA];
  disect(img, squares, true);
  disect(img2, tiles, false);

  //Generate cost matrix for each pairing of input and output:
  int table[AREA][AREA_S];
  #pragma omp parallel for collapse(2)
  for(int i = AREA-1; i>=0;i--){
    for(int j = AREA_S-1;j>=0;j--){
      table[i][j]=cost(squares[j],tiles[i]);
    }
  }
  
  //Find optimal solution and output squares in corresponding order (need to be constructed externally with imagemagick)
  //TODO: find a way to stitch images back together without outputting them
  int out[AREA];
  optimize(table, out, 'g');
  cv::Mat new_img = img2.clone();
  for(int i = 0; i<AREA; i++){
    //std::cout << i << ":  " <<(i/SQWIDTH)*SQSIZE << "  " <<(i/SQWIDTH)*SQSIZE+119 << "  " << (i%SQWIDTH)*SQSIZE << "  " <<(i%SQWIDTH)*SQSIZE+119 << '\n';
    cv::Mat roi = new_img(cv::Range((i/SQWIDTH)*SQSIZE,(i/SQWIDTH)*SQSIZE+SQSIZE-1),cv::Range((i%SQWIDTH)*SQSIZE,(i%SQWIDTH)*SQSIZE+SQSIZE-1));
    squares[out[i]].copyTo(roi);
    cv::imwrite("output/"+ std::to_string(i)+"out.bmp", squares[out[i]]);
  }

  cv::imwrite("test.bmp", new_img);
  

  
  //img.at<cv::Vec3b>(1000,0) = {0,0,255};
  //cv::putText(img, "Graphic design is my passion", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
  //cv::imwrite("test.bmp", transform(img,1,3));

  /*for(int i=0; i<(16*9*8*16*9); i++){
  std::cout << "pixel: " << cost(squares[1][1], squares[5][9]);
  std::cout << "\n avg:"<< cost(squares[1][1], squares[5][9], 'a');
  std::cout << '\n' << i << std::endl;
  }*/

  const auto end = std::chrono::high_resolution_clock::now(); //End timer
  const std::chrono::duration<double> elapsed_seconds{end-start};
  std::cout << "run in " << elapsed_seconds.count() <<"s\n";
  //system("feh test.bmp");
  //system("feh -i *out.bmp");
  return 0;
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


/* TODO:
 * 3. implement hungarian alg
 * 4. figure out how to restitch the image back together
 * 5. implement the canny edge finding
 * 6. replace this : montage $(seq -f "%gout.bmp" 0 2303) -tile 64x36 -geometry +0+0 -font DejaVu-Sans output.jpg
 * 7. implement in GPU for real time (maybe reararange previous frame to get current frame?)
 * 8. Do the same thing but for sound; reararange song into itself or into another song
 * */
