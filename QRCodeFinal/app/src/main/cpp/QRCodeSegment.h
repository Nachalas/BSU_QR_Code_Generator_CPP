#pragma once

#include <vector>
#include "mode.h"

class QRCodeSegment {
public:

    vector<int> getBits() const {
        return bits;
    }

    int getBitCount() const {
        return bits.size();
    }

    int getCharCount() const {
        return charCount;
    }

    QRCodeSegment(const vector<int>& input, int charcount, Mode inputMode) : segmentMode(inputMode), charCount(charcount) {
        bits = input;
    }

    const Mode segmentMode;
private:
    vector<int> bits;
    int charCount;
};
