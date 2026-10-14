#pragma once

#include "Client.h"
#include "Vehicle.h"
#include <string>
#include <utility>
using std::pair;
using std::string;

class Context {
    private:
        vector<Client>  clients; // posição 0 é o depósito
        vector<Vehicle> vehicles;
        int dimension;
        int qtdClients;
        vector<vector<double>> matrixDistances;

    public:
        Context() : clients(), vehicles(), dimension(0), qtdClients(0) {}

        void load(string &filename, int numVehicles);
        void distances();
        void buildNearestClients(int maxNeighbors = (20));

        int getDimension() { 
            return dimension; 
        }

        int getQtdClients() { 
            return qtdClients; 
        }

        vector<Client> &getClients() { 
            return clients; 
        }

        Client &getClient(int i) { 
            return clients[i]; 
        }

        vector<Vehicle> &getVehicles() { 
            return vehicles; 
        }

        Vehicle &getVehicle(int i) { 
            return vehicles[i]; 
        }

        double getDistance(int i, int j) { 
            return matrixDistances[i][j]; 
        }
};