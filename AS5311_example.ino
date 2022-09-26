#include "AS5311.h"
#include "Arduino.h"
#include "uTimerLib.h"

constexpr uint16_t PinData{1};
constexpr uint16_t PinClock{24};
constexpr uint16_t PinSel{0};
constexpr unsigned long Timeout{500000};

AS5311 as5311(PinData, PinClock, PinSel);

double axisValue{0};

void timed_function()
{
    if (as5311.isValueValid(AS5311::ValueType::PositionValue))
    {
        int encoderVal = as5311.value(AS5311::ValueType::PositionValue);
        float distance = as5311.distance();

        Serial.print("Pulse: ");
        Serial.print(encoderVal);
        Serial.print("  ");
        Serial.print(axisValue);
        Serial.print("um");
        Serial.print("\r");
    }
    else
    {
        axisValue = 0;
        Serial.print("Pulse: 0000");
        Serial.print("  ");
        Serial.print(axisValue);
        Serial.print("um");
        Serial.print("\r");
    }
}

void setup()
{
    constexpr uint32_t BaudRate{9600};
    Serial.begin(BaudRate);
    TimerLib.setInterval_us(timed_function, Timeout);
}

void loop()
{
    as5311.updateData(AS5311::ValueType::PositionValue);
    axisValue += as5311.distance();
}
