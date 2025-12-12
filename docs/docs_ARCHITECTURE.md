# LoRa868-to-MQTT Gateway Architecture

This document describes the system architecture for the LoRa868-to-MQTT Gateway project.

## System Block Diagram

```mermaid
graph TB
    subgraph "LoRa868-to-MQTT Gateway (Pico2 W RP235x)"
        subgraph "Hardware Layer"
            PICO[Pico2 W<br/>RP235x Dual-Core<br/>Wi-Fi Enabled]
            SX127X[SX127x LoRa Module<br/>868MHz SPI]
            FLASH[Flash Storage<br/>4K Sector]
            
            PICO <--> |SPI + IRQ GPIO| SX127X
            PICO <--> |Flash I/O| FLASH
        end
        
        subgraph "Core 0 - Network & Web"
            WIFI[Wi-Fi Manager<br/>lwIP Stack]
            MQTT[MQTT Handler<br/>Pub/Sub Client]
            WEB[Web Server<br/>Config UI]
            
            WIFI --> MQTT
            WIFI --> WEB
        end
        
        subgraph "Core 1 - LoRa Processing"
            LORA_H[LoRa Handler<br/>RX/TX Manager]
            PKT_DEC[Packet Decoder<br/>JSON Mapping]
            WHITELIST[Whitelist Filter<br/>ID Validation]
            
            LORA_H --> PKT_DEC
            PKT_DEC --> WHITELIST
        end
        
        subgraph "Shared Components"
            CONFIG[Config Manager<br/>Flash R/W<br/>JSON Parser]
            SCANNER[Scanner Mode<br/>Device Discovery]
            
            CONFIG <--> FLASH
            CONFIG --> WHITELIST
            CONFIG --> PKT_DEC
            SCANNER --> WEB
        end
        
        PICO --> WIFI
        PICO --> LORA_H
        
        LORA_H <--> |IRQ Event| PICO
        WHITELIST --> MQTT
        SCANNER <--> LORA_H
    end
    
    subgraph "External Systems"
        NODES[LoRa Sensor Nodes<br/>868MHz]
        BROKER[MQTT Broker<br/>Network]
        HA[Home Assistant<br/>MQTT Discovery]
        USER[User Browser<br/>Config Interface]
        
        NODES <--> |LoRa RF| SX127X
        MQTT <--> |TCP/IP| BROKER
        BROKER <--> |MQTT| HA
        USER <--> |HTTP| WEB
    end
    
    style PICO fill:#e1f5ff
    style SX127X fill:#fff3e0
    style FLASH fill:#f3e5f5
    style MQTT fill:#e8f5e9
    style WEB fill:#fff9c4
    style LORA_H fill:#ffe0b2
    style CONFIG fill:#f1f8e9
```

## UML Class Diagram

