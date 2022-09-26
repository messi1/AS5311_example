#include "AS5311.h"
#include "Arduino.h"

AS5311::AS5311(uint8_t dataPin, uint8_t clockPin, uint8_t chipSelectPin)
    : mDataPin(dataPin), mClockPin(clockPin), mSelectPin(chipSelectPin)
{
    constexpr unsigned long Delay{100};
    pinMode(mDataPin, INPUT_PULLUP);
    pinMode(mClockPin, OUTPUT);
    pinMode(mSelectPin, OUTPUT);

    // Turn on the AS5311
    digitalWrite(mSelectPin, HIGH);
    digitalWrite(mClockPin, LOW);
    delay(Delay);
}

void AS5311::updateData(ValueType valueData)
{
    constexpr float MircoMeterPerPulse{0.48828125};

    if (valueData == ValueType::PositionValue)
    {
        uint32_t totalValue{0};

        for (int i = 0; i < mAverageFactor; ++i)
        {
            mAngularPositionData.rawValue = readChip(ValueType::PositionValue);
            totalValue += mAngularPositionData.value;
        }

        mPreviousAvgValue = mCurrentAvgValue;
        mCurrentAvgValue = totalValue / mAverageFactor;
        mDistance = calculateDistance() * MircoMeterPerPulse;
    }
    else
    {
        mMagneticFieldData.rawValue = readChip(ValueType::MagneticValue);
    }
}

int16_t AS5311::calculateDistance()
{
    constexpr uint16_t MinPusleValue{0};
    constexpr uint16_t MaxPusleValue{4095};
    uint16_t diffDistanceOverLimit{0};
    const uint16_t diffDistanceBetweenValues = abs(mPreviousAvgValue - mCurrentAvgValue);

    if (mCurrentAvgValue > mPreviousAvgValue)
    {
        diffDistanceOverLimit = (MaxPusleValue - mCurrentAvgValue) + (mPreviousAvgValue - MinPusleValue);
    }
    else if (mPreviousAvgValue > mCurrentAvgValue)
    {
        diffDistanceOverLimit = (MaxPusleValue - mPreviousAvgValue) + (mCurrentAvgValue - MinPusleValue);
    }
    else
    {
        return 0;
    }

    if (diffDistanceBetweenValues < diffDistanceOverLimit)
    {
        return (mPreviousAvgValue > mCurrentAvgValue) ? (-1) * diffDistanceBetweenValues :
                                                        diffDistanceBetweenValues;
    }
    else
    {
        return (mCurrentAvgValue > mPreviousAvgValue) ? (-1) * diffDistanceOverLimit : diffDistanceOverLimit;
    }
}

float AS5311::distance() const
{
    return mDistance;
}

uint16_t AS5311::value(ValueType valueData) const
{
    if (valueData == ValueType::PositionValue)
    {
        return mCurrentAvgValue;
    }
    else
    {
        return mMagneticFieldData.value;
    }
}

bool AS5311::isValueValid(ValueType valueData) const
{
    if (valueData == ValueType::PositionValue)
    {
        return (checkParityIsEven(mAngularPositionData.rawValue) && mAngularPositionData.OCF &&
                not mAngularPositionData.COF && not mAngularPositionData.LIN);
    }
    else
    {
        return (checkParityIsEven(mMagneticFieldData.rawValue) && mMagneticFieldData.OCF &&
                not mMagneticFieldData.COF && not mMagneticFieldData.LIN);
    }
}

void AS5311::setAverageFactor(uint8_t factor)
{
    if (mAverageFactor != factor)
    {
        mAverageFactor = factor;
    }
}

uint8_t AS5311::averageFactor() const
{
    return mAverageFactor;
}

uint32_t AS5311::readChip(ValueType valueData)
{
    constexpr uint8_t NoOfBits{18};
    // constexpr unsigned long Delay{1};
    uint32_t rawValue{0};

    digitalWrite(mSelectPin, HIGH);

    if (valueData == ValueType::PositionValue)
    {
        digitalWrite(mClockPin, HIGH);

        // delayMicroseconds(Delay);
        digitalWrite(mSelectPin, LOW);
        // delayMicroseconds(Delay);

        digitalWrite(mClockPin, LOW);
        // delayMicroseconds(Delay);
    }

    if (valueData == ValueType::MagneticValue)
    {
        digitalWrite(mClockPin, LOW);
        // delayMicroseconds(Delay);

        digitalWrite(mSelectPin, LOW);
        // delayMicroseconds(Delay);
    }

    for (uint8_t c = 0; c < NoOfBits; ++c)
    {
        digitalWrite(mClockPin, HIGH);
        // delayMicroseconds(Delay);
        digitalWrite(mClockPin, LOW);
        int inputstream = digitalRead(mDataPin);
        rawValue = ((rawValue << 1u) + inputstream);
    }
    digitalWrite(mClockPin, HIGH);
    digitalWrite(mSelectPin, HIGH);

    return rawValue;
}

bool AS5311::checkParityIsEven(const uint32_t value) const
{
    constexpr uint8_t NoOfBits{18};
    constexpr uint8_t Two{2};
    uint8_t count = 0;
    uint32_t b = 1;

    for (uint8_t i = 0; i < NoOfBits; ++i)
    {
        if (value & (b << i))
        {
            count++;
        }
    }

    if ((count % Two))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void AS5311::rangeIndication()
{
    constexpr uint32_t LowerLimit{0x20};
    constexpr uint32_t UpperLimit{0x5f};
    constexpr uint32_t OkayValue{0x3f};

    if (mAngularPositionData.MagINC && mAngularPositionData.MagDEC && mAngularPositionData.LIN &&
        (mMagneticFieldData.value < LowerLimit || mMagneticFieldData.value > UpperLimit))
    {
        // return red
    }

    if (mAngularPositionData.MagINC && mAngularPositionData.MagDEC && not mAngularPositionData.LIN &&
        (mMagneticFieldData.value > LowerLimit && mMagneticFieldData.value < UpperLimit))
    {
        // return yellow
    }

    if (not(mAngularPositionData.MagINC && mAngularPositionData.MagDEC) && not mAngularPositionData.LIN &&
        mMagneticFieldData.value == OkayValue)
    {
        // return greem
    }
}