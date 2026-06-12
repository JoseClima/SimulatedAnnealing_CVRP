#include "Context.h"
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <string>
#include <cmath>
#include <utility>
using std::vector;
using std::sort;
using std::string;
using std::ifstream;
using std::pair;
using std::sqrt;
using std::pow;


void Context::load(const string& filename, int numVehicles){
    ifstream file(filename);
    if(file.is_open() == true){
        string word;
        int capacity = 0; //evitar erros
        while(file >> word){

            if(word == "DIMENSION"){
                file >> word;
                file >> dimension;
            }

            else if(word == "CAPACITY"){
                file >> word;
                file >> capacity;
            }

            else if(word == "NODE_COORD_SECTION"){
                clients.reserve(dimension); //resize nao funciona pois nao tem construtor padrao
                int id, x, y;
                for(int i = 0; i < dimension; i++){
                    file >> id >> x >> y;
                    clients.emplace_back(id, x, y, 0); //emplace_back parece ser melhor que o push_back
                }
            }

            else if(word == "DEMAND_SECTION"){
                int id, dimAux;
                for(int i = 0; i <dimension; i++){
                    file >> id >> dimAux;
                    clients[id-1].demand = dimAux;
                }
            }

            else if (word == "DEPOT_SECTION"){ //parece que todos tem o deposito no id 1
                break;
            }

        }

        file.close();
        qtdClients = dimension - 1;
        for(int i = 0; i < numVehicles; i++){
            vehicles.push_back(capacity); 
            //vehicles[i].capacity = capacity;
        }

    }
    else{
        printf("Erro ao abrir o arquivo");
    }

}

void Context::distances(){
    double distAux = 0;
    matrixDistances.assign(dimension, vector<double>(dimension, 0.0)); //faz a matriz ser [dimension][dimension]
    for(int i = 0; i < dimension; i++){
        for(int j = i+1; j < dimension; j++){
            matrixDistances[i][j] = sqrt(pow((clients[i].x-clients[j].x),2)+pow((clients[i].y-clients[j].y),2));
            matrixDistances[j][i] = matrixDistances[i][j];
        }
    }
}

void Context::buildNearestClients(int maxNeighbors){
    for(int i = 0; i < dimension; i++){

        struct distToOther
        {
            double dist;
            int position;
        };
        
        vector<distToOther> distToOthers;
        distToOthers.reserve(dimension - 1);

        for(int j = 0; j < dimension; j++){
            if (i != j){
                distToOthers.push_back({matrixDistances[i][j],j});
            }
        }

        //assim que se faz pq estou usando struct
        sort(distToOthers.begin(), distToOthers.end(), [](const distToOther& a, const distToOther &b){
            return a.dist < b.dist;
        });
        
        int limit;
        if(maxNeighbors > dimension -1){
            limit = dimension-1;
        }
        else{
            limit = maxNeighbors;
        }

        for(int k = 0; k < limit; k++){
            clients[i].nearest.push_back(distToOthers[k].position); //push back melhor que emplace pq o valor ja existe pelo visto
        }


    }
}