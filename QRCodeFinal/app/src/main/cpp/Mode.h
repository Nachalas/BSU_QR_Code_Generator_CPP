#pragma once

class Mode {
public:

    Mode(int mode, int charCount1, int charCount2, int charCount3) : modeIndicator(mode) {
        sizeIndicator[0] = charCount1;
        sizeIndicator[1] = charCount2;
        sizeIndicator[2] = charCount3;
    }

    int getCharCount(int version) const {
        return sizeIndicator[(version + 7) / 17];
    }

    int getMode() const {
        return modeIndicator;
    }

    static const Mode NUMERIC;
    static const Mode ALPHANUMERIC;
    static const Mode BYTE;

private:
    const int modeIndicator;
    int sizeIndicator[3];
};

const Mode Mode::NUMERIC(1, 10, 12, 14);
const Mode Mode::ALPHANUMERIC(2, 9, 11, 13);
const Mode Mode::BYTE(4, 8, 16, 16);
