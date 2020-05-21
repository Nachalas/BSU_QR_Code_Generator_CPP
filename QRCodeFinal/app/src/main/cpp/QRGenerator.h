#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <sstream>
#include "Constants.h"
#include "BitStream.h"
#include "Mode.h"
#include "QRCode.h"
#include "QRCodeSegment.h"

using namespace std;

bool isNumeric(const string& input) {
    for (const char& i : input) {
        if (i < '0' || i > '9') {
            return false;
        }
    }
    return true;
}

bool isAlphanumeric(const string& input) {
    for (const auto& i : input) {
        if (ALPHANUMERIC_SYMBOLS_SET.find(toupper(i)) == string::npos) {
            return false;
        }
    }
    return true;
}

QRCodeSegment createNumericSegment(const string& input) {
    BitStream bitStream;
    bitStream.reserve((input.size() / 3) * 10 + (input.size() % 3) * 3 + 1);
    int intoSeg = 0;
    int count = 0;
    for (const auto& i : input) {
        intoSeg = intoSeg * 10 + (i - '0');
        count++;
        if (count == 3) {
            bitStream.push_back(intoSeg, 10);
            count = 0;
            intoSeg = 0;
        }
    }
    if (count > 0) {
        bitStream.push_back(intoSeg, count * 3 + 1);
    }
    return QRCodeSegment(bitStream.getBits(), input.size(), Mode::NUMERIC);
}

QRCodeSegment createAlphanumericSegment(const string& input) {
    BitStream bitStream;
    bitStream.reserve((input.size() / 2) * 11 + (input.size() % 2) * 6);
    int intoSeg = 0;
    int count = 0;
    for (int i = 0; i < input.size(); i++) {
        if (i + 1 != input.size()) {
            int fco = ALPHANUMERIC_SYMBOLS_SET.find(toupper(input[i]));
            int sco = ALPHANUMERIC_SYMBOLS_SET.find(toupper(input[i + 1]));
            intoSeg = fco * 45 + sco;
            bitStream.push_back(intoSeg, 11);
            i++;
        }
        else {
            bitStream.push_back(ALPHANUMERIC_SYMBOLS_SET.find(toupper(input[i])), 6);
        }
    }
    return QRCodeSegment(bitStream.getBits(), input.size(), Mode::ALPHANUMERIC);
}

QRCodeSegment createByteSegment(const string& input) {
    BitStream bitStream;
    bitStream.reserve(input.size() * 8);
    for (const auto& i : input) {
        bitStream.push_back(static_cast<uint8_t>(i), 8);
    }
    return QRCodeSegment(bitStream.getBits(), input.size(), Mode::BYTE);
}

int getSegmentsOverallLength(const vector<QRCodeSegment>& segments, int version) {
    int toReturn = 0;
    for (const QRCodeSegment& seg : segments) {
        int segmentSize = seg.segmentMode.getCharCount(version);
        toReturn += 4;
        toReturn += segmentSize;
        toReturn += seg.getBitCount();
    }
    return toReturn;
}

QRCode encodeSegments(const vector<QRCodeSegment>& segments, ErrCorrLevel levelOfCorrection, bool maximizeECL = true) {
    int dataBits = 0;
    int version = 0;
    int maxCapacity = 0;
    for (version = MIN_VER; version < MAX_VER; version++) {
        maxCapacity = MAX_DATA_BY_CORR_LVL[levelOfCorrection][version];
        dataBits = getSegmentsOverallLength(segments, version);
        if (dataBits <= maxCapacity) {
            break;
        }
    }

    if (maximizeECL) {
        for (ErrCorrLevel newEcl : vector<ErrCorrLevel>{ ErrCorrLevel::MEDIUM, ErrCorrLevel::QUARTILE, ErrCorrLevel::HIGH }) {
            if (dataBits <= MAX_DATA_BY_CORR_LVL[newEcl][version])
                levelOfCorrection = newEcl;
        }
    }

    BitStream bitStream;
    bitStream.reserve(dataBits);

    for (const QRCodeSegment& seg : segments) {
        bitStream.push_back(seg.segmentMode.getMode(), 4);
        bitStream.push_back(seg.getCharCount(), seg.segmentMode.getCharCount(version));
        bitStream.push_back(seg.getBits());
    }

    bitStream.push_back(0, min(4, maxCapacity - bitStream.size()));

    bitStream.push_back(0, (8 - bitStream.size() % 8) % 8);

    const int FILLER_1 = 236;
    const int FILLER_2 = 17;
    bool flag = true;
    while (bitStream.size() != maxCapacity) {
        if (flag) {
            bitStream.push_back(FILLER_1, 8);
            flag = false;
        }
        else {
            bitStream.push_back(FILLER_2, 8);
            flag = true;
        }
    }

    vector<uint8_t> dataCodewords(bitStream.size() / 8);
    for (size_t i = 0; i < bitStream.size(); i++)
        dataCodewords[i >> 3] |= (bitStream[i] ? 1 : 0) << (7 - (i & 7)); // Перевод в байты

    return QRCode(version, levelOfCorrection, dataCodewords);
}

auto encodeString(const string& input, ErrCorrLevel levelOfCorrection = ErrCorrLevel::LOW) {
    vector<QRCodeSegment> segments;
    if (isNumeric(input)) {
        segments.push_back(createNumericSegment(input));
    }
    else if (isAlphanumeric(input)) {
        segments.push_back(createAlphanumericSegment(input));
    }
    else {
        segments.push_back(createByteSegment(input));
    }
    return encodeSegments(segments, levelOfCorrection);
}
