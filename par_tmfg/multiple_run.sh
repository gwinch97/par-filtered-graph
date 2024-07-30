#!/bin/bash

# Directory containing the dataset files
DATASET_DIR="../datasets/time-test-20000"
# Output directory
OUTPUT_DIR="outputs/time-test-20000"
# Constant parameters
N=20000
DISTANCE_FILENAME=0
METHOD="exact"
THRESHOLD=1
ROUND=1
# Ensure the tmfg executable is available
if ! [ -x ./tmfg ]; then
    echo "Error: tmfg executable not found or not executable."
    exit 1
fi

# Create the output directory if it does not exist
mkdir -p $OUTPUT_DIR

# Loop through each file in the dataset directory
for file in $DATASET_DIR/*; do
    # Get the base name of the file (without the directory path)
    base_filename=$(basename "$file" .bin)
	
	time_file="times/${base_filename}-time.txt"
	
    # Run the tmfg command with the current file
    # ./tmfg "$file" "$OUTPUT_DIR/$base_filename" $N $DISTANCE_FILENAME $METHOD $THRESHOLD $ROUND
	{ time ./tmfg "$file" "$OUTPUT_DIR/$base_filename" $N $DISTANCE_FILENAME $METHOD $THRESHOLD $ROUND; } 2> "$time_file"

    # Check if the command was successful
    #if [ $? -ne 0 ]; then
    #    echo "Error processing file: $file"
    #else
    #    echo "Successfully processed file: $file"
    #fi
done
