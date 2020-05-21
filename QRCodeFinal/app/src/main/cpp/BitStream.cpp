#include "BitStream.h"

using namespace std;

void BitStream::reserve(int length) {
    bits.reserve(length);
}

void BitStream::push_back(int to_push, int length) {
    for (int i = length - 1; i >= 0; i--)
        bits.push_back(((to_push >> i) & 1));
}

void BitStream::push_back(const vector<int>& input) {
    for (const auto& i : input) {
        bits.push_back(i);
    }
}

vector<int> BitStream::getBits() {
    return bits;
}

int BitStream::size() const {
    return bits.size();
}

const int& BitStream::operator[](int index) {
    return bits[index];
}