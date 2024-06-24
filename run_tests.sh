#!/bin/bash

BUILD_DIR=build

make -C ${BUILD_DIR}
ctest --test-dir ${BUILD_DIR} --rerun-failed --output-on-failure