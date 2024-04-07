/*
 * Celestica Redstone-DP
 *
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_REDSTONE_DP_H__
#define CEL_REDSTONE_DP_H__

#define CPLD_IO_BASE 0xA100
#define CPLD_IO_SIZE 0x02ff

enum {
	I2C_I801_BUS = 0,
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,
	I2C_CPLD_MUX_1 = 20,
	I2C_CPLD_MUX_2,
	I2C_CPLD_MUX_3,
	I2C_CPLD_MUX_4,
	I2C_CPLD_MUX_FIRST_PORT = 30,
};

#define CEL_REDSTONE_DP_NUM_PORTS  (54)

#define CPLD_I2C_CLK_50KHZ  (0x00)
#define CPLD_I2C_CLK_100KHZ (0x40)

struct cpld_bus_data {
	int io_base;
	uint32_t clock;
	uint32_t timeout;
	int bus;
};

struct cpld_mux_data {
	struct i2c_adapter *parent_adapter;
	int mux_base_port_num;
	int mux_num_ports;
	int mux_base_id;
	int mux_ports_base_bus;
};

/*-----------------------------------------------------------------------------
 *
 * Register info from:
 * R1055-M0011-01 Rev03 Redstone-DP HW Spec_0616.pdf
 *
 */

/* CPLD */

#define CPLD_I2C_SELECT_REG                  (0xA108)
#define   CPLD_I2C_SEL_MASK                  (0x01)
#define   CPLD_BMC_SEL                       (0x01)
#define   CPLD_CPU_SEL                       (0x00)

/*-----------------------------------------------------------------------------
 *
 * Register info from:
 * R1055-M0016-01_Rev0.4_Redstone-DP_CPLD_Design_Specification_20170112.pdf
 *
 */

/* CPLD1 */

#define CPLD1_VERSION_REG                    (0xA300)
#define   CPLD_ID_MASK                       (0xF0)
#define   CPLD_ID_SHIFT                      (4)
#define   CPLD_VERSION_MASK                  (0x0F)

#define CPLD1_SOFTWARE_SCRATCH_REG           (0xA301)

#define CPLD1_BOARD_REVISION_REG             (0xA302)
#define   CPLD1_BOARD_REVISION_MASK          (0x07)

#define CPLD1_RESET_CONTROL_1_REG            (0xA303)
#define   CPLD1_PCA9548A_U36_RESET_MASK      (0x80)
#define   CPLD1_PCA9548A_U1_RESET_MASK       (0x40)
#define   CPLD1_I210_RESET_MASK              (0x20)
#define   CPLD1_DPLL_RESET_MASK              (0x10)
#define   CPLD1_BCM88375_RESET_MASK          (0x08)
#define   CPLD1_CPLD4_RESET_MASK             (0x04)
#define   CPLD1_CPLD3_RESET_MASK             (0x02)
#define   CPLD1_CPLD2_RESET_MASK             (0x01)

#define CPLD1_RESET_CONTROL_2_REG            (0xA304)
#define   CPLD1_BCM52311_RESET_MASK          (0x10)
#define   CPLD1_BCM52311_PCIE_RESET_MASK     (0x08)
#define   CPLD1_BCM52311_CORE_RESET_MASK     (0x04)
#define   CPLD1_UCD90120_RESET_MASK          (0x02)
#define   CPLD1_ZL30253_RESET_MASK           (0x01)

#define CPLD1_I2C_PORT_ID_REG                (0xA310)
#define   CPLD1_I2C_BAUD_RATE_MASK           (0x40)
#define   CPLD1_I2C_PORT_ID_MASK             (0x3F)

#define CPLD1_I2C_OP_CODE_REG                (0xA311)
#define   CPLD1_I2C_DATA_BYTE_LEN_MASK       (0xF0)
#define   CPLD1_I2C_DATA_BYTE_LEN_SHIFT      (4)
#define   CPLD1_I2C_CMD_BYTE_LEN_MASK        (0x03)

