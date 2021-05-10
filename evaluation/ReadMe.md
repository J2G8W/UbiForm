# Instructions for Evaluation
The programs contained within this folder were used to 
evaluate UbiForm within the dissertation which describes 
it. 

All of the evaluation executables are built when the 
flag `BUILD_EVALUATION` is set to On in the CMake build 
process. The executables are built into the evaluation 
folder.


##Brokered experiment
This experiment is based on a middleware benchmark which 
is further described in the dissertation. The benchmark 
is based on smart cities and is supposed to be an 
industrial measurement.

There are two separate files: 
- `eval_Broker.cpp` file makes the 
  `brokered_speed_test_broker` executable file which 
  runs the broker for the experiment
  
- `eval_PublisherSubscriber.cpp` makes the 
  `brokered_speed_test_pubsub` executable file which is 
  run separately for a publisher component and a 
  subscriber component.
  
The concept is that one device acts as a broker, and 
another device starts two separate components which are 
the publisher and the subscriber.

The broker is run trivially with 
`brokered_speed_test_broker` and it is reconfigured on 
running, so it starts with no information about what it 
will be brokering

The publisher component runs with 
`brokered_speed_test_pubsub PUBLISHER [broker address]`

The subscriber component runs with 
`brokered_speed_test_pubsub SUBSCRIBER [broker address]`

The `run_brokered_pub_sub.sh` automates the startup of 
the publisher and subscriber almost simultaneously and 
again accepts the broker address as its one argument. 

The output from this comes into a files as such:
- `broker_publisher_results.txt` has a format of first 
  line has the list of "send times" and the second line 
  has the list of "receive times". Basically, each 
  column is the time it took to send the message and 
  receive a reply

- `broker_subscriber_results.txt` has a format of first 
  line has a summary of results (total_length, #messagss 
  received, #messages lost) the second line has the time 
  a message was received (which can be compared back to 
  the publisher results text)
  
## Resource Discovery Scalability
This experiment was run with LifeHub demonstration where 
the phone ran the RDH. But this doesn't have to be the case.

We have two separate files:
- `eval_ResourceDiscoveryQuery.cpp` which makes the 
  `resource_discovery_scalability_query` executable

- `eval_ResourceDiscoveryRegister.cpp` which makes the 
  `resource_discovery_scalability_register` executable
  

We first run `resource_discovery_scalability_register 
[rdh_url] [# components]` which registers the requested 
number of components with the RDH.

We then run `resource_discovery_scalability_query 
[rdh_url]` which times how long it takes to query the 
rdh once things have been register. The results go into 
`rd_scalability.csv` in the form: time for query, 
number of components. The query automatically does 5 
queries one after the other for repeats, with a repeat 
per line.


## Streaming & Reconfiguration Scalability
This experiment is built to test the speed of the music 
player demonstration as devices scale. 

File is `eval_ScalabilityStreaming.cpp` built into 
executable `scalability_streaming_test`. 

Simple execution is `scalability_streaming_test 
[device URL] [# simultaneous connections]`. The results 
come out to:
- `scalability_streaming_results.csv` which has the 
  values for the streaming. For each stream made we 
  have: time for stream, size of file streamed, number 
  of components connected

- `scalability_reconfiguration_results.csv` which has 
  the values for reconfiguration - the measures of the 
  time taken to start the connection. For each stream 
  made we have: time to start connection, number of 
  components simultaneously streaming at startup.
  

## Section Speeds for UbiForm
This experiment looks at how long different sections of 
UbiForm takes. It can be used with and without resource 
discovery. We have a sender and receiver connected to 
some RDH and we measure the speeds which things take place

File is `eval_SectionSpeeds.cpp` built into executable 
`section_speed_test`. 

The options for what is done is specified IN THE C++ 
code, with `USE_RDH` and `USE_ASYNC_SEND.

The sender is executed with: `section_speed_test SENDER 
[dial address/RDH]`.
The receiver is executed with: `section_speed_test 
RECEIVER (location of RDH if needed)`.


Output goes to`section_speeds_sender_results.csv` each line 
has the timings at points in the execution: start of 
process, component started, registered with hub, queried 
hub, connected to component, message built, message sent 


## Comparing UbiForm to NNG on streaming
We compare what the overheads of UbiForm compared to the 
raw NNG.

We have files:
- `eval_MessageSpeedNNG.cpp` which builds the executable 
  `nng_message_speed_test`.
- `eval_StreamingSpeedUbiForm.cpp` which builds the 
  executable `UbiForm_stream_speed_test`
  

Both are sent with `SENDER [file location] [block size]`

Both are received with `RECEIVER [dial location]`

The output goes to two files: 
- `messaging_nng_results.csv`
- `streaming_UbiForm_results.csv`

With the form: time for stream, file size, block size



This is a rough explanation of how the evaluation 
programs run
