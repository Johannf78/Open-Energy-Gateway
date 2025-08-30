# Product Context - AmpX Open Energy Gateway

## Problem Statement
Industrial and commercial facilities need comprehensive energy monitoring across multiple zones, departments, or tenants. Traditional solutions are often expensive, proprietary, or require complex infrastructure. There's a need for an open, cost-effective gateway that can aggregate data from multiple energy meters and provide both local monitoring and centralized data collection.

## Target Users

### Primary Users
- **Energy Managers**: Need real-time visibility into energy consumption patterns
- **Facility Managers**: Require multi-zone monitoring for cost allocation
- **System Integrators**: Deploy energy monitoring solutions for clients
- **Manufacturing Operations**: Monitor energy usage across production lines

### Use Cases
- **Manufacturing Facilities**: Multi-zone energy monitoring across production areas
- **Commercial Buildings**: Tenant energy tracking and billing
- **Energy Auditing**: Standardized data collection for efficiency assessments
- **Remote Monitoring**: Centralized meter management across multiple sites

## Solution Design

### Core Value Propositions
1. **Cost-Effective**: Open-source solution using affordable ESP32 hardware
2. **Scalable**: Monitor up to 32 meters from single gateway
3. **Flexible**: Support for both RS485 and TCP/IP communication
4. **Real-Time**: Live web interface with 3-second updates
5. **Cloud-Ready**: Automatic data upload to AmpX Portal

### User Experience Goals
- **Plug-and-Play Setup**: WiFiManager for easy network configuration
- **Auto-Discovery**: Automatic meter detection and configuration
- **Intuitive Interface**: Clean web dashboard showing all meter data
- **Mobile-Friendly**: Responsive design for smartphone/tablet access
- **Status Visibility**: LED indicators for system health monitoring

## Data Flow Architecture
```
Energy Meters → Modbus (RS485/TCP) → ESP32 Gateway → Local Web Interface
                                                   ↓
                                              WiFi Network
                                                   ↓
                                              AmpX Portal API
```

## Competitive Advantages
- **Open Source**: No vendor lock-in, customizable
- **Multi-Protocol**: Supports both RS485 and TCP/IP variants
- **Low Cost**: ESP32-based hardware vs expensive commercial gateways
- **Real-Time**: Live updates vs batch reporting
- **Modular**: Extensible architecture for future enhancements

## Success Criteria
- Stable operation in industrial environments
- Accurate data collection and transmission
- User-friendly setup and configuration
- Reliable 24/7 operation
- Integration compatibility with existing energy management systems
