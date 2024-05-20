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

void KiwiDetector::houghCircles(cv::Mat &grayscale, std::vector<cv::Vec3f> &circles) const
{
  // cv::HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1, gray.rows/100, 50, 30, 0, 20);
  
  cv::HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, m_dp, m_minDist, m_cannyThreshold, m_accumulatorThreshold, m_minRadius, m_maxRadius);

}

// angle is wrong direction
void KiwiDetector::followTarget(cv::Point2f &center, float radius, cluon::OD4Session &od4) const
{
    // float steeringAngle = (center.x - 320.0f) * 0.005f; // Assuming 640 is the width of the image
    // float steeringAngle = (m_width/2 - center.x)*0.005f;
    float steeringAngle = std::atan((m_width / 2 - center.x) / (m_height - center.y));
    float speed = std::max(0.5f, 0.5f - (radius / 100.0f)); // Speed control based on the size of the detected circle

    opendlv::proxy::GroundSteeringRequest gsr;
    gsr.groundSteering(steeringAngle);
    od4.send(gsr);
    std::cout << "Angle request: " << gsr.groundSteering() << std::endl;

    opendlv::proxy::PedalPositionRequest ppr;
    ppr.position(speed);
    od4.send(ppr);
    std::cout << "Speed request: " << ppr.position() << std::endl;
}

void KiwiDetector::lookAround(cluon::OD4Session &od4) const
{
  
  // turn angle = 19 deg
  opendlv::proxy::GroundSteeringRequest gsr;
  gsr.groundSteering(0.33f);
  od4.send(gsr);

  // speed = 0.5
  opendlv::proxy::PedalPositionRequest ppr;
  ppr.position(0.5f);
  od4.send(ppr);
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

    cv::Mat grayimg;
    std::vector<cv::Vec3f> circles; //circle coords (x, y, radius) floats
    
    std::vector<cv::Vec3f> lastDetection;
    uint32_t framesSinceDetection = 0;


    // Endless loop; end the program by pressing Ctrl-C.
    // TODO: move towards circle coodinates
    // TODO: add counter for frames, i.e if x frames without circles = car lost
    // TODO: if circles are moving sideways, follow
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

      // change image to grayscale, needed for HoughCircles
      cv::cvtColor(img, grayimg, cv::COLOR_BGR2GRAY);
      // std::cout << "image width: " << grayimg.cols << std::endl;

      // tutorial said to blur to avoid extra circles, not sure if needed
      //cv::medianBlur(gray, gray, 5);

      kiwiDetector.houghCircles(grayimg, circles);
      //std::cout << "nr of circles detected: " << circles.size() << std::endl;

      //move towards the first detected circle
      if (circles.size() > 0) {
    
         lastDetection = circles;
        framesSinceDetection = 0;

  
        cv::Point2f center(static_cast<float>(circles[0][0]), static_cast<float>(circles[0][1]));
        float radius = static_cast<float>(circles[0][2]);

  
        kiwiDetector.followTarget(center, radius, od4);
      } else {
   
        framesSinceDetection++;


        if (framesSinceDetection > 20 && !lastDetection.empty()) {
          for(const auto& circle : lastDetection) {
              std::cout << "Last detection: " << circle[0] << " " << circle[1] << " " << circle[2] << std::endl;
          }
        
            cv::Point2f center(static_cast<float>(lastDetection[0][0]), static_cast<float>(lastDetection[0][1]));
            float radius = static_cast<float>(lastDetection[0][2]);
            kiwiDetector.followTarget(center, radius, od4);
        } else if (framesSinceDetection <= 20) {
        
            if (!lastDetection.empty()) {
               cv::Point2f center(static_cast<float>(lastDetection[0][0]), static_cast<float>(lastDetection[0][1]));
               float radius = static_cast<float>(lastDetection[0][2]);
              kiwiDetector.followTarget(center, radius, od4);
           }
        } else {
        
            kiwiDetector.lookAround(od4);
         }
  }

      // draw the circles if verbose
      if (kiwiDetector.getVerbose()) {
          for (const auto &circle : circles) {
              cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
              int radius = cvRound(circle[2]);
              cv::circle(img, center, 3, cv::Scalar(0, 255, 0), -1);
              cv::circle(img, center, radius, cv::Scalar(0, 0, 255), 3);
          }
          cv::imshow("Detected Kiwis", img);
          cv::waitKey(10);
        }
        circles.clear();
    }
  }
    return 0;
}