```mermaid
classDiagram
    class LoRaHandler {
        -spi_inst_t* spi
        -uint8_t cs_pin
        -uint8_t irq_pin
        -bool rx_active
        +init() bool
        +send_packet(packet) bool
        +receive_packet() lora_pkt_t*
        +set_mode(mode) void
        +handle_irq() void
        -configure_radio() void
        -read_register(addr) uint8_t
        -write_register(addr, value) void
    }

    class lora_pkt_t {
        +uint8_t m
        +uint8_t n
        +uint16_t id
        +sensor_data_t sensors[MAX]
        +uint8_t checksum
        +validate_checksum() bool
    }

    class sensor_data_t {
        +uint8_t type
        +int16_t value
    }

    class MQTTHandler {
        -mqtt_client_t* client
        -char* broker_addr
        -uint16_t broker_port
        -char* username
        -char* password
        -bool connected
        +init(config) bool
        +connect() bool
        +disconnect() void
        +publish(topic, payload) bool
        +subscribe(topic, callback) bool
        +publish_discovery(device) bool
        -handle_connect_callback() void
        -handle_message_callback() void
    }

    class WebServer {
        -httpd_handle_t server
        -uint16_t port
        +init() bool
        +start() bool
        +stop() void
        +register_uri_handler(uri, handler) void
        -handle_config_page() void
        -handle_scanner_page() void
        -handle_api_config() void
        -handle_api_whitelist() void
        -handle_api_devices() void
    }

    class ConfigManager {
        -uint32_t flash_offset
        -whitelist_t whitelist
        -device_config_t devices[MAX]
        -wifi_config_t wifi
        -mqtt_config_t mqtt
        +load_config() bool
        +save_config() bool
        +load_whitelist() bool
        +save_whitelist() bool
        +add_device(config) bool
        +remove_device(id) bool
        +get_device_config(id) device_config_t*
        +is_whitelisted(id) bool
        -read_flash(offset, buffer, size) void
        -write_flash(offset, buffer, size) void
        -erase_sector(offset) void
    }

    class whitelist_t {
        +uint16_t device_ids[MAX]
        +uint16_t count
        +add_id(id) bool
        +remove_id(id) bool
        +contains(id) bool
    }

    class device_config_t {
        +uint16_t node_id
        +char name[32]
        +sensor_mapping_t sensors[MAX]
        +uint8_t sensor_count
        +to_json() char*
        +from_json(json) bool
    }

    class sensor_mapping_t {
        +char label[32]
        +uint8_t byte_offset
        +uint8_t length
        +char format[16]
        +float multiplier
        +char mqtt_topic[128]
    }

    class PacketDecoder {
        -ConfigManager* config_mgr
        +decode_packet(packet) decoded_data_t*
        +validate_packet(packet) bool
        -apply_sensor_mapping(raw, mapping) float
        -format_value(raw, format, multiplier) float
    }

    class decoded_data_t {
        +uint16_t node_id
        +sensor_value_t values[MAX]
        +uint8_t count
        +uint32_t timestamp
    }

    class sensor_value_t {
        +char label[32]
        +float value
        +char mqtt_topic[128]
    }

    class ScannerMode {
        -bool active
        -discovered_device_t devices[MAX]
        -uint16_t device_count
        +enable() void
        +disable() void
        +is_active() bool
        +add_discovered(packet) void
        +get_devices() discovered_device_t*
        +clear() void
    }

    class discovered_device_t {
        +uint16_t node_id
        +uint8_t packet_count
        +uint32_t last_seen
        +int8_t rssi
        +lora_pkt_t last_packet
    }

    class wifi_config_t {
        +char ssid[32]
        +char password[64]
        +bool ap_mode
    }

    class mqtt_config_t {
        +char broker[64]
        +uint16_t port
        +char username[32]
        +char password[64]
        +char base_topic[64]
    }

    class HomeAssistantIntegration {
        -MQTTHandler* mqtt
        +publish_discovery_config(device) bool
        +publish_sensor_state(topic, value) bool
        -build_discovery_payload(sensor) char*
        -build_state_payload(value) char*
    }

    class GatewayMain {
        -LoRaHandler lora
        -MQTTHandler mqtt
        -WebServer web
        -ConfigManager config
        -PacketDecoder decoder
        -ScannerMode scanner
        +init() bool
        +run() void
        -core0_main() void
        -core1_main() void
        -handle_lora_packet(packet) void
        -publish_sensor_data(data) void
    }

    %% Relationships
    GatewayMain *-- LoRaHandler
    GatewayMain *-- MQTTHandler
    GatewayMain *-- WebServer
    GatewayMain *-- ConfigManager
    GatewayMain *-- PacketDecoder
    GatewayMain *-- ScannerMode

    LoRaHandler .. > lora_pkt_t : creates
    lora_pkt_t *-- sensor_data_t

    PacketDecoder --> ConfigManager : uses
    PacketDecoder .. > decoded_data_t : creates
    decoded_data_t *-- sensor_value_t

    ConfigManager *-- whitelist_t
    ConfigManager *-- device_config_t
    ConfigManager *-- wifi_config_t
    ConfigManager *-- mqtt_config_t
    device_config_t *-- sensor_mapping_t

    ScannerMode ..> discovered_device_t : manages
    discovered_device_t *-- lora_pkt_t

    MQTTHandler <-- HomeAssistantIntegration
    HomeAssistantIntegration .. > device_config_t : uses

    WebServer --> ConfigManager : reads/writes
    WebServer --> ScannerMode : controls
```

## Normal Operation Sequence

```mermaid
sequenceDiagram
    participant Node as LoRa Node
    participant SX127x as SX127x Module
    participant LoRa as LoRaHandler
    participant Dec as PacketDecoder
    participant WL as Whitelist
    participant MQTT as MQTTHandler
    participant Broker as MQTT Broker
    participant HA as Home Assistant

    Note over Node,HA: Normal Operation - Whitelisted Device

    Node->>SX127x:  Transmit LoRa Packet (868MHz)
    SX127x->>LoRa: IRQ Trigger (RX Complete)
    activate LoRa
    
    LoRa->>LoRa: Read Packet via SPI
    LoRa->>LoRa: Validate Checksum
    LoRa->>WL: Check if ID Whitelisted
    activate WL
    
    alt Device Whitelisted
        WL-->>LoRa: ✓ Approved
        deactivate WL
        LoRa->>Dec: Pass Packet for Decoding
        activate Dec
        
        Dec->>Dec: Load Device Config from Flash
        Dec->>Dec:  Apply Sensor Mappings
        Dec->>Dec:  Convert Raw Values (multiplier)
        Dec-->>LoRa: Return Decoded Data
        deactivate Dec
        
        LoRa->>MQTT: Publish Sensor Data
        deactivate LoRa
        activate MQTT
        
        loop For Each Sensor
            MQTT->>Broker:  PUBLISH lora/sensor/1234/temperature
            Broker->>HA: Forward Message
            HA->>HA: Update Entity State
        end
        
        deactivate MQTT
        
    else Device Not Whitelisted
        WL-->>LoRa:  ✗ Rejected
        deactivate WL
        LoRa->>LoRa: Drop Packet
        deactivate LoRa
    end
```

