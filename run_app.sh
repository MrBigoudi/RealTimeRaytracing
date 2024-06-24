#!/bin/bash

BUILD_DIR=build
MAIN_PRGM=nvidiaRaytracing

make -C ${BUILD_DIR}
./${BUILD_DIR}/${MAIN_PRGM}
