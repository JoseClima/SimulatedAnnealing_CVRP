#include "../include/Context.h"
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <string>
#include <cmath>
#include <cctype>
#include <utility>
using std::vector;
using std::sort;
using std::string;
using std::ifstream;
using std::pair;
using std::sqrt;
using std::pow;


void Context::load(string& filename, int numVehicles){
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

            else if(word == "COMMENT"){
                string comment;
                std::getline(file, comment);
                // procura "ptimal" — cobre "Optimal value: 784" e "Optimal value : 784"
                size_t pos = comment.find("ptimal");
                if(pos != string::npos){
                    size_t i = pos;
                    while(i < comment.size() && !isdigit((unsigned char)comment[i])) i++;
                    int val = 0;
                    while(i < comment.size() && isdigit((unsigned char)comment[i])){
                        val = val * 10 + (comment[i] - '0');
                        i++;
                    }
                    optimalCost = val;
                }
            }

            else if(word == "NODE_COORD_SECTION"){
                clients.reserve(dimension);
                int id, x, y;
                for(int i = 0; i < dimension; i++){
                    file >> id >> x >> y;
                    clients.emplace_back(id, x, y, 0); 
                }
            }

            else if(word == "DEMAND_SECTION"){
                int id, dimAux;
                for(int i = 0; i < dimension; i++){
                    file >> id >> dimAux;
                    clients[id-1].setDemand(dimAux);
                }
            }

            else if (word == "DEPOT_SECTION"){ //deposito no id 1
                break;
            }

        }

        file.close();

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
    matrixDistances.assign(dimension, vector<double>(dimension, 0.0));
    for(int i = 0; i < dimension; i++){
        for(int j = i+1; j < dimension; j++){
            double dx = clients[i].getX() - clients[j].getX();
            double dy = clients[i].getY() - clients[j].getY();
            matrixDistances[i][j] = sqrt(dx*dx + dy*dy);
            matrixDistances[j][i] = matrixDistances[i][j];
        }
    }
}

void Context::buildNearestClients(int maxNeighbors){
    nearestNeighbors = maxNeighbors;

    struct aux
        {
            double dist;
            int position;
        };

        vector<aux> distToOthers;
        distToOthers.reserve(dimension - 1);

    for(int i = 0; i < dimension; i++){      
        distToOthers.clear();

        for(int j = 0; j < dimension; j++){
            if (i != j){
                distToOthers.push_back({matrixDistances[i][j],j});
            }
        }

        sort(distToOthers.begin(), distToOthers.end(), [](aux& a, aux &b){
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
            clients[i].addNearest(distToOthers[k].position);
        }


    }
}