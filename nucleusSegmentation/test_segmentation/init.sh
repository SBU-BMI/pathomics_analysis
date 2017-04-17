#!/bin/bash

#
# Run this program from the directory where the Dockerfile is located.
#

# Program name
PROGNAME=$(basename "$0")

# Error trapping
error_exit() {
   echo "${PROGNAME}: ${1:-"Error"}" 1>&2
   exit 1
}

image_name="sbubmi/test_segmentation"
container_name="$USER-test_segmentation"

# Build image from dockerfile
docker build -t "$image_name" . || error_exit "Could not build container."

# Start container
python ../script/run_docker_segment.py start $container_name $image_name
