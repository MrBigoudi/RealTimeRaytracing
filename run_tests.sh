#!/bin/bash

BUILD_DIR=build
LABEL_FILE=${BUILD_DIR}/test_labels.txt

# Default values
compile=true  # Default to compile before running
labels=()  # List of labels to filter tests

# Function to display help message
usage() {
  echo "Usage: $0 [-r] [-l <label>]... [-h|--help]"
  echo "  -r              Run only, skip compilation"
  echo "  -l <label>      Add label to filter tests (can be used multiple times)"
  echo "  -h, --help      Display this help message"
  echo ""
  echo "Available labels:"

  # Read and display available labels from the file
  if [ -f "$LABEL_FILE" ]; then
    sort -u "$LABEL_FILE" | tr ';' '\n' | sort -u | grep -v '^$' | while read -r label; do
      echo "  - $label"
    done
  else
    echo "  No labels found. Run CMake to generate the label file."
  fi
  exit 0
}

# Parse arguments
while getopts ":rl:h-:" opt; do
  case ${opt} in
    r )
      compile=false
      ;;
    l )
      labels+=("${OPTARG}")
      ;;
    h )
      usage
      ;;
    - )
      case "${OPTARG}" in
        help)
          usage
          ;;
        *)
          echo "Invalid option: --${OPTARG}" 1>&2
          exit 1
          ;;
      esac
      ;;
    \? )
      echo "Invalid option: -$OPTARG" 1>&2
      exit 1
      ;;
    : )
      echo "Invalid option: -$OPTARG requires an argument" 1>&2
      exit 1
      ;;
  esac
done

# Build the project if compile flag is true
if $compile; then
  make -C ${BUILD_DIR}
fi

# Construct the label filter string for ctest
label_filter=""
if [ ${#labels[@]} -gt 0 ]; then
  label_filter="-L ${labels[0]}"
  for label in "${labels[@]:1}"; do
    label_filter="${label_filter},${label}"
  done
else
  label_filter="-L customTests"  # Default label
fi

# Run the tests with the specified labels
ctest --test-dir ${BUILD_DIR} ${label_filter} --output-on-failure