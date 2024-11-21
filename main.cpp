#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <omp.h>
#define SQSIZE 60
#define SQWIDTH 32
#define SQHEIGHT 18

const static int AREA = SQWIDTH*SQHEIGHT;
const static int AREA_S = AREA*8;

int cost(cv::Mat, cv::Mat, char method = ' ');
int diff(cv::Scalar, cv::Scalar, char method = ' ');
cv::Mat transform(cv::Mat, bool reflect = 0, int rotate = 0);
void disect(cv::Mat, cv::Mat[], bool);
void optimize(int score[AREA][AREA_S], int output[AREA], char mode = 'm');

int main (int argc, char *argv[]){
  const auto start = std::chrono::high_resolution_clock::now(); //Start timer
  //read file
  std::string file2  = "../images/bmp/racoon.bmp";
  std::string file = "../images/bmp/sunset_tram.bmp";

  if(argc > 1) file = argv[1];
  if(argc > 2) file2 = argv[2];
  std::cout<<"reading "<<file<< " " <<file2<<'\n';

  cv::Mat img = cv::imread(file);
  cv::Mat img2 = cv::imread(file2);
  if (img.empty()||img2.empty()) {
      std::cerr << "Image not loaded!" << std::endl;
      return -1;
  }

  int table[AREA][AREA_S];
  cv::Mat squares[AREA_S];
  cv::Mat tiles[AREA];
  disect(img, squares, true);
  disect(img2, tiles, false);

  #pragma omp parallel for collapse(2)
  for(int i = AREA-1; i>=0;i--){
    for(int j = AREA_S-1;j>=0;j--){
      //std::cout << "w\n";
      table[i][j]=cost(squares[j],tiles[i]);
    }
  }

  int out[AREA];
  optimize(table, out);
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

cv::Mat transform(cv::Mat img, bool reflect, int rotate){
  //reflect then rotate 
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
  return img;
}


void disect(cv::Mat img, cv::Mat squares[], bool symetries){
  for(int i=SQHEIGHT-1; i>=0; i--){
    int yindex = i*SQSIZE;
    for(int j=SQWIDTH-1; j>=0; j--){
      int xindex = j*SQSIZE;
      cv::Mat tmp(cv::Size(SQSIZE,SQSIZE),CV_8UC3,cv::Scalar(0,0,0));
      int square = i*SQWIDTH+j;
      squares[square]=tmp;
      for(int k=SQSIZE-1; k>=0; k--){
        for(int l=SQSIZE-1;l>=0; l--){
          squares[square].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(yindex+k, xindex+l);
          //squares[i][j].at<cv::Vec3b>(k,l)=img.at<cv::Vec3b>(i, j);
        }
      }
    }
  }
  if(!symetries) return;

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
      break;
    }
    case 'h':{ // hungarian algorithm (optimal no-repeat solution)
      break;
    }
  }
}


/* TODO
 * 3. implement hungarian alg
 * 4. figure out how to restitch the image back together
 * 5. implement the canny edge finding
 *
 * */
