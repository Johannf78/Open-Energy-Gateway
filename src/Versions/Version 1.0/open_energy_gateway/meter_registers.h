

/*
# Meter Register Definitions Documentation
# ======================================
# 
# This file documents the meter register definitions used in meter_registers.json
# themanspas
# Format: [register_number, number_of_registers, data_type, friendly_name, json_key]
# 
# Data Types:
# 1 = int32 (32-bit integer)
# 2 = int64 (64-bit integer) 
# 3 = float (32-bit floating point)
#
# Register Definitions:
# ====================
#
# Serial Number
# - Register: 70
# - Count: 2 registers
# - Data Type: int32 (1)
# - Description: Serial Number
# - JSON Key: serial
#
# Voltage Readings
# - L1: Register 1010, 2 registers, float (3), "Voltage L1", "voltage_L1"
# - L2: Register 1012, 2 registers, float (3), "Voltage L2", "voltage_L2"  
# - L3: Register 1014, 2 registers, float (3), "Voltage L3", "voltage_L3"
#
# Current Readings
# - L1: Register 1000, 2 registers, float (3), "Current L1", "current_L1"
# - L2: Register 1002, 2 registers, float (3), "Current L2", "current_L2"
# - L3: Register 1004, 2 registers, float (3), "Current L3", "current_L3"
# - Avg: Register 1006, 2 registers, float (3), "Current Avg", "current_avg"
#
# Power Readings
# - L1: Register 1028, 2 registers, float (3), "Active Power L1", "active_power_L1"
# - L2: Register 1030, 2 registers, float (3), "Active Power L2", "active_power_L2"
# - L3: Register 1032, 2 registers, float (3), "Active Power L3", "active_power_L3"
# - Total: Register 1034, 2 registers, float (3), "Active Power Total", "active_power_tot"
#
# Energy Readings (Int64 - 4 registers each)
# - L1: Register 2500, 4 registers, int64 (2), "Energy Imported L1", "active_energy_imported_L1"
# - L2: Register 2504, 4 registers, int64 (2), "Energy Imported L2", "active_energy_imported_L2"
# - L3: Register 2508, 4 registers, int64 (2), "Energy Imported L3", "active_energy_imported_L3"
# - Total: Register 2512, 4 registers, int64 (2), "Energy Imported Total", "active_energy_imported_tot"
#
# Notes:
# - Energy readings use 4 registers because they are 64-bit integers
# - All other readings use 2 registers (32-bit values)
# - Register numbers are based on Modbus standard addressing
# - JSON keys are used for web interface element IDs
*/



String meter_registers_meatrol = R"(
{
  "serialNumber": [70, 2, 1, "Serial Number", "serial"],
  "voltageL1": [1010, 2, 3, "Voltage L1", "voltage_L1"],
  "voltageL2": [1012, 2, 3, "Voltage L2", "voltage_L2"],
  "voltageL3": [1014, 2, 3, "Voltage L3", "voltage_L3"],
  "currentL1": [1000, 2, 3, "Current L1", "current_L1"],
  "currentL2": [1002, 2, 3, "Current L2", "current_L2"],
  "currentL3": [1004, 2, 3, "Current L3", "current_L3"],
  "currentAvg": [1006, 2, 3, "Current Avg", "current_avg"],
  "activePowerL1": [1028, 2, 3, "Active Power L1", "active_power_L1"],
  "activePowerL2": [1030, 2, 3, "Active Power L2", "active_power_L2"],
  "activePowerL3": [1032, 2, 3, "Active Power L3", "active_power_L3"],
  "activePowerTotal": [1034, 2, 3, "Active Power Total", "active_power_tot"],
  "energyImportedL1": [2500, 4, 2, "Energy Imported L1", "active_energy_imported_L1"],
  "energyImportedL2": [2504, 4, 2, "Energy Imported L2", "active_energy_imported_L2"],
  "energyImportedL3": [2508, 4, 2, "Energy Imported L3", "active_energy_imported_L3"],
  "energyImportedTotal": [2512, 4, 2, "Energy Imported Total", "active_energy_imported_tot"]
}
)";
