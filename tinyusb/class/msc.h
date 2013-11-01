/**************************************************************************/
/*!
    @file     msc.h
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

/**
 *  \addtogroup ClassDriver Class Driver
 *  @{
 *  \defgroup ClassDriver_MSC MassStorage (MSC)
 *  @{ */

/**  \defgroup ClassDriver_MSC_Common Common Definitions
 *  @{ */

#ifndef _TUSB_MSC_H_
#define _TUSB_MSC_H_

#include "common/common.h"

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// USB Class Constant
//--------------------------------------------------------------------+

/// MassStorage Subclass
typedef enum {
  MSC_SUBCLASS_RBC = 1 , ///< Reduced Block Commands (RBC) T10 Project 1240-D
  MSC_SUBCLASS_SFF_MMC , ///< SFF-8020i, MMC-2 (ATAPI). Typically used by a CD/DVD device
  MSC_SUBCLASS_QIC     , ///< QIC-157. Typically used by a tape device
  MSC_SUBCLASS_UFI     , ///< UFI. Typically used by Floppy Disk Drive (FDD) device
  MSC_SUBCLASS_SFF     , ///< SFF-8070i. Can be used by Floppy Disk Drive (FDD) device
  MSC_SUBCLASS_SCSI      ///< SCSI transparent command set
}msc_subclass_type_t;

enum {
  MSC_CBW_SIGNATURE = 0x43425355, ///< Constant value of 43425355h (little endian)
  MSC_CSW_SIGNATURE = 0x53425355  ///< Constant value of 53425355h (little endian)
};

/// \brief MassStorage Protocol.
/// \details CBI only approved to use with full-speed floopy disk & should not used with highspeed or device other than floopy
typedef enum {
  MSC_PROTOCOL_CBI              = 0 ,  ///< Control/Bulk/Interrupt protocol (with command completion interrupt)
  MSC_PROTOCOL_CBI_NO_INTERRUPT = 1 ,  ///< Control/Bulk/Interrupt protocol (without command completion interrupt)
  MSC_PROTOCOL_BOT              = 0x50 ///< Bulk-Only Transport
}msc_protocol_type_t;

/// MassStorage Class-Specific Control Request
typedef enum {
  MSC_REQUEST_GET_MAX_LUN = 254, ///< The Get Max LUN device request is used to determine the number of logical units supported by the device. Logical Unit Numbers on the device shall be numbered contiguously starting from LUN 0 to a maximum LUN of 15
  MSC_REQUEST_RESET       = 255  ///< This request is used to reset the mass storage device and its associated interface. This class-specific request shall ready the device for the next CBW from the host.
}msc_request_type_t;

/// \brief Command Block Status Values
/// \details Indicates the success or failure of the command. The device shall set this byte to zero if the command completed
/// successfully. A non-zero value shall indicate a failure during command execution according to the following
typedef enum {
  MSC_CSW_STATUS_PASSED = 0 , ///< MSC_CSW_STATUS_PASSED
  MSC_CSW_STATUS_FAILED     , ///< MSC_CSW_STATUS_FAILED
  MSC_CSW_STATUS_PHASE_ERROR  ///< MSC_CSW_STATUS_PHASE_ERROR
}msc_csw_status_t;

/// Command Block Wrapper
typedef ATTR_PACKED_STRUCT(struct) {
  uint32_t signature   ; ///< Signature that helps identify this data packet as a CBW. The signature field shall contain the value 43425355h (little endian), indicating a CBW.
  uint32_t tag         ; ///< Tag sent by the host. The device shall echo the contents of this field back to the host in the dCSWTagfield of the associated CSW. The dCSWTagpositively associates a CSW with the corresponding CBW.
  uint32_t xfer_bytes  ; ///< The number of bytes of data that the host expects to transfer on the Bulk-In or Bulk-Out endpoint (as indicated by the Directionbit) during the execution of this command. If this field is zero, the device and the host shall transfer no data between the CBW and the associated CSW, and the device shall ignore the value of the Directionbit in bmCBWFlags.
  uint8_t  dir         ; ///< Bit 7 of this field define transfer direction \n - 0 : Data-Out from host to the device. \n - 1 : Data-In from the device to the host.
  uint8_t  lun         ; ///< The device Logical Unit Number (LUN) to which the command block is being sent. For devices that support multiple LUNs, the host shall place into this field the LUN to which this command block is addressed. Otherwise, the host shall set this field to zero.
  uint8_t  cmd_len     ; ///< The valid length of the CBWCBin bytes. This defines the valid length of the command block. The only legal values are 1 through 16
  uint8_t  command[16] ; ///< The command block to be executed by the device. The device shall interpret the first cmd_len bytes in this field as a command block
}msc_cmd_block_wrapper_t;

