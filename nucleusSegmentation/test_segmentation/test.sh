#!/bin/bash

container_name="$USER-test_segmentation"
exec_id="20170412133151"
input_file="TCGA-CS-4938-01Z-00-DX1_12560_47520_500_500_LGG.png"
output_file="$HOME/test_out.zip"

# Segment image
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