#define CPLD1_I2C_DEVICE_ADDRESS_REG         (0xA312)
#define   CPLD1_I2C_SLAVE_ADDRESS_MASK       (0xFE)
#define   CPLD1_I2C_SLAVE_ADDRESS_SHIFT      (1)
#define   CPLD1_I2C_READ_WRITE_MASK          (0x01)

#define CPLD1_I2C_COMMAND_BYTE0_REG          (0xA313)
#define CPLD1_I2C_COMMAND_BYTE1_REG          (0xA314)
#define CPLD1_I2C_COMMAND_BYTE2_REG          (0xA315)

#define CPLD1_I2C_STATUS_REG                 (0xA316)
#define   CPLD1_I2C_MASTER_ERROR_MASK        (0x80)
#define   CPLD1_I2C_BUSY_INDICATOR_MASK      (0x40)
#define   CPLD1_I2C_MASTER_RESET_MASK        (0x01)

#define CPLD1_I2C_WRITE_DATA_BYTE0_REG       (0xA320)
#define CPLD1_I2C_WRITE_DATA_BYTE1_REG       (0xA321)
#define CPLD1_I2C_WRITE_DATA_BYTE2_REG       (0xA322)
#define CPLD1_I2C_WRITE_DATA_BYTE3_REG       (0xA323)
#define CPLD1_I2C_WRITE_DATA_BYTE4_REG       (0xA324)
#define CPLD1_I2C_WRITE_DATA_BYTE5_REG       (0xA325)
#define CPLD1_I2C_WRITE_DATA_BYTE6_REG       (0xA326)
#define CPLD1_I2C_WRITE_DATA_BYTE7_REG       (0xA327)

#define CPLD1_I2C_READ_DATA_BYTE0_REG        (0xA330)
#define CPLD1_I2C_READ_DATA_BYTE1_REG        (0xA331)
#define CPLD1_I2C_READ_DATA_BYTE2_REG        (0xA332)
#define CPLD1_I2C_READ_DATA_BYTE3_REG        (0xA333)
#define CPLD1_I2C_READ_DATA_BYTE4_REG        (0xA334)
#define CPLD1_I2C_READ_DATA_BYTE5_REG        (0xA335)
#define CPLD1_I2C_READ_DATA_BYTE6_REG        (0xA336)
#define CPLD1_I2C_READ_DATA_BYTE7_REG        (0xA337)

#define CPLD1_QSFP_1_6_RESET_REG             (0xA360)

#define CPLD1_QSFP_1_6_LPMOD_REG             (0xA361)

#define CPLD1_QSFP_1_6_ABS_REG               (0xA362)

#define CPLD1_QSFP_1_6_INT_N_REG             (0xA363)

#define CPLD1_QSFP_1_6_I2C_READY_REG         (0xA364)

#define CPLD1_QSFP_1_6_PG_REG                (0xA365)

#define CPLD1_SWITCH_BOARD_TYPE_REG          (0xA370)
#define   CPLD1_SWITCH_BOARD_TYPE_MASK       (0xFF)
#define   CPLD1_REDSTONE_DP_BOARD            (0x06)
#define   CPLD1_MIDSTONE_BOARD               (0x05)
#define   CPLD1_QUESTONE_BOARD               (0x04)
#define   CPLD1_SEASTONE_BOARD               (0x03)
#define   CPLD1_SMALLSTONE_XP_BOARD          (0x02)
#define   CPLD1_REDSTONE_XP_BOARD            (0x01)

/* CPLD2 */

#define CPLD2_VERSION_REG                    (0xA200)

#define CPLD2_SOFTWARE_SCRATCH_REG           (0xA201)

#define CPLD2_I2C_PORT_ID_REG                (0xA210)
#define   CPLD2_I2C_BAUD_RATE_MASK           (0x40)
#define   CPLD2_I2C_PORT_ID_MASK             (0x3F)

#define CPLD2_I2C_OP_CODE_REG                (0xA211)
#define   CPLD2_I2C_DATA_BYTE_LEN_MASK       (0xF0)
#define   CPLD2_I2C_DATA_BYTE_LEN_SHIFT      (4)
#define   CPLD2_I2C_CMD_BYTE_LEN_MASK        (0x03)

