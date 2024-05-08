/*
 * Copyright (C) 2024 OpenDLV
 */

#include <cmath>
#include <type_traits>
#include <algorithm>
#include <cstdint>

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>

#include "cluon-complete.hpp"
#include "opendlv-message-standard.hpp"

#include "opendlv-perception-kiwidetection.h"

// void KiwiDetector::cannyEdge(cv::Mat &image, cv::Mat &cannyImage) const
// {
//   // treshold vals need tuning
//   cv::Canny(image, cannyImage, 150, 200, 3);
//   cv::Mat dilatedImage;
// }

// void KiwiDetector::findContours(cv::Mat &image, std::vector<std::vector<cv::Point>> &contours) const
// {
//   std::vector<cv::Vec4i> hierarchy;
//   cv::findContours(image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
// }

void KiwiDetector::houghCircles(cv::Mat &grayscale, std::vector<cv::Vec3f> &circles) const
{
  // cv::HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1, gray.rows/100, 50, 30, 0, 20);
  
  cv::HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, m_dp, m_minDist, m_cannyThreshold, m_accumulatorThreshold, m_minRadius, m_maxRadius);

}


int32_t main(int32_t argc, char **argv)
{
  auto cmd = cluon::getCommandlineArguments(argc, argv);
  if ((0 == cmd.count("cid")) || (0 == cmd.count("name")) ||
      (0 == cmd.count("width")) || (0 == cmd.count("height")) ||
      (0 == cmd.count("dp")) || (0 == cmd.count("minDist")) ||
      (0 == cmd.count("cannyThreshold")) || (0 == cmd.count("accumulatorThreshold")) ||
      (0 == cmd.count("minRadius")) || (0 == cmd.count("maxRadius")))
  {
    std::cout << argv[0]
              << " attaches to a shared memory area containing an ARGB image."
              << std::endl;
    std::cout << "Usage:   " << argv[0] << " "
              << "--cid=<OD4 session> --name=<name of shared memory area> "
                 "--width=<width of the video> --height=<height of the video> "
                 "--hue-low=<0-180> --hue-high=<0-180> --saturation-low=<0-255> "
                 "--saturation-high=<0-255> --value-low=<0-255> --value-high=<0-255> "
                 "--property=<color of detection> --dilations=<number of dilation iterations> "
                 "--erosions=<number of erosion iterations> [--verbose]"
              << std::endl;
    std::cout << "Example: " << argv[0] << " "
              << "--cid=111 --name=img.argb --width=640 --height=480 --property=green --hue-low=35 --hue-high=85 "
                 "--saturation-low=50 --saturation-high=255 --value-low=50 --value-high=255 --dilations=5 --erosions=5 --verbose "
              << std::endl;
    return 1;
  }

  // Construct detector
  KiwiDetector kiwiDetector(cmd);
  int32_t const format{kiwiDetector.getFormat()};
  uint32_t const width{kiwiDetector.getWidth()};
  uint32_t const height{kiwiDetector.getHeight()};

  // Attach to the shared memory.
  std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{kiwiDetector.getName()}};
  if (sharedMemory && sharedMemory->valid())
  {
    std::clog << argv[0] << ": Attached to shared memory '"
              << sharedMemory->name() << " (" << sharedMemory->size()
              << " bytes)." << std::endl;

    // Interface to a running OD4 session; here, you can send and receive messages.
    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(cmd["cid"]))};

    // cv::Mat cannyImg;
    // std::vector<std::vector<cv::Point>> contours;
    //std::vector<cv::Point> middlePointsOfContours;
    //cv::Scalar contourColor(0, 0, 255);
    

    cv::Mat grayimg;
    std::vector<cv::Vec3f> circles; //circle coords (x, y, radius) floats

    ////Uncomment is canny is wanted istead of rgb
    // cv::Mat cannyimg;
    // cv::Mat cannyImage;


    // Endless loop; end the program by pressing Ctrl-C.
    while (od4.isRunning())
    {
      cv::Mat img;

      // Wait for a notification of a new frame.
      sharedMemory->wait();

      // Lock the shared memory.
      sharedMemory->lock();
      {
        // Copy image into cvMat structure.
        cv::Mat wrapped(height, width, format, sharedMemory->data());
        img = wrapped.clone();
      }
      sharedMemory->unlock();

      // copy to be used for canny edge, comment if rgb is wanted
      //img.copyTo(cannyimg);
      // // Canny edge detection, uncomment if rgb is wanted
      //cv::Canny(cannyimg, cannyImage, 100, 200, 3);

      // change image to grayscale, needed for HoughCircles
      cv::cvtColor(img, grayimg, cv::COLOR_BGR2GRAY);
      // std::cout << "image size: " << grayimg.cols << std::endl;

      // tutorial said to blur to avoid extra circles, not sure if needed
      //cv::medianBlur(gray, gray, 5);


      kiwiDetector.houghCircles(grayimg, circles);
      //std::cout << "nr of circles detected: " << circles.size() << std::endl;

      // find distance between x-coords of circle centers
      std::vector<cv::Point> centers;
      
      if(circles.size() > 1){
        for( size_t i = 0; i < circles.size(); i++ ){
          centers = cv::Point(cvRound(circles[i][0]), cvRound(circles[i][1]));
          //centers[i] = center;
          // cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
          // std::cout << "circle center: " << center << std::endl;
          
          if(i > 0){
            // uint32_t centerDistance = cv::abs(centers[i-1].x - centers[i].x);
            // std::cout << "nr of circles detected: " << circles.size() << std::endl;
            // std::cout << "distance circle centers: " << centerDistance << std::endl;
          }
        }
      }

      // draw the circles
      for( size_t i = 0; i < circles.size(); i++ )
      {
          cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
          int radius = cvRound(circles[i][2]);
          // draw the circle center
          cv::circle( img, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
          // draw the circle outline
          cv::circle( img, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
      }
      if (kiwiDetector.getVerbose())
      {
        cv::namedWindow( "circles", 1 );
        cv::imshow( "circles", img );
        cv::waitKey(1);
      }

    }
  }
  return 0;
}