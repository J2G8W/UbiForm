#include <string>
#include <fstream>
#include "../../include/UbiForm/Component.h"

int main(int argc, char **argv) {
    if (argc >= 2){
        std::string rdhUrl = argv[1];
        Component component;
        FILE *pFile = fopen("EvaluationFiles/EvaluationManifest1.json", "r");
        if (pFile == nullptr) perror("ERROR");
        component.specifyManifest(pFile);
        fclose(pFile);
        try {
            component.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhUrl);
        } catch (std::logic_error &e) {
            std::cerr << "Error connecting to RDH: " <<  e.what() << std::endl;
            return -1;
        }
        std::chrono::duration<int64_t, std::nano> start[5];
        std::chrono::duration<int64_t, std::nano> end[5];
        int numberOfComponents[5];
        for(int i = 0; i<5;i++) {
            start[i] = std::chrono::high_resolution_clock::now().time_since_epoch();
            std::map<std::string, std::string> empty;
            auto response = component.getResourceDiscoveryConnectionEndpoint().getComponentsBySchema("pairEvaluation", empty);
            end[i] = std::chrono::high_resolution_clock::now().time_since_epoch();
            numberOfComponents[i] = response.size();
        }

        component.getResourceDiscoveryConnectionEndpoint().deRegisterFromAllHubs();

        std::ofstream results;
        results.open("rd_scalability.csv",std::fstream::out | std::fstream::app);
        for(int i = 0; i<5;i++){
            auto duration = end[i] - start[i];
            results << duration.count() << "," << numberOfComponents[i] << ",\n";
        }
        results.close();
    }else {
        std::cerr << "Error usage is " << argv[0] << " [RDH url]\n";
    }
}