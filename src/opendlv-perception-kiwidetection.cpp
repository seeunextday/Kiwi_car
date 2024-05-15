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

void followTarget(const cv::Point &center, int radius, cluon::OD4Session &od4) {
    float steeringAngle = (center.x - 320) * 0.005; // Assuming 640 is the width of the image
    float speed = std::max(0.1f, 1.0f - (radius / 100.0f)); // Speed control based on the size of the detected circle

    opendlv::proxy::GroundSteeringRequest gsr;
    gsr.groundSteering(steeringAngle);
    od4.send(gsr);

    opendlv::proxy::PedalPositionRequest ppr;
    ppr.position(speed);
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

      std::vector<cv::Vec3f> circles;
      kiwiDetector.houghCircles(grayimg, circles);
      //std::cout << "nr of circles detected: " << circles.size() << std::endl;

      }
      for (const auto &circle : circles) {
            cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
            int radius = cvRound(circle[2]);
            followTarget(center, radius, od4);
        }

        if (cmd.count("verbose")) {
            for (const auto &circle : circles) {
                cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
                int radius = cvRound(circle[2]);
                cv::circle(img, center, 3, cv::Scalar(0, 255, 0), -1);
                cv::circle(img, center, radius, cv::Scalar(0, 0, 255), 3);
            }
            cv::imshow("Detected Kiwis", img);
            cv::waitKey(10);
        }
    }
    return 0;
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