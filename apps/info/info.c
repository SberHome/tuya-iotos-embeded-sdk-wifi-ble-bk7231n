#include "net.h"
#include "rtos_pub.h"
#include "wlan_ui_pub.h"
#include <assert.h>

#define APP_DEBUG 1
#define print_inf(...)              \
    do                              \
    {                               \
        if (APP_DEBUG)              \
            os_printf(__VA_ARGS__); \
    } while (0)

typedef struct
{
    uint32_t Mode : 5; // Mode
    uint32_t T : 1;    // 1 - Thumb state, 0 - Arm state
    uint32_t F : 1;    // 1 - FIQ masked, 0 - FIQ not masked
    uint32_t I : 1;    // 1 - IRQ masked, 0 - IRQ not masked
    uint32_t A : 1;    // 1 - Async abort
    uint32_t E : 1;    // 1 - Big-endian, 0 - Little-endian
    uint32_t : 18;     // reserved
    uint32_t V : 1;    // Overflow flag
    uint32_t C : 1;    // Carry flag
    uint32_t Z : 1;    // Zero flag
    uint32_t N : 1;    // Negative flag
} ARM968es_CPSR_t;
static_assert(sizeof(ARM968es_CPSR_t) == 4, "ARM968es_CPSR_t incorrect size");

typedef struct
{
    uint32_t minor_rev : 4;
    uint32_t part_number : 12;
    uint32_t architecture : 4;
    uint32_t major_rev : 4;
    uint32_t implementer : 8;
} ARM968es_CP15_DeviceID_t;
static_assert(sizeof(ARM968es_CP15_DeviceID_t) == 4, "ARM968es_CP15_DeviceID_t incorrect size");

typedef struct
{
    uint32_t : 2;
    uint32_t ITCM_absent : 1;
    uint32_t : 4;
    uint32_t ITCM_size : 5;
    uint32_t : 3;
    uint32_t DTCM_absent : 1;
    uint32_t : 3;
    uint32_t DTCM_size : 5;
    uint32_t : 8;
} ARM968es_CP15_TCM_size_t;
static_assert(sizeof(ARM968es_CP15_TCM_size_t) == 4, "ARM968es_CP15_TCM_size_t incorrect size");

typedef struct
{
    uint32_t SBZ_1 : 1; // Should Be Zero
    uint32_t A : 1;
    uint32_t D : 1;
    uint32_t W : 1;
    uint32_t SBO_1 : 3; // Should Be One
    uint32_t B : 1;
    uint32_t SBO_2 : 4; // Should Be One
    uint32_t I : 1;
    uint32_t V : 1;
    uint32_t SBZ_2 : 1; // Should Be Zero
    uint32_t LT : 1;
    uint32_t SBZ_3 : 16; // Should Be Zero
} ARM968es_CP15_control_reg_t;
static_assert(sizeof(ARM968es_CP15_control_reg_t) == 4, "ARM968es_CP15_control_reg_t incorrect size");

uint32_t platform_cpsr_content(void);

uint32_t __attribute__((target("arm"))) get_device_id_reg()
{
    uint32_t value;
    __asm volatile(
        "MRC p15, 0, %0, c0, c0, 0;  \n"
        : "=r"(value)
        :
        : "memory");
    return value;
}

uint32_t __attribute__((target("arm"))) get_TCM_size_reg()
{
    uint32_t value;
    __asm volatile(
        "MRC p15, 0, %0, c0, c0, 2;  \n"
        : "=r"(value)
        :
        : "memory");
    return value;
}

uint32_t __attribute__((target("arm"))) get_control_reg()
{
    uint32_t value;
    __asm volatile(
        "MRC p15, 0, %0, c1, c0, 0;  \n"
        : "=r"(value)
        :
        : "memory");
    return value;
}

const char *print_CPSR_mode(uint32_t m)
{
    switch (m)
    {
    case 0b10000:
        return "User mode";
    case 0b10001:
        return "FIQ mode";
    case 0b10010:
        return "IRQ mode";
    case 0b10011:
        return "Supervisor mode";
    case 0b10111:
        return "Abort mode";
    case 0b11011:
        return "Undefined mode";
    case 0b11111:
        return "System mode";
    default:
        return "UNKNOWN mode";
    }
}

const char *print_implementer_name(uint32_t imp)
{
    switch (imp)
    {
    case 0x41:
        return "Arm Limited";
    case 0x44:
        return "Digital Equipment Corporation";
    case 0x4D:
        return "Motorola";
    case 0x56:
        return "Marvell Semiconductor Inc.";
    case 0x69:
        return "Intel Corporation";
    default:
        return "Unknown";
    }
}

const char *print_architecture_name(uint32_t arch)
{
    switch (arch)
    {
    case 0x01:
        return "v4";
    case 0x02:
        return "v4T";
    case 0x03:
        return "v5";
    case 0x04:
        return "v5T";
    case 0x05:
        return "v5TE";
    case 0x06:
        return "v5TEJ";
    case 0x07:
        return "v6";
    case 0x08:
        return "Revised CPUID";
    default:
        return "Unknown";
    }
}

