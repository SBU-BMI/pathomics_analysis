#!/bin/bash

# Program name
PROGNAME=$(basename "$0")

# Error trapping
error_exit() {
  echo "${PROGNAME}: ${1:-"Error"}" 1>&2
  exit 1
}

#
# Assuming same container name as set in init.sh
#
container_name="$USER-pathomics_analysis"
containerId="$(docker inspect --format '{{ .Id }}' $container_name)"
input_file="TCGA-41-3393-01Z-00-DX1_18300_68910_600_600_GBM.png"
input_dir="/tmp/input"
output_dir="/tmp/output"
cwd="$(pwd)"

# Segment image
test1()
{
  output_file="$cwd/test_out.zip"
  exec_id="20170412133151"

  # Using Python script to run mainSegmentFeatures
  # and copy output from container to local file system.
  python ../script/run_docker_segment.py \
  segment \
  "$container_name" \
  "$input_file" \
  "$output_file" \
  -t img \
  -j 2 \
  -s 12560,47520 \
  -b 500,500 \
  -d 500,500 \
  -a "$exec_id" \
  -c TCGA-CS-4938-01Z-00-DX1 \
  -p TCGA-CS-4938
}

# Small little test
test2()
{
  output_file="$output_dir/output_label.png"

  echo "Copying input file to docker"
  docker cp "$input_file" "$container_name":"$input_dir"
  #docker exec "$containerId" ls -lt "$input_dir"

  # Using Docker to run mainSegmentSmallImage
  echo "Running segmentation"
  docker exec -d "$container_name" mainSegmentSmallImage "$input_dir/$input_file" "$output_dir/output" || error_exit "Error running mainSegmentSmallImage"
  sleep 15
  #docker exec "$containerId" ls -lt "$output_dir"
  docker cp "$containerId":"$output_file" "$cwd" || error_exit "Could not download output_label.png"
  #while [ ! -f "$(docker exec $containerId ls $output_file)" ] ;
  #do
  #  echo "sleeping..."
  #  sleep 2
  #done
  #docker exec "$containerId" ls "$output_dir"
}

# Features
test3()
{
  mydate=$(date +"%m-%d-%Y")
  file="TCGA-CS-4938-01Z-00-DX1_12560_47520_500_500_LGG"

  input_file1="$file.png"
  input_file2="$file-mask.png"

  echo "Copying input files to docker"
  docker cp "$input_file1" "$container_name":"$input_dir"
  docker cp "$input_file2" "$container_name":"$input_dir"

  echo "Computing features"
  docker exec -d "$container_name" computeFeatures "$input_dir/$input_file1" "$input_dir/$input_file2" Y $output_dir/output_Y_$mydate.csv 12560 47520
  sleep 3
  docker cp "$containerId:$output_dir/output_Y_$mydate.csv" .

}

echo "Creating input and output dirs"
docker exec -d "$container_name" mkdir -p "$input_dir"
docker exec -d "$container_name" mkdir -p "$output_dir"
#docker exec "$containerId" ls -lt "/tmp"

echo ""
echo "test1"
test1

echo ""
echo "test2"
test2

echo ""
echo "test3"
test3

echo "Done."
