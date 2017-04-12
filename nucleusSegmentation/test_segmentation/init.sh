#!/bin/bash

image_name="sbubmi/test_segmentation"
container_name="$USER-test_segmentation"

# Build image from dockerfile
docker build -t "$image_name" .

# Start container
python ../script/run_docker_segment.py start $container_name $image_name

