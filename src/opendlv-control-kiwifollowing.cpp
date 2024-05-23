/*
 * Copyright (C) 2024 OpenDLV
 */

#include <cmath>
#include <cstdint>
#include <iostream>

#include "cluon-complete.hpp"
#include "opendlv-message-standard.hpp"


int32_t main(int32_t argc, char **argv)
{
  int32_t retCode{1};
  auto cmd = cluon::getCommandlineArguments(argc, argv);
  if (!cmd.contains("cid")) {
    std::cout << argv[0] << " is an OpenDLV microservice." << std::endl;
    std::cout << "Usage: " << argv[0] << " "
              << "--cid=<conference id; e.g. 111> "
              << "[--verbose] " << std::endl;
    return 0;
  } else {
    // Interface to a running OD4 session; here, you can send and receive messages.
    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(cmd["cid"]))};
    // uint16_t const cid = std::stoi(cmd.at("cid"));

    bool const verbose = (cmd.count("verbose") != 0);
    uint32_t const imageWidth{static_cast<uint32_t>(std::stoi(cmd["width"]))};
    uint32_t const imageHeight{static_cast<uint32_t>(std::stoi(cmd["height"]))};
    float const pedalPosition{static_cast<float>(std::stoi(cmd["pedal-position"]))};
    
    std::mutex directionMutex;
    float detectionAzimuthAngle;
    auto onDetectionDirection{[&verbose, &detectionAzimuthAngle, &directionMutex](cluon::data::Envelope &&envelope) {
      auto const dir =
          cluon::extractMessage<opendlv::logic::perception::DetectionDirection>(
              std::move(envelope));

      std::lock_guard<std::mutex> lck(directionMutex);
      detectionAzimuthAngle =  dir.azimuthAngle();
    }};
    od4.dataTrigger(opendlv::logic::perception::DetectionDirection::ID(), onDetectionDirection);

    std::mutex detectionDistanceMutex;
    float detectionDistance;
    auto onDetectionDistance{[&verbose, &detectionDistanceMutex, &detectionDistance](cluon::data::Envelope &&envelope) {
      auto const dist = 
          cluon::extractMessage<opendlv::logic::perception::DetectionDistance>(
              std::move(envelope));

      std::lock_guard<std::mutex> lck(detectionDistanceMutex);
      detectionDistance = dist.distance();
    }};
    od4.dataTrigger(opendlv::logic::perception::DetectionDistance::ID(), onDetectionDistance);

    // Endless loop; end the program by pressing Ctrl-C.
    while (od4.isRunning()) {
      opendlv::proxy::GroundSteeringRequest groundSteeringRequest;
      groundSteeringRequest.groundSteering(detectionAzimuthAngle);
      od4.send(groundSteeringRequest);

      opendlv::proxy::PedalPositionRequest ppr;

      if (detectionDistance > 18){
        ppr = 0.0f; // if car is too close, stop
      }
      if (detectionDistance > 13) {
        ppr = (0.6f - (detectionDistance / 100.0f)); // Speed control based on the size of the detected circle
      }
      else {
        ppr = 0.6f; // Speed control based on the size of the detected circle
      }
      od4.send(ppr);
    }
    retCode = 0;
  }
  return retCode;
}