STATIC_ASSERT(sizeof(msc_cmd_block_wrapper_t) == 31, "size is not correct");

/// Command Status Wrapper
typedef ATTR_PACKED_STRUCT(struct) {
  uint32_t signature    ; ///< Signature that helps identify this data packet as a CSW. The signature field shall contain the value 53425355h (little endian), indicating CSW.
  uint32_t tag          ; ///< The device shall set this field to the value received in the dCBWTag of the associated CBW.
  uint32_t data_residue ; ///< For Data-Out the device shall report in the dCSWDataResiduethe difference between the amount of data expected as stated in the dCBWDataTransferLength, and the actual amount of data processed by the device. For Data-In the device shall report in the dCSWDataResiduethe difference between the amount of data expected as stated in the dCBWDataTransferLengthand the actual amount of relevant data sent by the device
  uint8_t  status       ; ///< indicates the success or failure of the command. Values from \ref msc_csw_status_t
}msc_cmd_status_wrapper_t;

STATIC_ASSERT(sizeof(msc_cmd_status_wrapper_t) == 13, "size is not correct");

/// SCSI Command Operation Code
typedef enum {
  SCSI_CMD_TEST_UNIT_READY      = 0x00, ///< The SCSI Test Unit Ready command is used to determine if a device is ready to transfer data (read/write), i.e. if a disk has spun up, if a tape is loaded and ready etc. The device does not perform a self-test operation.
  SCSI_CMD_INQUIRY              = 0x12, ///< The SCSI Inquiry command is used to obtain basic information from a target device.
  SCSI_CMD_READ_CAPACITY_10     = 0x25, ///< The SCSI Read Capacity command is used to obtain data capacity information from a target device.
  SCSI_CMD_REQUEST_SENSE        = 0x03, ///< The SCSI Request Sense command is part of the SCSI computer protocol standard. This command is used to obtain sense data -- status/error information -- from a target device.
  SCSI_CMD_READ_FORMAT_CAPACITY = 0x23, ///< The command allows the Host to request a list of the possible format capacities for an installed writable media. This command also has the capability to report the writable capacity for a media when it is installed
  SCSI_CMD_READ_10              = 0x28, ///< The READ (10) command requests that the device server read the specified logical block(s) and transfer them to the data-in buffer.
  SCSI_CMD_WRITE_10             = 0x2A, ///< The WRITE (10) command requests thatthe device server transfer the specified logical block(s) from the data-out buffer and write them.
}scsi_cmd_type_t;

//--------------------------------------------------------------------+
// SCSI Primary Command (SPC-4)
//--------------------------------------------------------------------+

/// SCSI Test Unit Ready Command
typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t cmd_code    ; ///< SCSI OpCode for \ref SCSI_CMD_TEST_UNIT_READY
  uint8_t lun         ; ///< Logical Unit
  uint8_t reserved[3] ;
  uint8_t control     ;
} scsi_test_unit_ready_t;

STATIC_ASSERT(sizeof(scsi_test_unit_ready_t) == 6, "size is not correct");

/// SCSI Inquiry Command
typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t cmd_code     ; ///< SCSI OpCode for \ref SCSI_CMD_INQUIRY
  uint8_t reserved1    ;
  uint8_t page_code    ;
  uint8_t reserved2    ;
  uint8_t alloc_length ; ///< specifies the maximum number of bytes that USB host has allocated in the Data-In Buffer. An allocation length of zero specifies that no data shall be transferred.
  uint8_t control      ;
} scsi_inquiry_t, scsi_request_sense_t;

STATIC_ASSERT(sizeof(scsi_inquiry_t) == 6, "size is not correct");

/// SCSI Inquiry Response Data
typedef ATTR_PACKED_STRUCT(struct)
{
  uint8_t peripheral_device_type     : 5;
  uint8_t peripheral_qualifier       : 3;

  uint8_t                            : 7;
  uint8_t is_removable               : 1;

  uint8_t version;

  uint8_t response_data_format       : 4;
  uint8_t hierarchical_support       : 1;
  uint8_t normal_aca                 : 1;
  uint8_t                            : 2;

  uint8_t additional_length;

  uint8_t protect                    : 1;
  uint8_t                            : 2;
  uint8_t third_party_copy           : 1;
  uint8_t target_port_group_support  : 2;
  uint8_t access_control_coordinator : 1;
  uint8_t scc_support                : 1;

  uint8_t addr16                     : 1;
  uint8_t                            : 3;
  uint8_t multi_port                 : 1;
  uint8_t                            : 1; // vendor specific
  uint8_t enclosure_service          : 1;
  uint8_t                            : 1;

  uint8_t                            : 1; // vendor specific
  uint8_t cmd_que                    : 1;
  uint8_t                            : 2;
  uint8_t sync                       : 1;
  uint8_t wbus16                     : 1;
  uint8_t                            : 2;

  uint8_t vendor_id[8]        ; ///< 8 bytes of ASCII data identifying the vendor of the product. The T10 vendor identification shall be one assigned by INCITS. A list of assigned T10 vendor identifications is in Annex E and on the T10 web site (http://www.t10.org).
  uint8_t product_id[16]      ; ///< 16 bytes of ASCII data defined by the vendor.
  uint8_t product_revision[4] ; ///< 4 bytes of ASCII data defined by the vendor.
} scsi_inquiry_data_t;

