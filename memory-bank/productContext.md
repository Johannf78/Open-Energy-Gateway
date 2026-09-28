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
4. **Real-Time**: Live web interface with WebSocket updates. TCP uses one Modbus socket per sweep and services the WebSocket between registers (Meter Details ~5 s on 28 Sep 2026, down from ~40 s)
5. **Cloud-Ready**: Automatic data upload to AmpX Portal API (local XAMPP or live `ampx.app`) with shared `X-AmpX-Api-Key`; firmware **1.2.1** posts `/api/v3/` (frequency and per-phase power factor included; not `power_factor_tot`); portal Meters/View Data read Cloud Serverless via InfluxQL (Influx 2 remains on `/api/v2/` until sunset)

### User Experience Goals
- **Plug-and-Play Setup**: WiFiManager for easy network configuration; Admin Clear WiFi (ship-mode) so a boxed unit boots AP at the customer site
- **Auto-Discovery**: Automatic meter detection and configuration
- **Intuitive Interface**: Clean web dashboard showing all meter data
- **Portal Meter Data**: Scrollable wide tables; clear 1000-row / 30-day display limits; CSV export of full 30-day window
- **Field firmware updates**: Admin Check for update + HTTP OTA from `ampx.app/firmware/` (no USB after first flash)
- **Mobile-Friendly**: Responsive design for smartphone/tablet access
- **Status Visibility**: LED indicators for system health monitoring; LEDs off immediately before Admin reboot / Clear WiFi restart

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