#define CPLD2_I2C_DEVICE_ADDRESS_REG         (0xA212)
#define   CPLD2_I2C_SLAVE_ADDRESS_MASK       (0xFE)
#define   CPLD2_I2C_SLAVE_ADDRESS_SHIFT      (1)
#define   CPLD2_I2C_READ_WRITE_MASK          (0x01)

#define CPLD2_I2C_COMMAND_BYTE0_REG          (0xA213)
#define CPLD2_I2C_COMMAND_BYTE1_REG          (0xA214)
#define CPLD2_I2C_COMMAND_BYTE2_REG          (0xA215)

#define CPLD2_I2C_STATUS_REG                 (0xA216)
#define   CPLD2_I2C_MASTER_ERROR_MASK        (0x80)
#define   CPLD2_I2C_BUSY_INDICATOR_MASK      (0x40)
#define   CPLD2_I2C_MASTER_RESET_MASK        (0x01)

#define CPLD2_I2C_WRITE_DATA_BYTE0_REG       (0xA220)
#define CPLD2_I2C_WRITE_DATA_BYTE1_REG       (0xA221)
#define CPLD2_I2C_WRITE_DATA_BYTE2_REG       (0xA222)
#define CPLD2_I2C_WRITE_DATA_BYTE3_REG       (0xA223)
#define CPLD2_I2C_WRITE_DATA_BYTE4_REG       (0xA224)
#define CPLD2_I2C_WRITE_DATA_BYTE5_REG       (0xA225)
#define CPLD2_I2C_WRITE_DATA_BYTE6_REG       (0xA226)
#define CPLD2_I2C_WRITE_DATA_BYTE7_REG       (0xA227)

#define CPLD2_I2C_READ_DATA_BYTE0_REG        (0xA230)
#define CPLD2_I2C_READ_DATA_BYTE1_REG        (0xA231)
#define CPLD2_I2C_READ_DATA_BYTE2_REG        (0xA232)
#define CPLD2_I2C_READ_DATA_BYTE3_REG        (0xA233)
#define CPLD2_I2C_READ_DATA_BYTE4_REG        (0xA234)
#define CPLD2_I2C_READ_DATA_BYTE5_REG        (0xA235)
#define CPLD2_I2C_READ_DATA_BYTE6_REG        (0xA236)
#define CPLD2_I2C_READ_DATA_BYTE7_REG        (0xA237)

#define CPLD2_SFP_1_8_RX_LOS_REG             (0xA240)
#define CPLD2_SFP_9_16_RX_LOS_REG            (0xA241)
#define CPLD2_SFP_17_18_RX_LOS_REG           (0xA242)

#define CPLD2_SFP_1_8_TX_DISABLE_REG         (0xA250)
#define CPLD2_SFP_9_16_TX_DISABLE_REG        (0xA251)
#define CPLD2_SFP_17_18_TX_DISABLE_REG       (0xA252)

#define CPLD2_SFP_1_8_RS_REG                 (0xA253)
#define CPLD2_SFP_9_16_RS_REG                (0xA254)
#define CPLD2_SFP_17_18_RS_REG               (0xA255)

#define CPLD2_SFP_1_8_TX_FAULT_REG           (0xA256)
#define CPLD2_SFP_9_16_TX_FAULT_REG          (0xA257)
#define CPLD2_SFP_17_18_TX_FAULT_REG         (0xA258)

#define CPLD2_SFP_1_8_MOD_ABS_REG            (0xA259)
#define CPLD2_SFP_9_16_MOD_ABS_REG           (0xA25A)
#define CPLD2_SFP_17_18_MOD_ABS_REG          (0xA25B)

/* CPLD3 */

#define CPLD3_VERSION_REG                    (0xA280)

#define CPLD3_SOFTWARE_SCRATCH_REG           (0xA281)

#define CPLD3_I2C_PORT_ID_REG                (0xA290)
#define   CPLD3_I2C_BAUD_RATE_MASK           (0x40)
#define   CPLD3_I2C_PORT_ID_MASK             (0x3F)

