#pragma once

#include <vector>

using namespace std;

class BitStream {
public:
    void reserve(int length);
    void push_back(int to_push, int length);
    void push_back(const vector<int>& input);
    vector<int> getBits();
    int size() const;
    const int& operator[](int index);
private:
    vector<int> bits;
};

