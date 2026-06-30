#include "POTENTIAL_DIVIDER.hpp"
#include "POTENTIAL_DIVIDER_config.hpp"


void voltage_PD_configure()
{
    pinMode(voltageDividerPin,INPUT);
}

float voltage_PD_MeasureValue()
{
    //Read Raw Value
    uint16_t rawValue_ADC = analogRead(voltageDividerPin);

    float voltage_stepDown = ((float)rawValue_ADC / 1024.0) * v_ref;

    return voltageDividerFactor * voltage_stepDown;

}