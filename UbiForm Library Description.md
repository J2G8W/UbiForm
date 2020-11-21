# Library plan
### Component::
The point of this class is to represent the whole object we are running on. The expectation is that anything using my software will only use one Component object to run

`specifyManifest(FILE *)`
`specifyManifest(char *jsonString)`
> For our component we present raw data to build the manifest object which describes it. The Component then OWNS the manifest object (and is reponsible for deletion)

### (ABSTRACT) Endpoint::
The point of this class is to represent a socket connection with something. It will be able to do different types of connection, but basically abstracts of the idea of NNG sockets.
`Endpoint(ComponentManifest*)`
> We create the Endpoint by giving it a pointer to the manifest its going to use. It DOES NOT own the manifest, as the concept is that it will be owned by a component.

`ABSTRACT void createReceiver(const char url)`
`ABSTRACT void createInitiator(const char url)`
> These two methods are ABSTRACT as we need to specify a connection type. Also allows for extensiality

`void sendMessage(SocketMessage)`
> Sends message on whatever socket we have opened - blocking

`unique_ptr\<SocketMessage\> receiveMessage()`
> Receive message on whatever socket we have opened - blocking

### PairEndpoint:: (extends Endpoint)
This extends the Endpoint class and represents a Pair connection.

`void createReceiver(const char *url)`
> This will create a pair socket and which listens for connections on the url. The URL must be internal for us to listen on it.

`void createInitiator(const char *url)`
> Create a pair socket which dials for a connection the url. The URL can be external (i.e. it initiates the connection)

### PubSubEndpoint:: (extends Endpoint)
This extends the Endpoint class and does the Pub/Sub model.

`void createReceiver(const char *url)`
> This will create a Publisher endpoint which listens on an internal port and will do publisher things

`void createInitiator(const char *url)`
> This will create a Subscriber endpoint which dials to the to the external url and will do subscriber things

`void specifyTopics( TopicRepresentation)`
> This will be used by the Subscriber endpoint to specify which topics they want to listen in on

`void sendMessageOfTopic(SocketMessage, TopicRepresentation)`
> Used by the Publisher endpoint to send a message of a specific TopicRepresentation

### ComponentManifest::
This is used to specify the description of a component. It contains schemas for the endpoints

`ComponentManifest (const char *)`
`ComponentManifest (FILE *)`
> Our constructors from string and file pointer

`std::string stringify()`
> Returns a string version of our manifest which can be sent on networks etc

`void validate(SocketMessage& s, id)`
> This will validate our SocketMessage against the schema which was specified in the manifest given the id of the endpoint

`void addAttribute

### SocketMessage::
This is used to describe a message which we'll send on the socket.

`SocketMessage()`
`Socket Message(const char*)`
> We have two constructors, a default one creates the empty message, and the string ne will initialise a message from an input string.

`void addMember(std::string &attributeName, int value)`
`void addMember(std::string &attributeName, bool value)`
`void addMember(std::string &attributeName, std::string value)`
`void addMember(std::string &attributeName, std::vector<T> value)`
> We add members to our socket message. This will include replacement so if you try and add two attributes of the same name, the most recent value is kept.

`int getInteger(std::string &attributeName)`
`bool getBoolean(std::string &attributeName)`
`std::string getString(std::string &attributeName)`
`????? getArray(std::string &attributeName)`
> We have getter methods for our socket message so we can manipulate the data on the other end. The array is a challenge to represent as we wanted to have the ability to do nesting

`std::string stringify()`
> Is used to return a string of our message such that it can be sent on the network

### ResourceDiscoveryComponent:: (Extends Component)
This will designed such that the component sits in the network and can be connected to so other components can discover each other (and what data they accept)

`void acceptConnections()`
>An async process which will accept connections and on connection will record the manifest of the connecting component to send out. These connections will be of type pair and will mean that a component can request things from the RDC about different available manifests

### ResourceDiscoveryEndpoint:: (Extends Endpoint)
Each component will likely have an endpoint of this type such that it has the ability to connect to the ResourceDiscoveryComponent and make requests

`void connectToRDC()`
> We will use **some as yet undecided technique** to find the RDC on the network we are currently on. This will establish the connection with the RDC component

`??? findAvailableConnections(ComponentManifest)`
> We make a request to the RDC  to find available connections on the network which can relate to our Manifest. We then return this in some form of data structure.
<!--stackedit_data:
eyJoaXN0b3J5IjpbNzQwMjI2NDUsODAwNTU1NjgsLTEwODM3OT
AyODcsNjQ3NTc1MTkwXX0=
-->