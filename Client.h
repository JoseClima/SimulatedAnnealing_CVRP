#include <vector>
using std::vector;

class Client {
public:
    int id;
    int x, y;
    int demand;
    std::vector<int> nearest;
    
    Client(int id, int x, int y, int demand) : 
    id(id), x(x), y(y), demand(demand) {}
};
