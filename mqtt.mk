APP_NAME = mqtt
APP_VERSION = 1.0.0
SOC_NAME = bk7231n

INCLUDES =
SRC_C =

BUILD_DIR = build
BEKEN_DIR = platforms/bk7231n/bk7231n_os

TARGET_NAME=$(APP_NAME)_$(APP_VERSION)
OBJ_DIR=$(BUILD_DIR)/$(TARGET_NAME)/obj
BIN_DIR=$(BUILD_DIR)/$(TARGET_NAME)

OUTPUT_FULL_NAME=$(BIN_DIR)/$(APP_NAME)_$(APP_VERSION)

SOC_NAME_LDS = $(BEKEN_DIR)/beken378/build/bk7231n_ota.ld

DEBUG_DEFINES = 
DEBUG_DEFINES += -DMQTT_DEBUG=0
DEBUG_DEFINES += -DJL_DEBUG=1
DEBUG_DEFINES += -DAPP_DEBUG=1
DEBUG_DEFINES += -DRTOS_PUB_DEBUG=1
DEBUG_DEFINES += -DWLAN_UI_DEBUG=1
DEBUG_DEFINES += -DAPP_BK_DEBUG=0
DEBUG_DEFINES += -DROLE_LAUNCH_DEBUG=1
#DEBUG_DEFINES += -DSAAP_DEBUG=1
#DEBUG_DEFINES += -DSASTA_DEBUG=1


ENCRYPT = $(BEKEN_DIR)/tools/generate/package_tool/windows/encrypt.exe
ENCRYPT_ARGS = 0 0 0

CFG_BLE_5X_VERSION ?= 1
CFG_BLE_5X_RW ?= 1
CFG_BLE_5X_USE_RWIP_LIB ?= 1
CFG_GIT_VERSION ?= ""
CFG_WRAP_LIBC  ?= 1

ifeq ($(shell uname), Linux)
  TOOLCHAIN_DIR := ../toolchain
else
  TOOLCHAIN_DIR := C:/Program Files (x86)/GNU Arm Embedded Toolchain
endif
CROSS_COMPILE = $(TOOLCHAIN_DIR)/10 2020-q4-major/bin/arm-none-eabi-

# Compilation tools
AR = "$(CROSS_COMPILE)ar"
CC = "$(CROSS_COMPILE)gcc"
AS = "$(CROSS_COMPILE)as"
NM = "$(CROSS_COMPILE)nm"
LD = "$(CROSS_COMPILE)gcc"
GDB = "$(CROSS_COMPILE)gdb"
OBJCOPY = "$(CROSS_COMPILE)objcopy"
OBJDUMP = "$(CROSS_COMPILE)objdump"

# -------------------------------------------------------------------
# Include folder list
# -------------------------------------------------------------------


INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/client
INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/client/src
INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/packet/src
INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/mqtt_ui
INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/mqtt_ui/ssl_mqtt
INCLUDES += -Iapps/$(APP_NAME)/paho-mqtt/mqtt_ui/tcp_mqtt

SRC_C += apps/$(APP_NAME)/paho-mqtt/client/paho_mqtt_udp.c
SRC_C += apps/$(APP_NAME)/paho-mqtt/packet/src/MQTTPacket.c
SRC_C += apps/$(APP_NAME)/paho-mqtt/packet/src/MQTTConnectClient.c
SRC_C += apps/$(APP_NAME)/paho-mqtt/packet/src/MQTTSubscribeClient.c
SRC_C += apps/$(APP_NAME)/paho-mqtt/packet/src/MQTTDeserializePublish.c
SRC_C += apps/$(APP_NAME)/paho-mqtt/packet/src/MQTTSerializePublish.c

# Beken SDK include folder and source file list

