#include <string>
#include "../../include/UbiForm/Component.h"

int main(int argc, char **argv) {
    if (argc >= 3){
        std::string rdhUrl = argv[1];
        int numComponents = std::stoi(argv[2]);

        Component components[numComponents];

        for(int i = 0; i<numComponents;i++){
            FILE *pFile = fopen("EvaluationFiles/EvaluationManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            components[i].specifyManifest(pFile);
            fclose(pFile);
            try {
                components[i].getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhUrl);
            } catch (std::logic_error &e) {
                std::cerr << "Error connecting to RDH: " <<  e.what() << std::endl;
                return -1;
            }
        }
        std::cout << "Registered" << std::endl;
        // Wait for enter before deregistering
        std::string temp;
        std::cin >> temp;

        for(int i=0;i<numComponents;i++){
            components[i].getResourceDiscoveryConnectionEndpoint().deRegisterFromAllHubs();
        }
        std::cout << "DEREG" << std::endl;
    } else {
        std::cerr << "Error usage is " << argv[0] << " [RDH url] [number of components] \n";
    }
}