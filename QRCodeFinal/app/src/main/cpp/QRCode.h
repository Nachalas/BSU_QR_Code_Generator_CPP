#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include "constants.h"

class QRCode {
public:
    QRCode(int version, ErrCorrLevel level, const vector<uint8_t>& dataCodewords, int mask = -1) : version(version), levelOfCorrection(level) {
        size = version * 4 + 17; // Example: 40 * 4 + 17 = 177
        modules = vector<vector<bool>>(size, vector<bool>(size, false));
        isFunction = vector<vector<bool>>(size, vector<bool>(size));

        drawPatterns();
        const vector<uint8_t> allCodewords = addECB(dataCodewords);
        drawCodewords(allCodewords);

        cout << "ver: " << version << endl;

        if (mask == -1) {
            long minPenalty = LONG_MAX;
            for (int i = 0; i < 8; i++) {
                applyMask(i);
                drawFormatPart(i);
                long penalty = getPenaltyScore();
                if (penalty < minPenalty) {
                    mask = i;
                    minPenalty = penalty;
                }
                applyMask(i);
            }
        }
        this->mask = mask;
        applyMask(mask);
        drawFormatPart(mask);

        isFunction.clear();
        isFunction.shrink_to_fit();
    }

    void drawPatterns() {
        for (int i = 0; i < size; i++) {
            setModule(6, i, i % 2 == 0);
            setModule(i, 6, i % 2 == 0);
        }

        drawSearchPattern(3, 3);
        drawSearchPattern(size - 4, 3);
        drawSearchPattern(3, size - 4);

        vector<int> alignmentPatternsCoords = ALIGNMENT_PATTERNS_POSITIONS[version];

        for (int i = 0; i < alignmentPatternsCoords.size(); i++) {
            for (int j = 0; j < alignmentPatternsCoords.size(); j++) {
                if (!((i == 0 && j == 0) || (i == 0 && j == alignmentPatternsCoords.size() - 1) || (i == alignmentPatternsCoords.size() - 1 && j == 0)) || alignmentPatternsCoords.size() == 1) {
                    drawAlignmentPattern(alignmentPatternsCoords[i], alignmentPatternsCoords[j]);
                }
            }
        }

        drawFormatPart(0);
        drawVersion();
    }

    void drawSearchPattern(int x, int y) {
        for (int dy = -4; dy <= 4; dy++) {
            for (int dx = -4; dx <= 4; dx++) {
                int dist = max(abs(dx), abs(dy));
                int xx = x + dx, yy = y + dy;
                if (0 <= xx && xx < size && 0 <= yy && yy < size)
                    setModule(xx, yy, dist != 2 && dist != 4);
            }
        }
    }

