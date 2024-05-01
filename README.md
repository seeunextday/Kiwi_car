# OpenDLV microservice to detect blue and yellow cones in a video stream

This microservice reads image data from a shared memory resource and uses some of 
OpenCV's built-in functions to detect the contours of blue and yellow cones in the images.
For each frame it sends messages from the detection data using an od4 session from the libcluon message library.
It is part of the OpenDLV architecture for data-driven cyberphysical systems.

[![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)

## Interface

The microserivce uses OpenDLV messages with the `libcluon` middle-ware. The
standardized OpenDLV messages can be found [![here]](https://git.opendlv.org/core/opendlv-message).

### Input

This microserice triggers on the following inputs, with recommneded frequency:
* [opendlv::proxy::PedalPositionRequest (any frequency)]

[This microservice does not have any inputs.]

### Output

[This microservice generates the following outputs, with frequency:]

[This microservice does not have any outputs.]

## Dependencies

This microservice uses Docker to manage a dependency-free runtime environment.
See the `Dockerfile` for more information.

## Build

The following Docker command will build the microservice:
```bash
docker build -t registry.opendlv.org/community/opendlv-template-microservice-basic:local .
```

The microservice is automatically built for the following architectures (as part
of the GitLab CI/CD pipeline):
* `linux/amd64`
* `linux/arm64`
* `linux/arm/v7`

## Usage

The following Docker command will run the microservice:
```bash
docker run -ti --init --rm --net=host registry.opendlv.org/community/opendlv-template-microservice-basic:local --cid=111 --verbose
```
Note that the `local` tag only exists if first building locally. In most cases
use the latest pre-built image from the GitLab registry. Find all available tags
[![here]](https://git.opendlv.org/community/opendlv-template-microservice-basic/container_registry).

### Arguments

This microservice expects the following arguments:
* `--cid=<int>`: The OD4 conference id.
* `--verbose`: More print-out.

## License

* This project is released under the terms of the GNU GPLv3 License

