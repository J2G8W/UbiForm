# UbiForm
UbiForm is a Ubiquitous perFormance library. It has been created as part of Julian Wreford's Part II project for Cambridge University 2020-21.
The purpose of the library is to provide a Middleware for IoT developers which gives a flexible and reconfigurable interface.

The central concept to the library is the use of the Component class, there should only need to be one Component per program as we can have multiple endpoints and everything else.
We define endpoints as the things which do the communication, there is provision for: Pair, Pub/Sub and Req/Rep communication over TCP/IPv4 and IPC.

Far more detail about the setup can be found in the dissertation produced about this middleware, which will be linked once submitted.


## Installation
In order to build the project you must include rapidjson in the libs folder and nng-master inside the CMake folder.
Additionally it requires the CMake tool (min version 3.17) and optionally ninja. 
As part of the build process it will automatically download google-test which is used as the testing framework for our system.

```
cd UbiForm/CMake
mkdir build
cd build
cmake -G Ninja ..
ninja (or make)
```

Additionally, there are three options related to cmake:
* \-DBUILD_TESTS=On          => Build the tests in the google test framework (run_tests.exe)
* \-DBUILD_EXAMPLES=On       => Build the example programs as executables (they are further documented within the examples folder)
* \-DBUILD_SOUND_EXAMPLES=On => Build the music player program (requires the installation of SFML on the host system)
* \-DBUILD_EVALUATION=On     => Build the evaluation programs which were used to evaluate the performance of the program

## Usage Examples 
There are a couple of small examples provided in the examples directory, and some more fleshed out examples in the following GitHub repos:
* [LifeHub](https://github.com/J2G8W/UbiFormLifeHub)
* [Skeleton Key Android App](https://github.com/J2G8W/UbiFormSkeletonKey)
* [Example Sensor](https://github.com/J2G8W/UbiFormExampleIoTSensor)
* [Music Streaming Android App](https://github.com/J2G8W/UbiFormAndroidMusicStreaming)

These demonstrations show the power and capability of UbiForm as a middleware. They are roughly split into two different demos:
* The LifeHub, SkeletonKey and ExampleSensor show an example setup within the home where we have flexibile data and connection
* The MusicStreaming app combines with the MusicPlayerExample and ResourceDiscoveryExample in the example folder to show how we can stream music files from an Android device with flexibile discovery

## Documentation
Documentation can be generated with doxygen:
```
doxygen Doxyfile
```

## Meta
[Julian Wreford](https://www.linkedin.com/in/julian-wreford-986b34154/) – julian \[at\] ashleywreford \[dot\] com

Distributed under the MIT license. See ``LICENSE`` for more information.