STATIC_ASSERT(sizeof(scsi_inquiry_data_t) == 36, "size is not correct");


typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t response_code           : 7; ///< 70h - current errors, Fixed Format 71h - deferred errors, Fixed Format
  uint8_t valid                   : 1;

  uint8_t reserved; ///< Obsolete

  uint8_t sense_key               : 4;
  uint8_t                         : 1;
  uint8_t incorrect_len_idicatior : 1;
  uint8_t end_of_medium           : 1;
  uint8_t filemark                : 1;

  uint32_t information;
  uint8_t  additional_sense_len;
  uint32_t command_specific_info;
  uint8_t additional_sense_code;
  uint8_t additional_sense_qualifier;
  uint8_t field_replaceable_unit_code;

  uint8_t sense_key_specific[3]; ///< sense key specific valid bit is bit 7 of key[0], aka MSB in Big Endian layout

} scsi_sense_fixed_data_t;

STATIC_ASSERT(sizeof(scsi_sense_fixed_data_t) == 18, "size is not correct");

//--------------------------------------------------------------------+
// SCSI MMC
//--------------------------------------------------------------------+
/// SCSI Read Format Capacity: Write Capacity
typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t cmd_code;
  uint8_t reserved[6];
  uint16_t alloc_length;
  uint8_t control;
} scsi_read_format_capacity_t;

STATIC_ASSERT( sizeof(scsi_read_format_capacity_t) == 10, "size is not correct");

typedef ATTR_PACKED_STRUCT(struct){
  uint8_t reserved[3];
  uint8_t list_length; /// must be 8*n, length in bytes of formattable capacity descriptor followed it.

  uint32_t block_num; /// Number of Logical Blocks
  uint8_t  descriptor_type; // 00: reserved, 01 unformatted media , 10 Formatted media, 11 No media present

  uint8_t  reserved2;
  uint16_t block_size;

} scsi_read_format_capacity_data_t;

STATIC_ASSERT( sizeof(scsi_read_format_capacity_data_t) == 12, "size is not correct");

//--------------------------------------------------------------------+
// SCSI Block Command (SBC-3)
// NOTE: All data in SCSI command are in Big Endian
//--------------------------------------------------------------------+

/// SCSI Read Capacity 10 Command: Read Capacity
typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t  cmd_code                 ; ///< SCSI OpCode for \ref SCSI_CMD_READ_CAPACITY_10
  uint8_t  reserved1                ;
  uint32_t lba                      ; ///< The first Logical Block Address (LBA) accessed by this command
  uint16_t reserved2                ;
  uint8_t  partial_medium_indicator ;
  uint8_t  control                  ;
} scsi_read_capacity10_t;

STATIC_ASSERT(sizeof(scsi_read_capacity10_t) == 10, "size is not correct");

/// SCSI Read Capacity 10 Response Data
typedef struct {
  uint32_t last_lba   ; ///< The last Logical Block Address of the device
  uint32_t block_size ; ///< Block size in bytes
} scsi_read_capacity10_data_t;

STATIC_ASSERT(sizeof(scsi_read_capacity10_data_t) == 8, "size is not correct");

/// SCSI Read 10 Command
typedef ATTR_PACKED_STRUCT(struct) {
  uint8_t  cmd_code    ; ///< SCSI OpCode
  uint8_t  reserved    ; // has LUN according to wiki
  uint32_t lba         ; ///< The first Logical Block Address (LBA) accessed by this command
  uint8_t  reserved2   ;
  uint16_t block_count ; ///< Number of Blocks used by this command
  uint8_t  control     ;
} scsi_read10_t, scsi_write10_t;

STATIC_ASSERT(sizeof(scsi_read10_t) == 10, "size is not correct");
STATIC_ASSERT(sizeof(scsi_write10_t) == 10, "size is not correct");

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_MSC_H_ */

/// @}
/// @}
/// @}