    void drawAlignmentPattern(int x, int y) {
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++)
                setModule(x + dx, y + dy, max(abs(dx), abs(dy)) != 1);
        }
    }

    void drawFormatPart(int mask) {
        int bits = MASKANDECLCODES[getCorrLvlInt(levelOfCorrection)][mask];
        for (int i = 0; i <= 5; i++)
            setModule(8, i, getModule(bits, i));
        setModule(8, 7, getModule(bits, 6));
        setModule(8, 8, getModule(bits, 7));
        setModule(7, 8, getModule(bits, 8));
        for (int i = 9; i < 15; i++)
            setModule(14 - i, 8, getModule(bits, i));

        for (int i = 0; i < 8; i++)
            setModule(size - 1 - i, 8, getModule(bits, i));
        for (int i = 8; i < 15; i++)
            setModule(8, size - 15 + i, getModule(bits, i));
        setModule(8, size - 8, true);
    }

    void drawVersion() {
        if (version < 7)
            return;

        int bits = VERSIONCODES[version];

        for (int i = 0; i < 18; i++) {
            bool bit = getModule(bits, 17 - i);
            int a = size - 11 + i / 6;
            int b = i % 6;
            setModule(a, b, bit);
            setModule(b, a, bit);
        }
    }

    void drawCodewords(const vector<uint8_t>& data) {
        size_t i = 0;
        for (int right = size - 1; right >= 1; right -= 2) {
            if (right == 6)
                right = 5;
            for (int vert = 0; vert < size; vert++) {
                for (int j = 0; j < 2; j++) {
                    size_t x = static_cast<size_t>(right - j);
                    bool upward = ((right + 1) & 2) == 0;
                    size_t y = static_cast<size_t>(upward ? size - 1 - vert : vert);
                    if (!isFunction.at(y).at(x) && i < data.size() * 8) {
                        modules.at(y).at(x) = getModule(data.at(i >> 3), 7 - static_cast<int>(i & 7));
                        i++;
                    }
                }
            }
        }
    }

    void applyMask(int msk) {
        size_t sz = static_cast<size_t>(size);
        for (size_t y = 0; y < sz; y++) {
            for (size_t x = 0; x < sz; x++) {
                bool invert;
                switch (msk) {
                    case 0:
                        invert = (x + y) % 2 == 0;
                        break;
                    case 1:
                        invert = y % 2 == 0;
                        break;
                    case 2:
                        invert = x % 3 == 0;
                        break;
                    case 3:
                        invert = (x + y) % 3 == 0;
                        break;
                    case 4:
                        invert = (x / 3 + y / 2) % 2 == 0;
                        break;
                    case 5:
                        invert = x * y % 2 + x * y % 3 == 0;
                        break;
                    case 6:
                        invert = (x * y % 2 + x * y % 3) % 2 == 0;
                        break;
                    case 7:
                        invert = ((x + y) % 2 + x * y % 3) % 2 == 0;
                        break;
                }
                modules.at(y).at(x) = modules.at(y).at(x) ^ (invert & !isFunction.at(y).at(x));
            }
        }
    }

    long getPenaltyScore() const {
        long result = 0;

        for (int y = 0; y < size; y++) {
            bool runColor = false;
            int runX = 0;
            array<int, 7> runHistory = {};
            for (int x = 0; x < size; x++) {
                if (modules[x][y] == runColor) {
                    runX++;
                    if (runX == 5)
                        result += PENALTY_N1;
                    else if (runX > 5)
                        result++;
                }
                else {
                    finderPenaltyAddHistory(runX, runHistory);
                    if (!runColor)
                        result += finderPenaltyCountPatterns(runHistory) * PENALTY_N3;
                    runColor = modules[x][y];
                    runX = 1;
                }
            }
            result += finderPenaltyTerminateAndCount(runColor, runX, runHistory) * PENALTY_N3;
        }

        for (int x = 0; x < size; x++) {
            bool runColor = false;
            int runY = 0;
            array<int, 7> runHistory = {};
            for (int y = 0; y < size; y++) {
                if (modules[x][y] == runColor) {
                    runY++;
                    if (runY == 5)
                        result += PENALTY_N1;
                    else if (runY > 5)
                        result++;
                }
                else {
                    finderPenaltyAddHistory(runY, runHistory);
                    if (!runColor)
                        result += finderPenaltyCountPatterns(runHistory) * PENALTY_N3;
                    runColor = modules[x][y];
                    runY = 1;
                }
            }
            result += finderPenaltyTerminateAndCount(runColor, runY, runHistory) * PENALTY_N3;
        }

        for (int y = 0; y < size - 1; y++) {
            for (int x = 0; x < size - 1; x++) {
                bool  color = modules[x][y];
                if (color == modules[x + 1][y] &&
                    color == modules[x][y + 1] &&
                    color == modules[x + 1][y + 1])
                    result += PENALTY_N2;
            }
        }

        int black = 0;
        for (const vector<bool>& row : modules) {
            for (bool color : row) {
                if (color)
                    black++;
            }
        }
        int total = size * size;
        int k = static_cast<int>((abs(black * 20L - total * 10L) + total - 1) / total) - 1;
        result += k * PENALTY_N4;
        return result;
    }

    int finderPenaltyCountPatterns(const array<int, 7>& runHistory) const {
        int n = runHistory.at(1);
        bool core = n > 0 && runHistory.at(2) == n && runHistory.at(3) == n * 3 && runHistory.at(4) == n && runHistory.at(5) == n;
        return (core && runHistory.at(0) >= n * 4 && runHistory.at(6) >= n ? 1 : 0)
               + (core && runHistory.at(6) >= n * 4 && runHistory.at(0) >= n ? 1 : 0);
    }

    int finderPenaltyTerminateAndCount(bool currentRunColor, int currentRunLength, array<int, 7>& runHistory) const {
        if (currentRunColor) {
            finderPenaltyAddHistory(currentRunLength, runHistory);
            currentRunLength = 0;
        }
        currentRunLength += size;
        finderPenaltyAddHistory(currentRunLength, runHistory);
        return finderPenaltyCountPatterns(runHistory);
    }

    void finderPenaltyAddHistory(int currentRunLength, array<int, 7>& runHistory) const {
        if (runHistory.at(0) == 0)
            currentRunLength += size;
        copy_backward(runHistory.begin(), runHistory.end() - 1, runHistory.end());
        runHistory.at(0) = currentRunLength;
    }

    vector<uint8_t> addECB(const vector<uint8_t>& data) const {
        int numBlocks = BLOCKS_BY_CORR_LVL[static_cast<int>(levelOfCorrection)][version];
        int blockEccLen = CORR_BYTES_BY_CORR_LVL[static_cast<int>(levelOfCorrection)][version];
        int rawCodewords = (MAX_DATA_BY_CORR_LVL[static_cast<int>(levelOfCorrection)][version] / 8 + numBlocks * blockEccLen);
        int numShortBlocks = numBlocks - rawCodewords % numBlocks;
        int shortBlockLen = rawCodewords / numBlocks;
        vector<vector<uint8_t> > blocks;
        for (int i = 0, k = 0; i < numBlocks; i++) {
            vector<uint8_t> dat(data.cbegin() + k, data.cbegin() + (k + shortBlockLen - blockEccLen + (i < numShortBlocks ? 0 : 1)));
            k += static_cast<int>(dat.size());
            const vector<uint8_t> ecc = createECBPart(dat, blockEccLen);
            if (i < numShortBlocks)
                dat.push_back(0);
            dat.insert(dat.end(), ecc.cbegin(), ecc.cend());
            blocks.push_back(move(dat));
        }

        vector<uint8_t> result;
        for (size_t i = 0; i < blocks.at(0).size(); i++) {
            for (size_t j = 0; j < blocks.size(); j++) {
                if (i != static_cast<unsigned int>(shortBlockLen - blockEccLen) || j >= static_cast<unsigned int>(numShortBlocks))
                    result.push_back(blocks.at(j).at(i));
            }
        }
        return result;
    }

    vector<uint8_t> createECBPart(const vector<uint8_t>& data, int ecbQuantity = 28) const {
        vector<uint8_t> toReturn = data;
        vector<int> polynom = GENERATOR_POLYNOMS.at(ecbQuantity);
        toReturn.resize(max((int)data.size(), ecbQuantity), 0);
        for (size_t i = 0; i < data.size(); i++) {
            int A = toReturn[0];
            toReturn.erase(toReturn.begin(), toReturn.begin() + 1);
            toReturn.push_back(0);
            if (A == (uint8_t)0) {
                continue;
            }
            int B = GALUAINVERSEDFIELD[A];
            for (int j = 0; j < ecbQuantity; j++) {
                int C = polynom[j] + B;
                if (C > 254) {
                    C = C % 255;
                }
                int D = GALUAFIELD[C] ^ toReturn[j];
                toReturn[j] = D;
            }
        }
        return toReturn;
    }

    string toSvgString(int border) const {
        ostringstream sb;
        sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
        sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
        sb << (size + border * 2) << " " << (size + border * 2) << "\" stroke=\"none\">\n";
        sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
        sb << "\t<path d=\"";
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (modules[x][y]) {
                    if (x != 0 || y != 0)
                        sb << " ";
                    sb << "M" << (x + border) << " " << (y + border) << " h1v1h-1z";
                }
            }
        }
        sb << "\" fill=\"#000000\"/>\n";
        sb << "</svg>\n";
        return sb.str();
    }

private:
    int version;
    int size;
    int mask;
    ErrCorrLevel levelOfCorrection;
    vector<vector<bool>> modules;
    vector<vector<bool>> isFunction;

    void setModule(int x, int y, bool isBlack) {
        modules[y][x] = isBlack;
        isFunction[y][x] = true;
    }

    bool getModule(int x, int i) {
        return ((x >> i) & 1) != 0;
    }
};
