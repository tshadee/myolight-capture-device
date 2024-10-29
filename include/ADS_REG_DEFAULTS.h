#ifndef ADS_REG_DEFAULTS_H
#define ADS_REG_DEFAULTS_H

#define CONFIG_DEFAULT_CFG 0x8460
#define RANGE_A1_DEFAULT_CFG 0x8855
#define RANGE_A2_DEFAULT_CFG 0x8A55
#define RANGE_B1_DEFAULT_CFG 0x8C55
#define RANGE_B2_DEFAULT_CFG 0x8E55
#define LPF_DEFAULT_CFG 0x9A02
#define SEQ_STACK_0_DEFAULT_CFG 0xC047
#define SEQ_STACK_1_DEFAULT_CFG 0xC256
#define SEQ_STACK_2_DEFAULT_CFG 0xC465
#define SEQ_STACK_3_DEFAULT_CFG 0xC57B

#define CONFIGURATION__REG_ADDR static_cast<uint8_t>(0x2)
#define CHANNEL_SEL_REG_ADDR static_cast<uint8_t>(0x3)
#define RANGE_A1_REG_ADDR static_cast<uint8_t>(0x4)
#define RANGE_A2_REG_ADDR static_cast<uint8_t>(0x5)
#define RANGE_B1_REG_ADDR static_cast<uint8_t>(0x6)
#define RANGE_B2_REG_ADDR static_cast<uint8_t>(0x7)
#define STATUS_REG_ADDR static_cast<uint8_t>(0x8)
#define OVERRANGE_SET_A_REG_ADDR static_cast<uint8_t>(0xA)
#define OVERRANGE_SET_B_REG_ADDR static_cast<uint8_t>(0xB)
#define LPF_CONFIG_REG_ADDR static_cast<uint8_t>(0xD)
#define SEQ_STACK_BASE_REG_ADDR static_cast<uint8_t>(0x20)
#define SEQ_STACK_REG_ADDR(n) static_cast<uint8_t>(SEQ_STACK_BASE_REG + (n))  //until n = 31

#define READBACK_MASK 0x7E00

enum class cfgProfile
{
    Default,
    SingleSample,
    SingleChannel,
    SweepManualCS,
    SweepBurstSEQEN,
    HighOSR
};

#endif //ADS_REG_DEFAULTS_H