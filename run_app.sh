#!/bin/bash

BUILD_DIR=build
MAIN_PRGM_OPENGL=srcOpenGL/appVersionOpenGL
MAIN_PRGM_VULKAN=srcVulkan/appVersionVulkan

# Default to OpenGL
use_opengl=true

# Function to display help message
usage() {
  echo "Usage: $0 [-v] [-g] [-h]"
  echo "  -v        Use Vulkan version"
  echo "  -g        Use OpenGL version (default)"
  echo "  -h, --help Display this help message"
  exit 0
}

# Parse arguments
while getopts ":vgh-:" opt; do
  case ${opt} in
    v )
      use_opengl=false
      ;;
    g )
      use_opengl=true
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

# Build the project
make -C ${BUILD_DIR}

# Run the appropriate program
if $use_opengl; then
  ./${BUILD_DIR}/${MAIN_PRGM_OPENGL}
else
  ./${BUILD_DIR}/${MAIN_PRGM_VULKAN}
fi
