#!/bin/bash

container_name="$USER-test_segmentation"
exec_id="20170412133151"
input_file="TCGA-CS-4938-01Z-00-DX1_12560_47520_500_500_LGG.png"
output_file="$HOME/test_out.zip"
input_dir="/tmp/input"
output_dir="/tmp/output"

# Segment image
test1()
{
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
  docker exec $container_name mkdir -p $input_dir
  docker exec $container_name mkdir -p $output_dir
  input_file="TCGA-CS-4938-01Z-00-DX1_12560_47520_500_500_LGG.png"
  docker cp $input_file $container_name:$input_dir
  docker exec $container_name mainSegmentSmallImage $input_dir/$input_file "$output_dir/output"
  docker cp $container_name:$output_dir/output_label.png .
  docker cp $container_name:$output_dir/processTestDeclump_label.png .
}

#test1
test2

echo "Done."
