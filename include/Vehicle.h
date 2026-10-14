#pragma once

class Vehicle {
    private:
        int capacity;

    public:
        Vehicle(int capacity) : capacity(capacity) {}

        int getCapacity() { 
            return capacity; 
        }
};