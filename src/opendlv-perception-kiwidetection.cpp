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



int32_t main(int32_t argc, char **argv)
{
  auto cmd = cluon::getCommandlineArguments(argc, argv);
  if ((0 == cmd.count("cid")) || (0 == cmd.count("name")) ||
      (0 == cmd.count("width")) || (0 == cmd.count("height")) ||
      (0 == cmd.count("hue-low")) || (0 == cmd.count("hue-high")) ||
      (0 == cmd.count("saturation-low")) || (0 == cmd.count("saturation-high")) ||
      (0 == cmd.count("value-low")) || (0 == cmd.count("value-high")) ||
      (0 == cmd.count("property")) ||
      (0 == cmd.count("dilations")) || (0 == cmd.count("erosions")))
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
  KiwiDetector kiwiDetector(cmd);

  // Construct detector
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

    }
  }
  return 0;
}