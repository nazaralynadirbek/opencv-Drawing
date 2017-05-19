#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace cv;

// Global variables
VideoCapture cap;

// Mats
Mat src;
Mat out;

void init() {
  cap.open(0);

  cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
}

void contours() {
  size_t largestContour = 0;
  vector<Vec4i> hierarchy;
  vector<vector<Point> > contours;

  findContours(out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
  for (size_t i = 1; i < contours.size(); i++) {
    if (contourArea(contours[i]) > contourArea(contours[largestContour])) {
      largestContour = i;
    }
  }

  drawContours(src, contours, largestContour, Scalar(0, 0, 255), 1);
}

int main() {
  init();

  namedWindow("Video Capture", WINDOW_AUTOSIZE);
  namedWindow("Object Detection", WINDOW_AUTOSIZE);

  for (;;) {
    cap >> src;

    if (src.empty()) {
      break;
    }

    // Detect black objects
    inRange(src, Scalar(0, 0, 0, 0), Scalar(180, 255, 30, 0), out);

    // Find object with biggest area and draw contours
    contours();

    // Show
    imshow("Video Capture", src);
    imshow("Object Detection", out);

    if (waitKey(1) == 27) break;
  }

  return 0;
}
