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
IplImage* drw;

// Coordinates
int x;
int y;

int counter = 0;

// Arrays

void init() {
  cap.open(0);

  cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
}

float innerAngle(float px1, float py1, float px2, float py2, float cx1, float cy1) {
  // Finding angle between defects

  float dist1 = sqrt((px1-cx1) * (px1-cx1) + (py1-cy1) * (py1-cy1));
  float dist2 = sqrt((px2-cx1) * (px2-cx1) + (py2-cy1) * (py2-cy1));

  float Ax, Ay;
  float Bx, By;
  float Cx, Cy;
  Cx = cx1;
  Cy = cy1;

  if(dist1 < dist2) {
    Bx = px1;
    By = py1;
    Ax = px2;
    Ay = py2;
  } else {
    Bx = px2;
    By = py2;
    Ax = px1;
    Ay = py1;
  }

  float Q1 = Cx - Ax;
  float Q2 = Cy - Ay;
  float P1 = Bx - Ax;
  float P2 = By - Ay;

  float A = acos((P1*Q1 + P2*Q2) / (sqrt(P1*P1+P2*P2) * sqrt(Q1*Q1+Q2*Q2)));

  A = A*180/CV_PI;
  return A;
}

void convex(vector<vector<Point> > contours, size_t largestContour) {
  // Find the hull of the biggest object

  if (!contours.empty()) {
    vector<vector<Point> > hull(1);
    convexHull(Mat(contours[largestContour]), hull[0], false);
    drawContours(src, hull, 0, cv::Scalar(0, 255, 0), 3);

    if (hull[0].size() > 2) {
      vector<int> hullIndexes;
      convexHull(Mat(contours[largestContour]), hullIndexes, true);
      vector<Vec4i> convexityDefect;
      convexityDefects(Mat(contours[largestContour]), hullIndexes, convexityDefect);
      Rect boundingBox = boundingRect(hull[0]);
      rectangle(src, boundingBox, Scalar(255, 0, 0));
      Point center = Point(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);
      vector<Point> validPoints;

      for (size_t i = 0; i < convexityDefect.size(); i++) {
        Point p1 = contours[largestContour][convexityDefect[i][0]];
        Point p2 = contours[largestContour][convexityDefect[i][1]];
        Point p3 = contours[largestContour][convexityDefect[i][2]];

        double angle = atan2(center.y - p1.y, center.x - p1.x) * 180 / CV_PI;
        double inAngle = innerAngle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
        double length = sqrt(pow(p1.x - p3.x, 2) + pow(p1.y - p3.y, 2));

        if (angle > -30 && angle < 160 && abs(inAngle) > 20 && abs(inAngle) < 120 && length > 0.1 * boundingBox.height) {
          validPoints.push_back(p1);
        }
      }

      for (size_t i = 0; i < validPoints.size(); i++) {
        circle(src, validPoints[i], 9, Scalar(0, 255, 0), 2);
      }

      // Coordinates of right-top corner
      x = boundingBox.x + boundingBox.width / 2;
      y = boundingBox.y;

      // Num of fingers
      counter = validPoints.size();
    }
  }
}

void contours() {
  // Find contours of all black objects,
  // then find the biggest one
  // and draw it

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

  convex(contours, largestContour);
}

int draw() {
  circle(src, Point(x, y), 10, Scalar(0, 125, 125), -1, 8, 0);

  string word = "Number is equal to " + to_string(counter);
  putText(src, word, Point(10, 50), FONT_HERSHEY_PLAIN, 1.5, Vec3b(255,255,255), 1);
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
    cvtColor(src, out, CV_BGR2HSV);
    inRange(out, Scalar(0, 0, 0, 0), Scalar(180, 255, 30, 0), out);

    // Find object with biggest area and draw contours
    contours();

    // Draw
    draw();

    // Show
    imshow("Video Capture", src);
    imshow("Object Detection", out);

    if (waitKey(1) == 27) break;
  }

  return 0;
}