void print_CPSR_info(uint32_t cpsr)
{
    ARM968es_CPSR_t cpsrt = *((ARM968es_CPSR_t *)(&cpsr));

    print_inf("\n");
    print_inf("CPSR\n");
    print_inf("CPSR is: 0x%x\n", cpsr);

    if (cpsrt.E == 0x01)
        print_inf("E=1 Big endian\n");
    else
        print_inf("E=0 Little endian\n");

    if (cpsrt.A == 0x01)
        print_inf("A=1 Async abort masked\n");
    else
        print_inf("A=0 Async abort not masked\n");

    if (cpsrt.I == 0x01)
        print_inf("I=1 IRQ masked\n");
    else
        print_inf("I=0 IRQ not masked\n");

    if (cpsrt.T == 0x01)
        print_inf("T=1 Thumb mode\n");
    else
        print_inf("T=0 Arm mode\n");

    if (cpsrt.F == 0x01)
        print_inf("F=1 FIQ masked\n");
    else
        print_inf("F=0 FIQ not masked\n");

    print_inf("Mode: 0x%x - %s\n", cpsrt.Mode, print_CPSR_mode(cpsrt.Mode));
}

const char *print_TCM_size(uint32_t sz)
{
    switch (sz)
    {
    case 0b00000:
        return "None";
    case 0b00001:
        return "1KB";
    case 0b00010:
        return "2KB";
    case 0b00011:
        return "4KB";
    case 0b00100:
        return "8KB";
    case 0b00101:
        return "16KB";
    case 0b00110:
        return "32KB";
    case 0b00111:
        return "64KB";
    case 0b01000:
        return "128KB";
    case 0b01001:
        return "256KB";
    case 0b01010:
        return "512KB";
    case 0b01011:
        return "1MB";
    case 0b01100:
        return "2MB";
    case 0b01101:
        return "4MB";
    default:
        return "UNKNOWN";
    }
}

void print_DeviceID_info(uint32_t device_id_reg)
{
    ARM968es_CP15_DeviceID_t devid = *((ARM968es_CP15_DeviceID_t *)(&device_id_reg));

    print_inf("\n");
    print_inf("Device ID\n");
    print_inf("device_id_reg is: 0x%x\n", device_id_reg);
    print_inf("Implementer: 0x%x - %s\n", devid.implementer, print_implementer_name(devid.implementer));
    print_inf("Major revision: 0x%x\n", devid.major_rev);
    print_inf("Architecture: 0x%x - %s\n", devid.architecture, print_architecture_name(devid.architecture));
    print_inf("Part number: 0x%x\n", devid.part_number);
    print_inf("Minor revision: 0x%x\n", devid.minor_rev);
}

void print_TCM_size_info(uint32_t TCM_size_reg)
{
    ARM968es_CP15_TCM_size_t tcm = *((ARM968es_CP15_TCM_size_t *)(&TCM_size_reg));

    print_inf("\n");
    print_inf("TCM Size\n");
    print_inf("TCM_size_reg is: 0x%x\n", TCM_size_reg);
    print_inf("DTCM absent: 0x%x\n", tcm.DTCM_absent);
    print_inf("DTCM size: 0x%x - %s\n", tcm.DTCM_size, print_TCM_size(tcm.DTCM_size));
    print_inf("ITCM absent: 0x%x\n", tcm.ITCM_absent);
    print_inf("ITCM size: 0x%x - %s\n", tcm.ITCM_size, print_TCM_size(tcm.ITCM_size));
}

void print_control_reg_info(uint32_t control_reg)
{
    ARM968es_CP15_control_reg_t cr = *((ARM968es_CP15_control_reg_t *)(&control_reg));

    print_inf("\n");
    print_inf("Control reg\n");
    print_inf("control_reg is: 0x%x\n", control_reg);
    print_inf("SBZ_1: 0x%x - %s\n", cr.SBZ_1, (cr.SBZ_1 == 0x00) ? "OK" : "ERROR");
    print_inf("A: 0x%x - fault checking of address alignment %s\n", cr.A, (cr.A == 1) ? "enabled" : "disabled");
    print_inf("D: 0x%x - data accesses use %s interface\n", cr.D, (cr.D == 1) ? "DTCM" : "AHB");
    print_inf("W: 0x%x - AHB write buffer %s\n", cr.W, (cr.W == 1) ? "enabled" : "disabled");
    print_inf("SBO_1: 0x%x - %s\n", cr.SBO_1, (cr.SBO_1 == 0b111) ? "OK" : "ERROR");
    print_inf("B: 0x%x - %s-endian memory mapping\n", cr.B, (cr.B == 1) ? "big" : "little");
    print_inf("SBO_2: 0x%x - %s\n", cr.SBO_2, (cr.SBO_2 == 0b1111) ? "OK" : "ERROR");
    print_inf("I: 0x%x - instruction accesses use %s interface\n", cr.I, (cr.I == 1) ? "ITCM" : "AHB");
    print_inf("V: 0x%x - vector address range is %s\n", cr.V, (cr.V == 1) ? "0xFFFF0000 to 0xFFFF001C" : "0x00000000 to 0x0000001C");
    print_inf("SBZ_2: 0x%x - %s\n", cr.SBZ_2, (cr.SBZ_2 == 0b0) ? "OK" : "ERROR");
    print_inf("LT: 0x%x - %s\n", cr.LT, (cr.LT == 1) ? "loading PC does not set T bit" : "loading PC sets T bit");
    print_inf("SBZ_3: 0x%x - %s\n", cr.SBZ_3, (cr.SBZ_3 == 0x00) ? "OK" : "ERROR");
}

void entry_main(void)
{

    uint32_t cpsr = platform_cpsr_content();
    print_CPSR_info(cpsr);

    uint32_t device_id_reg = get_device_id_reg();
    print_DeviceID_info(device_id_reg);

    uint32_t TCM_size_reg = get_TCM_size_reg();
    print_TCM_size_info(TCM_size_reg);

    uint32_t control_reg = get_control_reg();
    print_control_reg_info(control_reg);

    while (true);
}