# Project

A real time raytracer following nvidia papers

## HOWTO

To run this project, first clone the github project:
```sh
git clone git@github.com:MrBigoudi/RealTimeRaytracing.git
```

From there, clone the submodules:
```sh
cd RealTimeRaytracing
git submodule update --init --recursive
```

Then launch the project
```sh
./run_app.sh --help
./run_app.sh
```

You can also run tests with
```sh
./run_tests.sh --help
./run_tests.sh
```