## Scanner Mode Sequence

```mermaid
sequenceDiagram
    participant User as User Browser
    participant Web as WebServer
    participant Scan as ScannerMode
    participant LoRa as LoRaHandler
    participant Node as Unknown Node
    participant Cfg as ConfigManager

    Note over User,Cfg: Scanner Mode - Device Discovery

    User->>Web: Enable Scanner Mode
    activate Web
    Web->>Scan: Enable()
    activate Scan
    Scan->>LoRa: Disable Whitelist Filter
    Web-->>User: Scanner Active
    deactivate Web

    Node->>LoRa: LoRa Packet (Any ID)
    activate LoRa
    LoRa->>Scan: New Packet Received
    Scan->>Scan: Add to Discovered List
    Scan->>Scan: Store RSSI, Timestamp
    deactivate LoRa

    User->>Web: GET /api/scanner/devices
    activate Web
    Web->>Scan: Get Discovered Devices
    Scan-->>Web: Device List (ID, RSSI, Last Seen)
    Web-->>User: JSON Response
    deactivate Web

    User->>Web: POST /api/whitelist/add {id: 1234}
    activate Web
    Web->>Cfg: Add to Whitelist
    activate Cfg
    Cfg->>Cfg: Update Whitelist
    Cfg->>Cfg: Write to Flash
    Cfg-->>Web: Success
    deactivate Cfg
    Web-->>User: Device Whitelisted
    deactivate Web

    User->>Web:  POST /api/devices/config {node_id, sensors... }
    activate Web
    Web->>Cfg: Save Device Config
    activate Cfg
    Cfg->>Cfg: Parse JSON Mapping
    Cfg->>Cfg: Write to Flash
    Cfg-->>Web: Success
    deactivate Cfg
    Web-->>User: Config Saved
    deactivate Web

    User->>Web:  Disable Scanner Mode
    activate Web
    Web->>Scan: Disable()
    Scan->>LoRa:  Enable Whitelist Filter
    Scan->>Scan: Clear Discovered List
    deactivate Scan
    Web-->>User: Scanner Disabled
    deactivate Web
```

## Component Interaction

```mermaid
graph LR
    subgraph "Core 0 - Network Thread"
        A[Wi-Fi<br/>Connection]
        B[MQTT<br/>Client]
        C[Web<br/>Server]
        D[HTTP<br/>Handlers]
        
        A --> B
        A --> C
        C --> D
    end
    
    subgraph "Core 1 - LoRa Thread"
        E[LoRa<br/>IRQ Handler]
        F[Packet<br/>RX Queue]
        G[Packet<br/>Processor]
        
        E --> F
        F --> G
    end
    
    subgraph "Shared Data (Mutex Protected)"
        H[(Config<br/>Manager)]
        I[(Whitelist)]
        J[(Device<br/>Configs)]
        K[(Scanner<br/>State)]
        
        H --> I
        H --> J
        H --> K
    end
    
    G --> I
    G --> J
    G --> B
    G --> K
    D --> H
    K --> D
    
    style A fill:#e3f2fd
    style B fill:#e8f5e9
    style C fill:#fff9c4
    style E fill:#ffe0b2
    style H fill:#f3e5f5
```

## Key Design Principles

1. **Dual-Core Architecture**
   - **Core 0:** Handles Wi-Fi, MQTT, and Web Server
   - **Core 1:** Handles LoRa packet processing

2. **Flash-Based Persistence**
   - Configuration stored in 4K flash sector
   - Whitelist and device configs persisted across reboots

3. **Security by Whitelist**
   - Only configured device IDs are processed
   - Scanner mode for easy onboarding

4. **Flexible Sensor Mapping**
   - JSON-based configuration per device
   - Support for various data formats and multipliers

5. **Home Assistant Integration**
   - MQTT Discovery for automatic sensor creation
   - State updates published to configured topics

---

**Generated:** 2025-12-12  
**Project:** LoRa868-to-MQTT Gateway