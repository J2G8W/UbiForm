# Library plan
Below documents the central classes I plan to implement. This has scope to be changed, but allows me to plan as I start writing stuff. I believe it encompasses the key functionality that is required at the time of writing.

## Parts of the component
### Component::
The point of this class is to represent the whole object we are running on. The expectation is that anything using my software will only use one Component object to run

```
specifyManifest(FILE *)`
specifyManifest(char *jsonString)
```
> For our component we present raw data to build the manifest object which describes it. The Component then OWNS the manifest object (and is reponsible for deletion)

`void createNewPairEndpoint(std::string type, std::string id)`
> This function will create a new PairEndpoint which we can use. It stores the endpoint internally, and we can return it using other functions. The type refers to which type is specified in our manifest (details at the bottom of the document) and the id refers to the local id of the endpoint within the component. (We can have lots of endpoints of the same type, but with different ids)

```
DataReceiverEndpoint *getReceiverEndpoint(const std::string &id)
DataSenderEndpoint *getSenderEnpoint(const std::string &id)
```
> We return an endpoint from our component's data set. This can then be operated on using the functions presented by our different endpoints.

### ComponentManifest::
This is used to specify the description of a component. It contains a schema for each endpoint that the component can have. 
```
ComponentManifest (const char *)
ComponentManifest (FILE *)
```
> Our constructors from string and file pointer. The structure of the input JSON is described elsewhere.

`std::string stringify()`
> Returns a string version of our manifest which can be sent on networks etc

```
EndpointSchema *getReceiverSchema(const std::string& typeOfEndpoint)
EndpointSchema *getSenderSchema(const std::string &typeOfEndpoint)
```
>These methods will return pointers to our endpoint schemas given the typeOfEndpoint. They could be combined into a single function call with a boolean differentiater, but I feel this over complicates things

**METHODS TO CHANGE INDIVIDUAL EndpointSchema - further research is needed into exactly what I want to allow to change**

`void changeAttributeType( std::string &AttributeName, TYPE)`
> We change the type of an attribute

`void addRequired(std::string &AttributeName)`
> We specify that the attribute name is requried


## Endpoints
### (ABSTRACT) DataReceiverEndpoint::
The point of this class, is to represent something which receives data from a source. It is designed to always be the iniator of the conversation (dialer)

`DataReceiverEndpoint(EndpointSchema*)`
> We create the Endpoint by giving it a pointer to the schema it will be using. Becuase it is a pointer we are able to change the underlying schema from above it. The Endpoint is **not responsible** for memory management of the schema.

`virtual void dialConnection (const char* url)`
> This is implemented differently in each extending class, but has the concept of dialling something at the given URL

`unique_ptr<SocketMessage> receiveMessage()`
> Receive message on whatever socket we have opened - blocking. This SocketMessage has a unique pointer, so memory management is easier here. 

`void receiveAsyncMessage (HandlerFunction)`
> Receive message on whatever socket we have opened - non-blocking. We take in a function to handle the message rather than return the message as it by definition async

### (ABSTRACT) DataSenderEndpoint::
The point of this class, is to represent something which sends data to a source. It is designed to always be the reciver of the call to start. I.e. there has been some sort of request to it for info

`DataSenderEndpoint(EndpointSchema*)`
> We create the Endpoint by giving it a pointer to the schema it will be using. Becuase it is a pointer we are able to change the underlying schema from above it. The Endpoint is **not responsible** for memory management of the schema.

`virtual void listenForConnection(const char *url)`
> This is implemented differently in each extending class, but has the concept of listening for something at the given local URL

`void sendMessage(SocketMessage)`
> Sends message on whatever socket we have opened - blocking


`void sendAsyncMessage(SocketMessage)`
> Send message on whatever socket we have opened - non- blocking


### PairEndpoint:: (extends DataReceivingEndpoint, DataSendingEndpoint)
This gives the ability for each endpoint to send and receiver data (using the same underlying socket)

`PairEndpoint(EndpointSchema * receiveSchema, EndpointSchema * sendSchema)`
> This will create a new PairEndpoint based on the given schemas. Note that this will likely be private such that this can only be made via the **Component** interface.

`void listenForConnection(const char *url) override`
> This implements the required method

`void dialConnection(const char *url) override`
> This implements the required method


### PublisherEndpoint:: (extends DataSenderEndpoint)
This does the Publisher bit of the Pub/Sub model

`PublisherEndpoint(EndpointSchema * sendSchema)`
> This will create a new PublisherEndpoint based on the given schemas. Note that this will likely be private such that this can only be made via the **Component** interface.

`void listenForConnection(const char *url) override`
> This implements the required method

**May or may not implement this, it is extra**

`void sendMessageOfTopic(SocketMessage, TopicRepresentation)`
> Used by the Publisher endpoint to send a message of a specific TopicRepresentation


### SubscriberEndpoint:: (extends DataReceiverEndpoint)
This does the Subscriber bit of the Pub/Sub model

`SubscriberEndpoint(EndpointSchema * receiveSchema)`
> This will create a new SubscriberEndpoint based on the given schema. Note that this will likely be private such that this can only be made via the **Component** interface.

`void dialConnection(const char *url) override`
> This implements the required method

** May or may not implement this, it is extra **
`void specifyTopics( TopicRepresentation)`
> This will be used by the Subscriber endpoint to specify which topics they want to listen in on


### ResourceDiscoveryEndpoint:: (Extends SomeEndpoint)
Each component will likely have an endpoint of this type such that it has the ability to connect to the ResourceDiscoveryComponent and make requests

`void connectToRDC()`
> We will use **some as yet undecided technique** to find the RDC on the network we are currently on. This will establish the connection with the RDC component

`??? findAvailableConnections(EndpointSchema)`
> We make a request to the RDC  to find available connections on the network which can relate to our Manifest. We then return this in some form of data structure.

### MutatorEndpoint (extends PairEndpoint)
We want to send and receive changes which will change the manifest of our components.

**Will have very similar methods to the mutator methods for the ComponentManifest, these are TBC so I won't have any more depth here**

## Other bookkeeping
### SocketMessage::
This is used to describe a message which we'll send on the socket.

`SocketMessage()`
`Socket Message(const char*)`
> We have two constructors, a default one creates the empty message, and the string ne will initialise a message from an input string.
```
void addMember(std::string &attributeName, int value)
void addMember(std::string &attributeName, bool value)
void addMember(std::string &attributeName, std::string value)
void addMember(std::string &attributeName, std::vector<T> value)
```
> We add members to our socket message. This will include replacement so if you try and add two attributes of the same name, the most recent value is kept.

```
int getInteger(std::string &attributeName)
bool getBoolean(std::string &attributeName)
std::string getString(std::string &attributeName)
????? getArray(std::string &attributeName)
```
> We have getter methods for our socket message so we can manipulate the data on the other end. The array is a challenge to represent as we wanted to have the ability to do nesting

`std::string stringify()`
> Is used to return a string of our message such that it can be sent on the network

### EndpointSchema
This is used to describe schema for a given endpoint. It uses JSON schema to describe what data it can accept/send and means we can validate incoming messages against it. It is largely a wrapper class around our rapidjson object.

`EndpointSchema(rapidjson::Value &doc) :`
> Initialises out object. This method will likely be private such that they can only be initialised through the Component class

`void validate(const SocketMessage &messageToValidate)`
> This checks our schema against the given SocketMessage and throws an exception if there are any errors. The type of exception thrown will be clarified.

