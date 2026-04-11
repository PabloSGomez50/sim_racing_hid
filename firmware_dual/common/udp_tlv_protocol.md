# UDP TLV Protocol v1

This document defines the wire protocol used between ESP32 (UDP sender) and Pico2W (UDP receiver).

## Transport

- Network: Wi-Fi
- Role: Pico2W AP, ESP32 STA
- Transport: UDP
- Destination: 192.168.4.1:7777

## Header (packed)

Size: 16 bytes

- magic: u16, fixed 0x5352 ('SR')
- version_major: u8
- version_minor: u8
- msg_type: u8
- flags: u8
- seq: u16
- timestamp_us: u32
- payload_len: u16
- header_crc: u16 (CRC16/IBM with this field set to 0 while computing)

## Message types

- 1: INPUT_EVENTS
- 2: HEARTBEAT
- 3: ACK

## INPUT_EVENTS payload

- event_count: u8
- repeated TLV entries:
  - event_id: u16
  - event_class: u8
  - value_type: u8
  - value_len: u8
  - value: bytes[value_len]

Current sender emits:
- BTN_1 as BOOL
- ENC_1_DELTA as I16
- LINK_UPTIME_MS as I32

## Enums and logs

Event names and classes are not transmitted as strings.
Both firmwares use enum IDs from udp_tlv_protocol.h and format logs locally.

## Compatibility

- major version mismatch: incompatible
- minor version mismatch: compatible if parser can ignore unknown TLVs
- parsers must reject malformed lengths and invalid CRC
