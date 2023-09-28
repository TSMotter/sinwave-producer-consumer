# Sinwave producer consumer with websocket
- This is a demonstration of a producer consumer problem in C++
- The project consists of a python script that sets up a websocket that streams samples of a sinusoidal wave and a C++ application that connects to this websocket, and consumes these samples in order to represent them on the console as if they represented the movement of a device in the X axis
- The project can be looked at as a producer-consumer problem in 2 different perspectives:
    - Websocket as producer of data and C++ app as consumer of that data
    - C++ app thread websocket thread as producer of data and C++ app thread that draws to console as consumer of that data
- A list of dependencies and a small guide can be found in the following sections
- Read the `scripts/readme.md` for more information about the scripts
___

## How to operate the repository
- Execute the `scripts/stream_sin_wave.py` python script in order to setup a websocket streaming samples of a sinusoidal wave
```shell
python scripts/stream_sin_wave.py -op JSON_TO_WEBSOCKET -c channel_200_2_1 -l DEBUG
```
- Compile and execute the C++ application in another terminal
```shell
./bbuild.sh -r -e app
```

## How to compile and execute the application
- If you wish to use docker, build the image and launch it using the helper scripts inside of the `docker` folder
- The repository can be operated outside of the docker container if all the dependencies are met (there is a list of the dependencies and it's versions in the end of this readme)
- Once the environment is set (whether inside or outside the container), the following commands can be issued:

- To format the code base with clang-format:
```bash
./bbuild.sh -f
```

- To perform an static analysis in the code base with clang-tidy:
```bash
./bbuild.sh -s
```

- To build:
```bash
./bbuild.sh -b <target>
```

- To rebuild:
```bash
./bbuild.sh -r <target>
```

- To execute the built binary:
```bash
./bbuild.sh -e <target>
```

- To format, analyze, rebuild and execute with verbose turned ON:
```bash
./bbuild.sh -v -f -s -r -e <target>
```
- Example: `./bbuild.sh -v -f -s -r -e app`
- Example: `./bbuild.sh -v -f -s -r -e test`

- To generate doxygen documentation (generated docs will be available at `build/documentation/html/index.html`):
```bash
./bbuild.sh -r documentation
```

- To check all options available::
```bash
./bbuild.sh --help
```

___

## This project uses:
- **cmake** & **make** are used as build system
- **clang-format** is used as formatter/code beautifier
- **doxygen** is used to generate documentation
- **gtest** is downloaded in build-time into the `build/_deps` folder and provides infrastructure for unit tests
    - Link: [github.com/google/googletest](https://github.com/google/googletest)
- **easywsclient** is incorporated in the `external` folder and provides websocket functionalities
    - Link: [github.com/dhbaird/easywsclient](https://github.com/dhbaird/easywsclient)
- **nlohmann/json** is incorporated in the `external` folder and provides json functionalities
    - Link: [github.com/nlohmann/json](https://github.com/nlohmann/json)
- **matplotlib** is a python visualization library that is used to plot the points received from the webserver
    - Link: [matplotlib.org](https://matplotlib.org/)

### Versions present in development machine:
- **cmake:** cmake version 3.22.1
- **make:** GNU Make 4.3
- **clang-format:** Ubuntu clang-format version 14.0.0-1ubuntu1
- **doxygen:** 1.9.1
- **gtest:** 1.13.0
- **easywsclient:** Commit hash: `afc1d8c`
- **nlohmann/json:** 3.11.2
- **matplotlib:** 3.7.1