#define CPLD3_I2C_OP_CODE_REG                (0xA291)
#define   CPLD3_I2C_DATA_BYTE_LEN_MASK       (0xF0)
#define   CPLD3_I2C_DATA_BYTE_LEN_SHIFT      (4)
#define   CPLD3_I2C_CMD_BYTE_LEN_MASK        (0x03)

#define CPLD3_I2C_DEVICE_ADDRESS_REG         (0xA292)
#define   CPLD3_I2C_SLAVE_ADDRESS_MASK       (0xFE)
#define   CPLD3_I2C_SLAVE_ADDRESS_SHIFT      (1)
#define   CPLD3_I2C_READ_WRITE_MASK          (0x01)

#define CPLD3_I2C_COMMAND_BYTE0_REG          (0xA293)
#define CPLD3_I2C_COMMAND_BYTE1_REG          (0xA294)
#define CPLD3_I2C_COMMAND_BYTE2_REG          (0xA295)

#define CPLD3_I2C_STATUS_REG                 (0xA296)
#define   CPLD3_I2C_MASTER_ERROR_MASK        (0x80)
#define   CPLD3_I2C_BUSY_INDICATOR_MASK      (0x40)
#define   CPLD3_I2C_MASTER_RESET_MASK        (0x01)

#define CPLD3_I2C_WRITE_DATA_BYTE0_REG       (0xA2A0)
#define CPLD3_I2C_WRITE_DATA_BYTE1_REG       (0xA2A1)
#define CPLD3_I2C_WRITE_DATA_BYTE2_REG       (0xA2A2)
#define CPLD3_I2C_WRITE_DATA_BYTE3_REG       (0xA2A3)
#define CPLD3_I2C_WRITE_DATA_BYTE4_REG       (0xA2A4)
#define CPLD3_I2C_WRITE_DATA_BYTE5_REG       (0xA2A5)
#define CPLD3_I2C_WRITE_DATA_BYTE6_REG       (0xA2A6)
#define CPLD3_I2C_WRITE_DATA_BYTE7_REG       (0xA2A7)

#define CPLD3_I2C_READ_DATA_BYTE0_REG        (0xA2B0)
#define CPLD3_I2C_READ_DATA_BYTE1_REG        (0xA2B1)
#define CPLD3_I2C_READ_DATA_BYTE2_REG        (0xA2B2)
#define CPLD3_I2C_READ_DATA_BYTE3_REG        (0xA2B3)
#define CPLD3_I2C_READ_DATA_BYTE4_REG        (0xA2B4)
#define CPLD3_I2C_READ_DATA_BYTE5_REG        (0xA2B5)
#define CPLD3_I2C_READ_DATA_BYTE6_REG        (0xA2B6)
#define CPLD3_I2C_READ_DATA_BYTE7_REG        (0xA2B7)

#define CPLD3_SFP_19_26_RX_LOS_REG           (0xA2C0)
#define CPLD3_SFP_27_34_RX_LOS_REG           (0xA2C1)
#define CPLD3_SFP_35_36_RX_LOS_REG           (0xA2C2)

#define CPLD3_SFP_19_26_TX_DISABLE_REG       (0xA2D0)
#define CPLD3_SFP_27_34_TX_DISABLE_REG       (0xA2D1)
#define CPLD3_SFP_35_36_TX_DISABLE_REG       (0xA2D2)

#define CPLD3_SFP_19_26_RS_REG               (0xA2D3)
#define CPLD3_SFP_27_34_RS_REG               (0xA2D4)
#define CPLD3_SFP_35_36_RS_REG               (0xA2D5)

#define CPLD3_SFP_19_26_TX_FAULT_REG         (0xA2D6)
#define CPLD3_SFP_27_34_TX_FAULT_REG         (0xA2D7)
#define CPLD3_SFP_35_36_TX_FAULT_REG         (0xA2D8)

#define CPLD3_SFP_19_26_MOD_ABS_REG          (0xA2D9)
#define CPLD3_SFP_27_34_MOD_ABS_REG          (0xA2DA)
#define CPLD3_SFP_35_36_MOD_ABS_REG          (0xA2DB)

/* CPLD4 */

