# Example programs
This is a quick document explaining what the different 
example programs do. These have served as a kind of 
system test throughout development.

These are all built with the `BUILD_EXAMPLES` flag on 
the CMake build. Apart from the MusicPlayer which is 
built when `BUILD_SOUND_EXAMPLES` flag is set.

## Pair Connection
A minimal example using the pair communication paradigm 
with both blocking and asynchronous receive. 

Different execution setups:
- `pair_example SENDER`, this will listen on tcp://127.0.0.
  1:8000 for a connection. And once it forms a 
  connection it will send messages asynchronously every 1s
  
- `pair_example RECEIVER `, will automatically try to 
  connect to whatever is at tcp://127.0.0.1:8000.  It 
  will print out a value "temp" from every message 
  received. It uses blocking functionality.
  
- `pair_example AIO`, will automatically try to
  connect to whatever is at tcp://127.0.0.1:8000.  It
  will print out a value "temp" from every message
  received. It uses non-blocking functionality.
  

## Publisher/Subscriber Connection
A minimal example using the pub/sub communication 
paradigm.

Different exectuon setups:
- `pubsub_example PUBLISHER`, will listen on tcp://127.0.
  0.1:8000 and publish time data every second.
  
- `pubsub_example SUBSCRIBER`, will dial tcp://127.0.0.
  1:8000 and print out the time data being sent out from 
  the publisher
  
## Resource Discovery Example
A minimal example using Resouce Discovery.

Different execution setups:
- `resource_discovery_example HUB (listen address)`, 
  will listen on the provided listen address, or listen 
  on port 8000 ON ALL connections. It acts as a simple 
  RDH as well as offering a publisher endpoint with some 
  data.

- `resource_discovery_example PUBLISHER (listen address) 
  (rdh address)`,
  will listen on the provided listen address, or listen
  on port 8000 ON ALL connections. It will then either 
  search for an RDH or connect to the specified one. It 
  offers a publisher endpoint 
  
- `resource_discovery_example CONNECTION (listen address)
  (rdh address)`, will listen on the provided listen 
  address, or listen
  on port 8000 ON ALL connections. It will then either
  search for an RDH or connect to the specified one. It 
  creates endpoints by schemas with EVERYTHING it can 
  find. It uses the register startup functionality to 
  start doing things. It additionally, will delete and 
  restart all of its endpoints every four seconds to 
  show local use of endpoint manipulation.
  
  
## Streaming Example 
To be done

## Music Player Example
To be done

## Reconfiguration Example
To be done