INCLUDES += -I$(BEKEN_DIR)/beken378/common
INCLUDES += -I$(BEKEN_DIR)/beken378/app
INCLUDES += -I$(BEKEN_DIR)/beken378/app/config
INCLUDES += -I$(BEKEN_DIR)/beken378/app/standalone-station
INCLUDES += -I$(BEKEN_DIR)/beken378/app/standalone-ap
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/common
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/ke/
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/mac/
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/hal
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/mm
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/ps
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/rd
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/rwnx
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/rx
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/scan
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/sta
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/tx
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/vif
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/rx/rxl
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/tx/txl
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/p2p
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/chan
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/td
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/tpc
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/lmac/src/tdls
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/mesh
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/rc
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/apm
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/bam
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/llc
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/me
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/rxu
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/scanu
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/sm
INCLUDES += -I$(BEKEN_DIR)/beken378/ip/umac/src/txu
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/include
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/common/reg
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/entry
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/dma
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/intc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/phy
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/pwm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/rc_beken
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/flash
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/rw_pub
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/common/reg
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/common
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/uart
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/sys_ctrl
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/gpio
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/general_dma
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/spidma
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/icu
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/spi
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/ip/ble/hl/inc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/ip/ble/profiles/sdp/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/ip/ble/profiles/comm/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/rwip/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/app/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/common/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/dbg/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/rwip/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/rf/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/modules/ecc_p256/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/arch 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/arch/boot 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/arch/compiler 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/arch/ll 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/arch                                                     
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/build/ble_full/reg/fw                                    
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/driver/reg                                               
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_pub/plf/refip/src/driver/ble_icu                                           
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/inc                                                          
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/api                                                          
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapm 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpm 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gap 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/attc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/attm 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/atts 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattm 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cm 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/ll/src/rwble                                                    
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/ll/src/lld                                                      
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/ll/src/em                                                       
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/ll/src/llm                                                      
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ble/ll/src/llc                                                      
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/em/api                                                              
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ea/api                                                              
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/hci/api 
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/hci/src                                                             
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/ip/ahi/api                                                             
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/modules/ke/api                                                         
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/modules/ke/src                                                         
#INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble/ble_lib/modules/h4tl/api  
INCLUDES += -I$(BEKEN_DIR)/beken378/func/include
INCLUDES += -I$(BEKEN_DIR)/beken378/func/rf_test
INCLUDES += -I$(BEKEN_DIR)/beken378/func/user_driver
INCLUDES += -I$(BEKEN_DIR)/beken378/func/power_save
INCLUDES += -I$(BEKEN_DIR)/beken378/func/uart_debug
INCLUDES += -I$(BEKEN_DIR)/beken378/func/ethernet_intf
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/hostapd
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/bk_patch
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/src/utils
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/src/common
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/src/drivers
INCLUDES += -I$(BEKEN_DIR)/beken378/func/hostapd-2.5/src
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/port
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/include
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/include/netif
INCLUDES += -I$(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/include/lwip
INCLUDES += -I$(BEKEN_DIR)/beken378/func/temp_detect
INCLUDES += -I$(BEKEN_DIR)/beken378/func/spidma_intf
INCLUDES += -I$(BEKEN_DIR)/beken378/func/rwnx_intf
INCLUDES += -I$(BEKEN_DIR)/beken378/func/joint_up
INCLUDES += -I$(BEKEN_DIR)/beken378/func/bk_tuya_pwm
INCLUDES += -I$(BEKEN_DIR)/beken378/os/include
INCLUDES += -I$(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es
INCLUDES += -I$(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/include
INCLUDES += -I$(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0

ifeq ($(CFG_WRAP_LIBC),1)
INCLUDES += -I$(BEKEN_DIR)/beken378/func/libc
endif

#zephyr head start
#ble include
#INCLUDES += -I./beken378/driver/ble_5_x/arch/armv5
#INCLUDES += -I./beken378/driver/ble_5_x/arch/armv5/compiler
#INCLUDES += -I./beken378/driver/ble_5_x/arch/armv5/ll

#INCLUDES += -I./beken378/driver/ble_5_x/platform/7231n/config
#INCLUDES += -I./beken378/driver/ble_5_x/platform/7231n/rwip/api
#INCLUDES += -I./beken378/driver/ble_5_x/platform/7231n/rwip/import/reg
#INCLUDES += -I./beken378/driver/ble_5_x/platform/7231n/driver/reg

#INCLUDES += -I./beken378/driver/ble_5_x/sys/common/api
#INCLUDES += -I./beken378/driver/ble_5_x/sys/ke/api

#INCLUDES += -I./beken378/driver/ble_5_x/modules/dbg/api
#INCLUDES += -I./beken378/driver/ble_5_x/modules/aes/api
#INCLUDES += -I./beken378/driver/ble_5_x/modules/ecc_p256/api

#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/api
#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/import/reg
#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/src
#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/src/llc
#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/src/lld
#INCLUDES += -I./beken378/driver/ble_5_x/ip/ble/ll/src/llm
#INCLUDES += -I./beken378/driver/ble_5_x/ip/em/api
#INCLUDES += -I./beken378/driver/ble_5_x/ip/hci/api
#INCLUDES += -I./beken378/driver/ble_5_x/ip/hci/src
#INCLUDES += -I./beken378/driver/ble_5_x/ip/sch/api
#INCLUDES += -I./beken378/driver/ble_5_x/ip/sch/import

#INCLUDES += -I./beken378/driver/ble_5_x/zephyr/host/include
#INCLUDES += -I./beken378/driver/ble_5_x/zephyr/port/include
#zephyr head end

# -------------------------------------------------------------------
# Source file list
# -------------------------------------------------------------------
SRC_OS =


#myapp
SRC_C += apps/$(APP_NAME)/mqtt.c

SRC_C += $(BEKEN_DIR)/beken378/os/mem_arch.c
SRC_C += $(BEKEN_DIR)/beken378/os/platform_stub.c
SRC_C += $(BEKEN_DIR)/beken378/os/str_arch.c

#application layer
SRC_C += $(BEKEN_DIR)/beken378/app/app_bk.c
SRC_C += $(BEKEN_DIR)/beken378/app/ate_app.c
SRC_C += $(BEKEN_DIR)/beken378/app/config/param_config.c
SRC_C += $(BEKEN_DIR)/beken378/app/standalone-ap/sa_ap.c
SRC_C += $(BEKEN_DIR)/beken378/app/standalone-station/sa_station.c

#demo module
SRC_C += $(BEKEN_DIR)/beken378/demo/ieee802_11_demo.c

#driver layer
SRC_C += $(BEKEN_DIR)/beken378/driver/common/dd.c
SRC_C += $(BEKEN_DIR)/beken378/driver/common/drv_model.c
SRC_C += $(BEKEN_DIR)/beken378/driver/dma/dma.c
SRC_C += $(BEKEN_DIR)/beken378/driver/driver.c
SRC_C += $(BEKEN_DIR)/beken378/driver/entry/arch_main.c
#SRC_C += ./beken378/driver/fft/fft.c
SRC_C += $(BEKEN_DIR)/beken378/driver/flash/flash.c
SRC_C += $(BEKEN_DIR)/beken378/driver/general_dma/general_dma.c
SRC_C += $(BEKEN_DIR)/beken378/driver/gpio/gpio.c
#SRC_C += ./beken378/driver/i2s/i2s.c
SRC_C += $(BEKEN_DIR)/beken378/driver/icu/icu.c
SRC_C += $(BEKEN_DIR)/beken378/driver/intc/intc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/irda/irda.c
SRC_C += $(BEKEN_DIR)/beken378/driver/macphy_bypass/mac_phy_bypass.c
SRC_C += $(BEKEN_DIR)/beken378/driver/phy/phy_trident.c
SRC_C += $(BEKEN_DIR)/beken378/driver/pwm/pwm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/pwm/pwm_new.c
SRC_C += $(BEKEN_DIR)/beken378/driver/pwm/mcu_ps_timer.c
SRC_C += $(BEKEN_DIR)/beken378/driver/pwm/bk_timer.c
SRC_C += $(BEKEN_DIR)/beken378/driver/rw_pub/rw_platf_pub.c
SRC_C += $(BEKEN_DIR)/beken378/driver/saradc/saradc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/spi/spi_bk7231n.c
SRC_C += $(BEKEN_DIR)/beken378/driver/spi/spi_master_bk7231n.c
SRC_C += $(BEKEN_DIR)/beken378/driver/spi/spi_slave_bk7231n.c
SRC_C += $(BEKEN_DIR)/beken378/driver/spidma/spidma.c
SRC_C += $(BEKEN_DIR)/beken378/driver/sys_ctrl/sys_ctrl.c
SRC_C += $(BEKEN_DIR)/beken378/driver/uart/Retarget.c
SRC_C += $(BEKEN_DIR)/beken378/driver/uart/uart_bk.c
SRC_C += $(BEKEN_DIR)/beken378/driver/uart/printf.c
SRC_C += $(BEKEN_DIR)/beken378/driver/wdt/wdt.c
#SRC_C += ./beken378/driver/ble/ble.c
#SRC_C += ./beken378/driver/ble/ble_pub/ip/ble/hl/src/prf/prf.c
#SRC_C += ./beken378/driver/ble/ble_pub/ip/ble/profiles/sdp/src/sdp_service.c
#SRC_C += ./beken378/driver/ble/ble_pub/ip/ble/profiles/sdp/src/sdp_service_task.c
#SRC_C += ./beken378/driver/ble/ble_pub/ip/ble/profiles/comm/src/comm.c
#SRC_C += ./beken378/driver/ble/ble_pub/ip/ble/profiles/comm/src/comm_task.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/app/src/app_ble.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/app/src/app_task.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/app/src/app_sdp.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/app/src/app_sec.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/app/src/app_comm.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/common/src/common_list.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/common/src/common_utils.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/common/src/RomCallFlash.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/dbg/src/dbg.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/dbg/src/dbg_mwsgen.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/dbg/src/dbg_swdiag.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/dbg/src/dbg_task.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/rwip/src/rwip.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/rf/src/ble_rf_xvr.c
#SRC_C += ./beken378/driver/ble/ble_pub/modules/ecc_p256/src/ecc_p256.c
#SRC_C += ./beken378/driver/ble/ble_pub/plf/refip/src/driver/uart/uart.c           

#zephyr source start
#ble controller source
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/co/ble_util_buf.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_chmap_upd.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_clk_acc.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_con_upd.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_cte.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_dbg.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_disconnect.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_dl_upd.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_encrypt.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_feat_exch.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_hci.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_le_ping.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_llcp.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_past.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_phy_upd.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_task.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llc/llc_ver_exch.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_adv.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_con.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_init.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_per_adv.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_scan.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_sync.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/lld/lld_test.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_adv.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_hci.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_init.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_scan.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_task.c
#SRC_C += ./beken378/driver/ble_5_x/ip/ble/ll/src/llm/llm_test.c
#SRC_C += ./beken378/driver/ble_5_x/ip/hci/src/hci.c
#SRC_C += ./beken378/driver/ble_5_x/ip/hci/src/hci_fc.c
#SRC_C += ./beken378/driver/ble_5_x/ip/hci/src/hci_msg.c
#SRC_C += ./beken378/driver/ble_5_x/ip/hci/src/hci_tl.c
#SRC_C += ./beken378/driver/ble_5_x/ip/sch/src/sch_alarm.c
#SRC_C += ./beken378/driver/ble_5_x/ip/sch/src/sch_arb.c
#SRC_C += ./beken378/driver/ble_5_x/ip/sch/src/sch_plan.c
#SRC_C += ./beken378/driver/ble_5_x/ip/sch/src/sch_prog.c
#SRC_C += ./beken378/driver/ble_5_x/ip/sch/src/sch_slice.c

#ble host source
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/att.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/conn.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/crypto.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/gatt.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/hci_core.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/hci_ecc.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/keys.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/l2cap.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/multi_adv.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/rpa.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/smp.c
#SRC_C += ./beken378/driver/ble_5_x/zephyr/host/src/uuid.c
#zephyr source end

ifeq (${CFG_BLE_5X_VERSION}, 1) && (${CFG_BLE_5X_RW}, 1)
####################################################
#rw head start
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/inc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/attc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/attm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/atts
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/import/reg
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/em/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/src
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/import
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/common/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ecc_p256/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/h4tl/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/prf
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/rwip/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/rwip/import/reg
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/nvds/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/config
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/reg
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/rf
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/uart
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/entry
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/arch/armv5
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/arch/armv5/ll
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/arch/armv5/compiler
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/profiles/comm/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/app/api
INCLUDES += -I$(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/ui
#rw head end

####################################################
ifeq ("${CFG_BLE_5X_USE_RWIP_LIB}", "0")
#rw source start
#ble lib
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_cte.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_hci.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_past.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_sig.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_smp.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapc/gapc_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_actv.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_addr.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_adv.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_cfg.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_init.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_list.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_per_sync.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_scan.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_smp.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gap/gapm/gapm_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/attc/attc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/attm/attm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/attm/attm_db.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/atts/atts.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc/gattc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc/gattc_rc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc/gattc_sdp.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc/gattc_svc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattc/gattc_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattm/gattm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/gatt/gattm/gattm_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_lecb.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_pdu.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_sig.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/l2c/l2cm/l2cm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/hl/src/rwble_hl/rwble_hl.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/co/ble_util_buf.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_chmap_upd.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_clk_acc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_con_upd.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_cte.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_dbg.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_disconnect.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_dl_upd.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_encrypt.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_feat_exch.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_hci.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_le_ping.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_llcp.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_past.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_phy_upd.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llc/llc_ver_exch.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_adv.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_con.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_init.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_per_adv.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_scan.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_sync.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/lld/lld_test.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_adv.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_hci.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_init.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_scan.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/ble/ll/src/llm/llm_test.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/src/hci.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/src/hci_fc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/src/hci_msg.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/hci/src/hci_tl.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/src/sch_alarm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/src/sch_arb.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/src/sch_plan.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/src/sch_prog.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/ip/sch/src/sch_slice.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_c1.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_ccm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_cmac.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_f4.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_f5.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_f6.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_g2.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_k1.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_k2.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_k3.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_k4.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_rpa.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/aes/src/ble_aes_s1.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/common/src/common_list.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/common/src/common_utils.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_iqgen.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_mwsgen.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_swdiag.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_trc.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_trc_mem.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/dbg/src/dbg_trc_tl.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ecc_p256/src/ecc_p256.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/h4tl/src/h4tl.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_event.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_mem.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_msg.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_queue.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/ke/src/kernel_timer.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_lib/modules/rwip/rwip_driver.c
####################################################
endif  ###ifeq ("${CFG_BLE_5X_USE_RWIP_LIB}", "0")

#ble pub
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/prf/prf.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/prf/prf_utils.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/profiles/comm/src/comm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/profiles/comm/src/comm_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/app/src/app_comm.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/app/src/app_ble.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/app/src/app_task.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/ui/ble_ui.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/rwip/src/rwip.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/rwip/src/rwble.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/entry/ble_main.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/rf/rf_xvr.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/uart/uart_ble.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/platform/7231n/driver/rf/ble_rf_port.c
SRC_C += $(BEKEN_DIR)/beken378/driver/ble_5_x_rw/ble_pub/app/src/app_ble_task.c

#rw source end
else  ###ifeq ("${CFG_BLE_5X_VERSION}", "1") && ("${CFG_BLE_5X_RW}", "1")


####################################################
endif ##ifeq ("${CFG_BLE_5X_VERSION}", "1") && ("${CFG_BLE_5X_RW}", "1")


#function layer
SRC_C += $(BEKEN_DIR)/beken378/func/func.c
SRC_C += $(BEKEN_DIR)/beken378/func/bk7011_cal/bk7231U_cal.c
SRC_C += $(BEKEN_DIR)/beken378/func/bk7011_cal/bk7231N_cal.c
SRC_C += $(BEKEN_DIR)/beken378/func/bk7011_cal/manual_cal_bk7231U.c
SRC_C += $(BEKEN_DIR)/beken378/func/joint_up/role_launch.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd_intf/hostapd_intf.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/bk_patch/ddrv.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/bk_patch/signal.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/bk_patch/sk_intf.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/bk_patch/fake_socket.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/hostapd/main_none.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/aes-internal.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/aes-internal-dec.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/aes-internal-enc.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/aes-unwrap.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/aes-wrap.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/bk_md5.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/md5-internal.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/rc4.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/bk_sha1.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/sha1-internal.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/sha1-pbkdf2.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/sha1-prf.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/crypto/tls_none.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ap_config.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ap_drv_ops.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ap_list.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ap_mlme.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/beacon.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/drv_callbacks.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/hostapd.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/hw_features.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ieee802_11_auth.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ieee802_11.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ieee802_11_ht.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ieee802_11_shared.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/ieee802_1x.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/sta_info.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/tkip_countermeasures.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/utils.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/wmm.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/wpa_auth.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/wpa_auth_glue.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/ap/wpa_auth_ie.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/common/hw_features_common.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/common/ieee802_11_common.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/common/wpa_common.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/drivers/driver_beken.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/drivers/driver_common.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/drivers/drivers.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/l2_packet/l2_packet_none.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/rsn_supp/wpa.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/rsn_supp/wpa_ie.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/utils/common.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/utils/eloop.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/utils/os_none.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/src/utils/wpabuf.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/blacklist.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/bss.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/config.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/config_none.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/events.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/main_supplicant.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/notify.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/wmm_ac.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/wpa_scan.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/wpas_glue.c
SRC_C += $(BEKEN_DIR)/beken378/func/hostapd-2.5/wpa_supplicant/wpa_supplicant.c

ifeq ($(CFG_WRAP_LIBC),1)
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdio/lib_libvscanf.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdio/lib_memsistream.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdio/lib_meminstream.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdio/lib_sscanf.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdio/lib_vsscanf.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdlib/lib_strtod.c
SRC_C += $(BEKEN_DIR)/beken378/func/libc/stdlib/lib_qsort.c
endif

SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/port/ethernetif.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/port/net.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/port/sys_arch.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/api_lib.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/api_msg.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/err.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/netbuf.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/netdb.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/netifapi.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/sockets.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/api/tcpip.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/def.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/dns.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/inet_chksum.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/init.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ip.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/autoip.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/dhcp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/etharp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/icmp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/igmp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/ip4_addr.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/ip4.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv4/ip4_frag.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/dhcp6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/ethip6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/icmp6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/inet6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/ip6_addr.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/ip6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/ip6_frag.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/mld6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/ipv6/nd6.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/mem.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/memp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/netif.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/pbuf.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/raw.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/stats.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/sys.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/tcp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/tcp_in.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/tcp_out.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/timeouts.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/core/udp.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/lwip-2.0.2/src/netif/ethernet.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/dhcpd/dhcp-server.c
SRC_C += $(BEKEN_DIR)/beken378/func/lwip_intf/dhcpd/dhcp-server-main.c
SRC_C += $(BEKEN_DIR)/beken378/func/misc/fake_clock.c
SRC_C += $(BEKEN_DIR)/beken378/func/misc/target_util.c
SRC_C += $(BEKEN_DIR)/beken378/func/misc/start_type.c
SRC_C += $(BEKEN_DIR)/beken378/func/power_save/power_save.c
SRC_C += $(BEKEN_DIR)/beken378/func/power_save/manual_ps.c
SRC_C += $(BEKEN_DIR)/beken378/func/power_save/mcu_ps.c
SRC_C += $(BEKEN_DIR)/beken378/func/rf_test/rx_sensitivity.c
SRC_C += $(BEKEN_DIR)/beken378/func/rf_test/tx_evm.c
SRC_C += $(BEKEN_DIR)/beken378/func/rwnx_intf/rw_ieee80211.c
SRC_C += $(BEKEN_DIR)/beken378/func/rwnx_intf/rw_msdu.c
SRC_C += $(BEKEN_DIR)/beken378/func/rwnx_intf/rw_msg_rx.c
SRC_C += $(BEKEN_DIR)/beken378/func/rwnx_intf/rw_msg_tx.c
SRC_C += $(BEKEN_DIR)/beken378/func/sim_uart/gpio_uart.c
SRC_C += $(BEKEN_DIR)/beken378/func/sim_uart/pwm_uart.c
SRC_C += $(BEKEN_DIR)/beken378/func/spidma_intf/spidma_intf.c
SRC_C += $(BEKEN_DIR)/beken378/func/temp_detect/temp_detect.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/cmd_evm.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/cmd_help.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/cmd_reg.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/cmd_rx_sensitivity.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/command_line.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/command_table.c
SRC_C += $(BEKEN_DIR)/beken378/func/uart_debug/udebug.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverFlash.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverRng.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverGpio.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverPwm.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverUart.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverWdg.c
SRC_C += $(BEKEN_DIR)/beken378/func/user_driver/BkDriverTimer.c
SRC_C += $(BEKEN_DIR)/beken378/func/wlan_ui/wlan_cli.c
SRC_C += $(BEKEN_DIR)/beken378/func/wlan_ui/wlan_ui.c
SRC_C += $(BEKEN_DIR)/beken378/func/bk_tuya_pwm/bk_tuya_pwm.c
SRC_C += $(BEKEN_DIR)/beken378/func/net_param_intf/net_param.c

#rwnx ip module
#SRC_C += ./beken378/ip/common/co_dlist.c
#SRC_C += ./beken378/ip/common/co_list.c
#SRC_C += ./beken378/ip/common/co_math.c
#SRC_C += ./beken378/ip/common/co_pool.c
#SRC_C += ./beken378/ip/common/co_ring.c
#SRC_C += ./beken378/ip/ke/ke_env.c
#SRC_C += ./beken378/ip/ke/ke_event.c
#SRC_C += ./beken378/ip/ke/ke_msg.c
#SRC_C += ./beken378/ip/ke/ke_queue.c
#SRC_C += ./beken378/ip/ke/ke_task.c
#SRC_C += ./beken378/ip/ke/ke_timer.c
#SRC_C += ./beken378/ip/lmac/src/chan/chan.c
#SRC_C += ./beken378/ip/lmac/src/hal/hal_desc.c
#SRC_C += ./beken378/ip/lmac/src/hal/hal_dma.c
#SRC_C += ./beken378/ip/lmac/src/hal/hal_machw.c
#SRC_C += ./beken378/ip/lmac/src/hal/hal_mib.c
#SRC_C += ./beken378/ip/lmac/src/mm/mm.c
#SRC_C += ./beken378/ip/lmac/src/mm/mm_bcn.c
#SRC_C += ./beken378/ip/lmac/src/mm/mm_task.c
#SRC_C += ./beken378/ip/lmac/src/mm/mm_timer.c
#SRC_C += ./beken378/ip/lmac/src/p2p/p2p.c
#SRC_C += ./beken378/ip/lmac/src/ps/ps.c
#SRC_C += ./beken378/ip/lmac/src/rd/rd.c
#SRC_C += ./beken378/ip/lmac/src/rwnx/rwnx.c
#SRC_C += ./beken378/ip/lmac/src/rx/rx_swdesc.c
#SRC_C += ./beken378/ip/lmac/src/rx/rxl/rxl_cntrl.c
#SRC_C += ./beken378/ip/lmac/src/rx/rxl/rxl_hwdesc.c
#SRC_C += ./beken378/ip/lmac/src/scan/scan.c
#SRC_C += ./beken378/ip/lmac/src/scan/scan_shared.c
#SRC_C += ./beken378/ip/lmac/src/scan/scan_task.c
#SRC_C += ./beken378/ip/lmac/src/sta/sta_mgmt.c
#SRC_C += ./beken378/ip/lmac/src/td/td.c
#SRC_C += ./beken378/ip/lmac/src/tdls/tdls.c
#SRC_C += ./beken378/ip/lmac/src/tdls/tdls_task.c
#SRC_C += ./beken378/ip/lmac/src/tpc/tpc.c
#SRC_C += ./beken378/ip/lmac/src/tx/tx_swdesc.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_buffer.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_buffer_shared.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_cfm.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_cntrl.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_frame.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_frame_shared.c
#SRC_C += ./beken378/ip/lmac/src/tx/txl/txl_hwdesc.c
#SRC_C += ./beken378/ip/lmac/src/vif/vif_mgmt.c
#SRC_C += ./beken378/ip/mac/mac.c
#SRC_C += ./beken378/ip/mac/mac_ie.c
#SRC_C += ./beken378/ip/umac/src/apm/apm.c
#SRC_C += ./beken378/ip/umac/src/apm/apm_task.c
#SRC_C += ./beken378/ip/umac/src/bam/bam.c
#SRC_C += ./beken378/ip/umac/src/bam/bam_task.c
#SRC_C += ./beken378/ip/umac/src/me/me.c
#SRC_C += ./beken378/ip/umac/src/me/me_mgmtframe.c
#SRC_C += ./beken378/ip/umac/src/me/me_mic.c
#SRC_C += ./beken378/ip/umac/src/me/me_task.c
#SRC_C += ./beken378/ip/umac/src/me/me_utils.c
#SRC_C += ./beken378/ip/umac/src/rc/rc.c
#SRC_C += ./beken378/ip/umac/src/rc/rc_basic.c
#SRC_C += ./beken378/ip/umac/src/rxu/rxu_cntrl.c
#SRC_C += ./beken378/ip/umac/src/scanu/scanu.c
#SRC_C += ./beken378/ip/umac/src/scanu/scanu_shared.c
#SRC_C += ./beken378/ip/umac/src/scanu/scanu_task.c
#SRC_C += ./beken378/ip/umac/src/sm/sm.c
#SRC_C += ./beken378/ip/umac/src/sm/sm_task.c
#SRC_C += ./beken378/ip/umac/src/txu/txu_cntrl.c 

#ble lib
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ahi/src/ahi.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ahi/src/ahi_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapc/gapc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapc/gapc_hci.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapc/gapc_sig.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapc/gapc_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapm/gapm.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapm/gapm_hci.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapm/gapm_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/gapm/gapm_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpc/smpc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpc/smpc_api.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpc/smpc_crypto.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpc/smpc_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gap/smpm/smpm_api.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/attc/attc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/attm/attm.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/attm/attm_db.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/atts/atts.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattc/gattc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattc/gattc_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattm/gattm.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/gatt/gattm/gattm_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_lecb.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_pdu.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_sig.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cc/l2cc_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/l2c/l2cm/l2cm.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/prf/prf_utils.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/hl/src/rwble_hl/rwble_hl.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/em/em_buf.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc_ch_asses.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc_hci.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc_llcp.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llc/llc_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld_evt.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld_pdu.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld_sleep.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/lld/lld_wlcoex.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llm/llm.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llm/llm_hci.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llm/llm_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/llm/llm_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ble/ll/src/rwble/rwble.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/ea/src/ea.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/hci/src/hci.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/hci/src/hci_fc.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/hci/src/hci_msg.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/hci/src/hci_tl.c 
#SRC_C += ./beken378/driver/ble/ble_lib/ip/hci/src/hci_util.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/h4tl/src/h4tl.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_event.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_mem.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_msg.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_queue.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_task.c 
#SRC_C += ./beken378/driver/ble/ble_lib/modules/ke/src/kernel_timer.c 
#SRC_C += ./beken378/driver/ble/ble_lib/plf/refip/src/arch/main/ble_arch_main.c


#operation system module
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/croutine.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/event_groups.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/list.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es/port.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/MemMang/heap_4.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/queue.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/tasks.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/timers.c
SRC_OS += $(BEKEN_DIR)/beken378/os/FreeRTOSv9.0.0/rtos_pub.c



# Application
INCLUDES += -Iapps/$(APP_NAME)

#assembling files
SRC_S = 
SRC_S +=  $(BEKEN_DIR)/beken378/driver/entry/boot_handlers.S
SRC_S +=  $(BEKEN_DIR)/beken378/driver/entry/boot_vectors.S

# Generate obj list
# -------------------------------------------------------------------
OBJ_C_LIST = $(SRC_C:%.c=$(OBJ_DIR)/%.o)
DEPENDENCY_LIST = $(SRC_C:%.c=$(OBJ_DIR)/%.d)

OBJ_S_LIST = $(SRC_S:%.S=$(OBJ_DIR)/%.o)
DEPENDENCY_S_LIST = $(SRC_S:%.S=$(OBJ_DIR)/%.d)

OBJ_OS_LIST = $(SRC_OS:%.c=$(OBJ_DIR)/%.o)
DEPENDENCY_OS_LIST = $(SRC_OS:%.c=$(OBJ_DIR)/%.d)

# Compile options
# -------------------------------------------------------------------
CFLAGS += -g -mthumb -mcpu=arm968e-s -march=armv5te -mthumb-interwork -mlittle-endian -Os -std=c99 -ffunction-sections -fdata-sections -nostdlib -fsigned-char -Wno-format -Wno-unknown-pragmas -fno-strict-aliasing
#CFLAGS += -g -mthumb -mcpu=arm968e-s -march=armv5te -mthumb-interwork -mlittle-endian -Os -std=c99 -ffunction-sections -Wall -fdata-sections -nostdlib -fsigned-char -Werror -Wno-format -Wno-unknown-pragmas -fno-strict-aliasing
#CFLAGS += -g -mthumb -mcpu=arm968e-s -march=armv5te -mthumb-interwork -mlittle-endian -Os -std=c99 -ffunction-sections -Wall -fdata-sections -nostdlib -fsigned-char -Wno-unused-function -Wunknown-pragmas -Wl,--gc-sections

ifeq ("${CFG_GIT_VERSION}", "")
else
CFLAGS += -DVCS_RELEASE_VERSION=\"$(CFG_GIT_VERSION)\"
endif

#debug defines
CFLAGS += $(DEBUG_DEFINES)

OSFLAGS =
OSFLAGS += -g -marm -mcpu=arm968e-s -march=armv5te -mthumb-interwork -mlittle-endian -Os -std=c99 -fsigned-char -Wunknown-pragmas -ffunction-sections -fdata-sections
#OSFLAGS += -g -mthumb -mcpu=arm968e-s -march=armv5te -mthumb-interwork -mlittle-endian -Os -std=c99 -ffunction-sections -Wall -fsigned-char -fdata-sections -Wunknown-pragmas -Wl,--gc-sections

ASMFLAGS = 
ASMFLAGS += -g -marm -mthumb-interwork -mcpu=arm968e-s -march=armv5te -x assembler-with-cpp

LFLAGS = 
LFLAGS += -g -Wl,--gc-sections -marm -mcpu=arm968e-s -mthumb-interwork -nostdlib
LFLAGS += -Wl,-wrap,malloc -Wl,-wrap,_malloc_r -Wl,-wrap,free -Wl,-wrap,_free_r -Wl,-wrap,zalloc -Wl,-wrap,calloc -Wl,-wrap,realloc  -Wl,-wrap,_realloc_r
LFLAGS += -Wl,-wrap,printf -Wl,-wrap,vsnprintf -Wl,-wrap,snprintf -Wl,-wrap,sprintf -Wl,-wrap,puts
#LFLAGS += -g -Wl,--gc-sections -mthumb -mcpu=arm968e-s -mthumb-interwork -nostdlib


LIBFLAGS =
LIBFLAGS += -L$(BEKEN_DIR)/beken378/lib -lrwnx
ifeq ("${CFG_BLE_5X_USE_RWIP_LIB}", "1")
LIBFLAGS += -L$(BEKEN_DIR)/beken378/lib/ -lble
endif

# stdlib wrapper
ifeq ($(CFG_WRAP_LIBC),1)
LFLAGS += -Wl,-wrap,strtod -Wl,-wrap,qsort
LFLAGS += -Wl,-wrap,sscanf
endif



# Compile
# first target is default
.PHONY: all
all: application 

.PHONY: application
application: prerequirement $(OBJ_C_LIST) $(OBJ_S_LIST) $(OBJ_OS_LIST)
	@echo LD $(TARGET_NAME).elf
	@$(LD) $(LFLAGS) -o $(BIN_DIR)/$(TARGET_NAME).elf $(OBJ_C_LIST) $(OBJ_S_LIST) $(OBJ_OS_LIST) $(LIBFLAGS) -T$(SOC_NAME_LDS) -Xlinker -Map=$(BIN_DIR)/$(TARGET_NAME).map
	@$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET_NAME).elf $(BIN_DIR)/$(TARGET_NAME).bin
	@echo "CRC $(BIN_DIR)/$(TARGET_NAME).bin"
	@$(ENCRYPT) $(BIN_DIR)/$(TARGET_NAME).bin $(BIN_DIR)/$(TARGET_NAME)_enc.bin 0 $(ENCRYPT_ARGS)
#	@$(LD) $(LFLAGS) -o $(BIN_DIR)/$(TARGET_NAME).elf $(OBJ_C_LIST) $(OBJ_S_LIST) $(OBJ_OS_LIST) $(LIBFLAGS) -T$(SOC_NAME_BSP_LDS) -Xlinker -Map=$(BIN_DIR)/$(TARGET_NAME)_bsp.map
#	@$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET_NAME).elf $(BIN_DIR)/$(TARGET_NAME)_bsp.bin
	@cd $(BEKEN_DIR)/tools/generate/package_tool/windows; python beken_packager_wrapper_win.py -c $(SOC_NAME) --firmware ../../../../../../../$(BIN_DIR)/$(TARGET_NAME)_enc.bin --out_dir ../../../$(BIN_DIR)
#	Бинарник для загрузки по адресу 0x00
	@mv $(BEKEN_DIR)/tools/generate/package_tool/windows/all_2M.1220.bin $(BIN_DIR)/$(TARGET_NAME)_all_2M.1220.bin
#	Бинарник для загрузки по адресу 0x00011000	
	@mv $(BEKEN_DIR)/tools/generate/package_tool/windows/$(TARGET_NAME)_enc_uart_2M.1220.bin $(BIN_DIR)

# Generate build info
# -------------------------------------------------------------------	
.PHONY: prerequirement
prerequirement:
	@echo ===========================================================
	@echo Build 
	@echo APP_NAME = $(APP_NAME)
	@echo APP_VERSION = $(APP_VERSION)
	@echo SOC_NAME = $(SOC_NAME)
	@echo ENCRYPT_ARGS = $(ENCRYPT_ARGS)
	@echo SOC_NAME_LDS = $(SOC_NAME_LDS)
	@echo ===========================================================


#.PHONY: application
#application: prerequirement $(SRC_O) $(SRC_S_O) $(SRC_OS_O)
#	@echo linking $(OUTPUT_NAME).axf
#	@$(LD) $(LFLAGS) -o $(OUTPUT_NAME).axf $(SRC_O) $(SRC_S_O) $(SRC_OS_O) $(LIBFLAGS) -T$(BEKEN_DIR)/beken378/build/bk7231_ota.ld
#	@echo making $(OUTPUT_NAME).sym
#	@$(NM) $(OUTPUT_NAME).axf | sort > $(OUTPUT_NAME).sym
#	@echo making $(OUTPUT_NAME).asm
#	@$(OBJDUMP) -d $(OUTPUT_NAME).axf > $(OUTPUT_NAME).asm
#	@echo making $(OUTPUT_NAME).bin
#	@$(OBJCOPY) -O binary $(OUTPUT_NAME).axf $(OUTPUT_NAME).bin
#	
#	@echo
#	@echo making otafix DOES NOTHING?
#	@cp $(OUTPUT_NAME).bin $(TOOLS_DIR)
#	@cd $(TOOLS_DIR); package_tool/windows/otafix.exe $(TARGET_NAME).bin 
#
#	@echo
#	@echo making $(TARGET)_enc.bin
#	@echo "*************************************************************************"
##	Key 510fb093 a3cbeadc 5993a17e c7adeb03 must be used for BK7231T, BK7231N Other keys don't work
#	@cd $(TOOLS_DIR); package_tool/windows/encrypt.exe $(TARGET_NAME).bin 510fb093 a3cbeadc 5993a17e c7adeb03 10000
##	No encryption
##	@cd $(TOOLS_DIR); package_tool/windows/encrypt.exe $(TARGET_NAME).bin 0 0 0 0 10000
#	@cp $(TOOLS_DIR)/$(TARGET_NAME)_enc.bin $(BIN_DIR)
#	@mv $(TOOLS_DIR)/$(TARGET_NAME).cpr $(BIN_DIR)
#	@mv $(TOOLS_DIR)/$(TARGET_NAME).out $(BIN_DIR)
#
#	@echo
#	@echo making config.json
#	@echo "*************************************************************************"
#	@cd $(TOOLS_DIR); python mpytools.py $(TARGET)_enc.bin
#
#	@echo
#	@echo packing
#	@echo "*************************************************************************"
#	@cd $(TOOLS_DIR); package_tool/windows/beken_packager.exe config.json
#
#	@echo
#	@echo making $(APP_NAME)_QIO_${APP_VERSION}.bin - bootloader + user app - loaded via flash programmer
#	@echo "*************************************************************************"
#	@mv $(TOOLS_DIR)/all_1.00.bin $(BIN_DIR)/$(APP_NAME)_QIO_${APP_VERSION}.bin
#
#	@echo
#	@echo making ${APP_NAME}_UA_${APP_VERSION}.bin - user app - loaded via uart
#	@echo "*************************************************************************"
#	@mv $(TOOLS_DIR)/$(TARGET_NAME)_enc_uart_1.00.bin $(BIN_DIR)/${APP_NAME}_UA_${APP_VERSION}.bin
#	
#	@rm -f $(TOOLS_DIR)/$(TARGET_NAME)_enc.bin
#	@rm -f $(TOOLS_DIR)/$(TARGET_NAME).bin
#	@rm -f $(TOOLS_DIR)/config.json
#	
#	@echo	
#	@echo making ${APP_NAME}_UG_${APP_VERSION}.bin - OTA binary
#	@echo "*************************************************************************"	
#	@$(TOOLS_DIR)/package_tool/windows/rt_ota_packaging_tool_cli.exe -f $(OUTPUT_NAME).bin -v $(CURRENT_TIME) -o $(OUTPUT_NAME).rbl -p app -c gzip -s aes -k 0123456789ABCDEF0123456789ABCDEF -i 0123456789ABCDEF
#	@$(TOOLS_DIR)/package_tool/windows/package.exe $(OUTPUT_NAME).rbl $(BIN_DIR)/${APP_NAME}_UG_${APP_VERSION}.bin $(APP_VERSION)
#
#	@echo CHECK OTA FILE SIZE IS SMALLER THAN 679936
#	@test `wc -c <$(BIN_DIR)/$(APP_NAME)_UG_$(APP_VERSION).bin` -lt 679936
#	
#	@echo	
#	@echo "*************************************************************************"
#	@echo "                      ${APP_NAME}_$(APP_VERSION).bin"
#	@echo "*************************************************************************"
#	@echo "**********************COMPILE SUCCESS************************************"
#	@echo "*************************************************************************"

$(OBJ_C_LIST): $(OBJ_DIR)/%.o : %.c
	@echo CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(patsubst %.o,%.d,$@)
	@chmod 777 $@

$(OBJ_S_LIST): $(OBJ_DIR)/%.o : %.S
	@echo AS $<
	@mkdir -p $(dir $@)
	@$(CC) $(ASMFLAGS) $(INCLUDES) -c $< -o $@
	@$(CC) $(ASMFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(patsubst %.o,%.d,$@)
	@chmod 777 $@

$(OBJ_OS_LIST): $(OBJ_DIR)/%.o : %.c
	@echo CC_OS $<
	@mkdir -p $(dir $@)
	@$(CC) $(OSFLAGS) $(INCLUDES) -c $< -o $@
	@$(CC) $(OSFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(patsubst %.o,%.d,$@)
	@chmod 777 $@

-include $(DEPENDENCY_LIST)
-include $(DEPENDENCY_S_LIST)
-include $(DEPENDENCY_OS_LIST)

# -------------------------------------------------------------------	
# Generate build info
# -------------------------------------------------------------------	
.PHONY: setup
setup:
	@echo "----------------"
	@echo Setup $(GDB_SERVER)
	@echo "----------------"
	
.PHONY: debug
debug:
	@if [ ! -f $(FLASH_TOOLDIR)/rtl_gdb_debug.txt ] ; then echo Please do \"make setup GDB_SERVER=[jlink or openocd]\" first; echo && false ; fi
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
	$(FLASH_TOOLDIR)/Check_Jtag.sh
	cmd /c start $(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_debug.txt 
else
	$(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_debug.txt
endif

.PHONY: clean
clean:
	@echo cleaning $(BUILD_DIR)/$(TARGET_NAME)
	rm -rf $(BUILD_DIR)/$(TARGET_NAME)
