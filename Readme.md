# UbiForm
UbiForm is a Ubiquitous perFormance library. It has been created as part of Julian Wreford's Part II project for Cambridge University 2020-21.
The purpose of the library is to provide a Middleware for IoT developers which gives a flexible and reconfigurable interface.

The central concept to the library is the use of the Component class, there should only need to be one Component per program as we can have multiple endpoints and everything else.
We define endpoints as the things which do the communication, there is provision for: Pair, Pub/Sub and Req/Rep communication over TCP/IPv4 and IPC.

In order to build the project you must include rapidjson in the libs folder and nng-master inside the CMake folder.
Additionally it requires the CMake tool (min version 3.17) and optionally ninja. 
As part of the build process it will automatically download google-test which is used as the testing framework for our system.

```
cd UbiForm/CMake
mkdir build
cd build
cmake ..
ninja (or make)
```

There are a couple of small examples provided in the examples directory, and some more fleshed out examples in the ___ GitHub repos.

Documentation can be generated with doxygen:
```
doxygen Doxyfile
```

And this should answer most things about what is happening