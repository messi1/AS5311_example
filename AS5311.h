#pragma once

#include "WString.h"
#include "stdint.h"

class AS5311
{
  public:
    enum class ValueType
    {
        PositionValue,
        MagneticValue
    };

  public:
    AS5311(uint8_t dataPin, uint8_t clockPin, uint8_t chipSelectPin);
    void updateData(ValueType valueData);
    uint16_t value(ValueType valueData) const;
    float distance() const;
    bool isValueValid(ValueType valueData) const;
    void rangeIndication();
    void setAverageFactor(uint8_t factor);
    uint8_t averageFactor() const;

  private:
    int16_t calculateDistance();
    uint32_t readChip(ValueType valueData);
    bool checkParityIsEven(const uint32_t value) const;

  private:
    const uint8_t mClockPin;
    const uint8_t mSelectPin;
    const uint8_t mDataPin;

    union SerialData
    {
        struct
        {
            const bool PAR : 1;
            const bool MagDEC : 1;
            const bool MagINC : 1;
            const bool LIN : 1;
            const bool COF : 1;
            const bool OCF : 1;
            const uint32_t value : 12;
        };
        uint32_t rawValue;
    };

    SerialData mMagneticFieldData{0};
    SerialData mAngularPositionData{0};

    uint8_t mAverageFactor{8};
    uint16_t mCurrentAvgValue{0};
    uint16_t mPreviousAvgValue{0};
    float mDistance{0.0f};
};
