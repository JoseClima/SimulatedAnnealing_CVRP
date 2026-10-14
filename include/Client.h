#pragma once
#include <vector>
using std::vector;

class Client {
    private:
        int id;
        int x, y;
        int demand;
        std::vector<int> nearest;

    public:
        Client(int id, int x, int y, int demand)
            : id(id), x(x), y(y), demand(demand) {}

        int getId() { 
            return id; 
        }

        int getX() { 
            return x; 
        }

        int getY() { 
            return y; 
        }

        int getDemand() { 
            return demand; 
        }

        void setDemand(int d) { 
            demand = d; 
        }

        vector<int> &getNearest() { 
            return nearest; 
        }

        void addNearest(int c) { 
            nearest.push_back(c); 
        }
};