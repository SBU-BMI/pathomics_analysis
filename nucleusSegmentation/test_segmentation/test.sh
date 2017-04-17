#!/bin/bash

#
# Assuming same container name as set in init.sh
#
container_name="$USER-test_segmentation"
input_file="TCGA-41-3393-01Z-00-DX1_18300_68910_600_600_GBM.png"

# Segment image
test1()
{
  exec_id="20170412133151"
  cwd=$(pwd)
  output_file="$cwd/test_out.zip"

  # Using Python script to run mainSegmentFeatures
  # and copy output from container to local file system.
  python ../script/run_docker_segment.py \
  segment \
  $container_name \
  $input_file \
  $output_file \
  -t img \
  -j Y \
  -s 12560,47520 \
  -b 500,500 \
  -d 500,500 \
  -a $exec_id \
  -c TCGA-CS-4938-01Z-00-DX1 \
  -p TCGA-CS-4938
}

# Small little test
test2()
{
  input_dir="/tmp/input"
  output_dir="/tmp/output"
  containerId=$(docker inspect --format '{{ .Id }}' $container_name)

  docker exec -d $container_name mkdir -p $input_dir
  docker exec -d $container_name mkdir -p $output_dir
  docker cp $input_file $container_name:$input_dir
  # Using Docker to run mainSegmentSmallImage
  docker exec -d $container_name mainSegmentSmallImage $input_dir/$input_file "$output_dir/output"
  docker cp $container_name:$output_dir/output_label.png .
  docker cp $container_name:$output_dir/processTestDeclump_label.png .
}

#test1
test2

echo "Done."
