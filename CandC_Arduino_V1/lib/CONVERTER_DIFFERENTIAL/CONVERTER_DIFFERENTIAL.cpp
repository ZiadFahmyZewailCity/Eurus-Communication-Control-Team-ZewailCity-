#include "CONVERTER_DIFFERENTIAL.hpp"
#include "CONVERTER_DIFFERENTIAL_config.hpp"


float filtered_voltage_VA = 0;
float filtered_voltage_VB = 0;

void differentialVoltage_config()
{
    pinMode(differentialVoltage_VA_PIN,INPUT);
    pinMode(differentialVoltage_VB_PIN,INPUT);
}


float differentialVoltage_measurment()
{

    uint16_t rawValue_ADC_VA = analogRead(differentialVoltage_VA_PIN);
    uint16_t rawValue_ADC_VB = analogRead(differentialVoltage_VB_PIN);

    float differentialVoltage_value_VA = ((float)rawValue_ADC_VA / 1024.0) * v_ref;
    float differentialVoltage_value_VB = ((float)rawValue_ADC_VB / 1024.0) * v_ref;

    //IIR Filter to remove high frequency changes
    filtered_voltage_VA = (filterParamerter * differentialVoltage_value_VA) + ((1.0 - filterParamerter) * filtered_voltage_VA);
    filtered_voltage_VB = (filterParamerter * differentialVoltage_value_VB) + ((1.0 - filterParamerter) * filtered_voltage_VB);

    return (filtered_voltage_VA - filtered_voltage_VB)* differentialVotlage_scaleFactor;

}