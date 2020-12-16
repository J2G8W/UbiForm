#include <fstream>

class SimpleCR : public testing::Test{
protected:
    SimpleCR(){
    }
    void loadComponentRepresentation(const char * location){
        FILE* pFile = fopen(location, "r");
        if (pFile == NULL){
            std::cerr << "Error finding requisite file ("<<location <<")" << std::endl;
        }
        componentRepresentation = new ComponentRepresentation(pFile);
    }
    void loadSocketMessage(std::string location){
        try {
            std::ifstream in(location);
            std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            socketMessage = new SocketMessage(contents.c_str());
        }catch(const std::ifstream::failure &e){
            std::cerr << "ERROR OPENING FILE: " << location <<std::endl;
            throw;
        }
    }

    ~SimpleCR(){
        delete componentRepresentation;
        delete socketMessage;
    }


    ComponentRepresentation* componentRepresentation;
    SocketMessage* socketMessage;
};

TEST_F(SimpleCR, EqualityOfExact){
    loadComponentRepresentation("TestManifests/Component1.json");
    loadSocketMessage("TestManifests/Endpoint1.json");

    ASSERT_TRUE(componentRepresentation->isEqual("v1",true, *socketMessage));
}

TEST_F(SimpleCR, EqualityOfFunctionallyExact){
    loadComponentRepresentation("TestManifests/Component1.json");
    loadSocketMessage("TestManifests/Endpoint2.json");

    ASSERT_TRUE(componentRepresentation->isEqual("v1",true, *socketMessage));
}

TEST_F(SimpleCR, FailureOfDifferentType){
    loadComponentRepresentation("TestManifests/Component1.json");
    loadSocketMessage("TestManifests/Endpoint3.json");

    ASSERT_FALSE(componentRepresentation->isEqual("v1",true, *socketMessage));
}

TEST_F(SimpleCR, ComplexEquality){
    loadComponentRepresentation("TestManifests/Component2.json");
    loadSocketMessage("TestManifests/Endpoint4.json");

    ASSERT_TRUE(componentRepresentation->isEqual("v1",false, *socketMessage));
}

TEST_F(SimpleCR, ComplexDifferences){
    loadComponentRepresentation("TestManifests/Component2.json");
    loadSocketMessage("TestManifests/Endpoint5.json");

    ASSERT_FALSE(componentRepresentation->isEqual("v1",false, *socketMessage));
}