#include "Client.h"
#include "Vehicle.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class Context{
    public:
        vector<Client> clients; //posicao 0 é o deposito
        vector<Vehicle> vehicles;
        int dimension; //termo usado no exemplo
        int qtdClients; //dimension - 1
        vector<vector<double>> matrixDistances;
        //vector<double>, índice [i*dimension + j]

        Context() : 
        clients(), vehicles(), dimension(0){}

        void load(const string& filename, int numVehicles);

        void distances();
        //void distances(vector<Clients> clients, int matrizDistancias[dimension][dimension]);
};