#define CPLD4_VERSION_REG                    (0xA380)

#define CPLD4_SOFTWARE_SCRATCH_REG           (0xA381)

#define CPLD4_I2C_PORT_ID_REG                (0xA390)
#define   CPLD4_I2C_BAUD_RATE_MASK           (0x40)
#define   CPLD4_I2C_PORT_ID_MASK             (0x3F)

#define CPLD4_I2C_OP_CODE_REG                (0xA391)
#define   CPLD4_I2C_DATA_BYTE_LEN_MASK       (0xF0)
#define   CPLD4_I2C_DATA_BYTE_LEN_SHIFT      (4)
#define   CPLD4_I2C_CMD_BYTE_LEN_MASK        (0x03)

#define CPLD4_I2C_DEVICE_ADDRESS_REG         (0xA392)
#define   CPLD4_I2C_SLAVE_ADDRESS_MASK       (0xFE)
#define   CPLD4_I2C_SLAVE_ADDRESS_SHIFT      (1)
#define   CPLD4_I2C_READ_WRITE_MASK          (0x01)

#define CPLD4_I2C_COMMAND_BYTE0_REG          (0xA393)
#define CPLD4_I2C_COMMAND_BYTE1_REG          (0xA394)
#define CPLD4_I2C_COMMAND_BYTE2_REG          (0xA395)

#define CPLD4_I2C_STATUS_REG                 (0xA396)
#define   CPLD4_I2C_MASTER_ERROR_MASK        (0x80)
#define   CPLD4_I2C_BUSY_INDICATOR_MASK      (0x40)
#define   CPLD4_I2C_MASTER_RESET_MASK        (0x01)

#define CPLD4_I2C_WRITE_DATA_BYTE0_REG       (0xA3A0)
#define CPLD4_I2C_WRITE_DATA_BYTE1_REG       (0xA3A1)
#define CPLD4_I2C_WRITE_DATA_BYTE2_REG       (0xA3A2)
#define CPLD4_I2C_WRITE_DATA_BYTE3_REG       (0xA3A3)
#define CPLD4_I2C_WRITE_DATA_BYTE4_REG       (0xA3A4)
#define CPLD4_I2C_WRITE_DATA_BYTE5_REG       (0xA3A5)
#define CPLD4_I2C_WRITE_DATA_BYTE6_REG       (0xA3A6)
#define CPLD4_I2C_WRITE_DATA_BYTE7_REG       (0xA3A7)

#define CPLD4_I2C_READ_DATA_BYTE0_REG        (0xA3B0)
#define CPLD4_I2C_READ_DATA_BYTE1_REG        (0xA3B1)
#define CPLD4_I2C_READ_DATA_BYTE2_REG        (0xA3B2)
#define CPLD4_I2C_READ_DATA_BYTE3_REG        (0xA3B3)
#define CPLD4_I2C_READ_DATA_BYTE4_REG        (0xA3B4)
#define CPLD4_I2C_READ_DATA_BYTE5_REG        (0xA3B5)
#define CPLD4_I2C_READ_DATA_BYTE6_REG        (0xA3B6)
#define CPLD4_I2C_READ_DATA_BYTE7_REG        (0xA3B7)

#define CPLD4_SFP_37_44_RX_LOS_REG           (0xA3C0)
#define CPLD4_SFP_45_48_RX_LOS_REG           (0xA3C1)

#define CPLD4_SFP_37_44_TX_DISABLE_REG       (0xA3D0)
#define CPLD4_SFP_45_48_TX_DISABLE_REG       (0xA3D1)

#define CPLD4_SFP_37_44_RS_REG               (0xA3D2)
#define CPLD4_SFP_45_48_RS_REG               (0xA3D3)

#define CPLD4_SFP_37_44_TX_FAULT_REG         (0xA3D4)
#define CPLD4_SFP_45_48_TX_FAULT_REG         (0xA3D5)

#define CPLD4_SFP_37_44_MOD_ABS_REG          (0xA3D6)
#define CPLD4_SFP_45_48_MOD_ABS_REG          (0xA3D7)

#endif /* CEL_REDSTONE_DP_H__ */
