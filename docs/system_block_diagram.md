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
