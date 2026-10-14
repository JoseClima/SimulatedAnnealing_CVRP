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
        int optimalCost;
        int nearestNeighbors;
        vector<vector<double>> matrixDistances;

    public:
        Context() : clients(), vehicles(), dimension(0),
                    optimalCost(0), nearestNeighbors(0) {}

        void load(string& filename, int numVehicles);
        void distances();
        void buildNearestClients(int maxNeighbors = 30);

        int getDimension() {
            return dimension;
        }

        int getQtdClients() {
            return dimension - 1;
        }

        int getOptimalCost() {
            return optimalCost;
        }

        int getNearestNeighbors() {
            return nearestNeighbors;
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