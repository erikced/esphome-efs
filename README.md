# ESPHome-EFS

ESPHome-EFS is a custom component for ESPHome that reads data from smart meters
through the P1 port. It supports Dutch Smart Meters (DSMR) and other European
smart meters that use the DLMS/COSEM standard with OBIS codes. This component
is a fork of the ESPHome [DSMR component](https://esphome.io/components/sensor/dsmr.html),
retaining most of its functionality while introducing a more flexible parser.
Consequently, settings under the `efs` key are identical to those for the `dsmr`
component, but the `sensor` configuration is different.

## Configuration

### Basic Configuration

```yaml
# Import the module as an external component
external_components:
  - source: github://erikced/esphome-efs

# Example minimal configuration
uart:
  baud_rate: 115200
  rx_pin: D7
  rx_buffer_size: 1700  # Ensure this is large enough for your meter's telegrams

efs:
  max_telegram_length: 1700  # Should match or be smaller than uart rx_buffer_size
```

### Advanced Configuration

```yaml
efs:
  max_telegram_length: 1700
  decryption_key: "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  # For encrypted meters
  request_pin: D5  # GPIO pin for request signal
  request_interval: 10s  # How often to request new data
  receive_timeout: 200ms  # Timeout for receiving telegram
```

## Sensors

### Configuration

Sensors are configured like standard ESPHome sensors but require an additional
`obis_code` parameter. For convenience, several predefined sensors are
available. Any parameter of a predefined sensor can be overridden, similar to
custom sensors. OBIS codes are specified using OBIS Reduced IDs with value
group F omitted, i.e. on the format `A-B:C.D.E`.

```yaml
sensor:
  - platform: efs
    energy_imported:
      name: Energy Imported
    power_imported:
      name: Power Imported
    voltage_l1:
      name: Voltage L1
    current_l1:
      name: Current L1
    my_custom_sensor:
      name: My Custom Value
      obis_code: 1-2:3.4.5  # Specify any valid OBIS code
      unit_of_measurement: kWh  # Optional
      accuracy_decimals: 3  # Optional
      device_class: energy  # Optional
      state_class: total_increasing  # Optional
```
See the [ESPHome Sensor Component](https://esphome.io/components/sensor/index.html)
documentation for more information on sensor configuration and filters.

### Available Predefined Sensors

| Sensor Name | OBIS Code | Unit | Description |
|-------------|-----------|------|-------------|
| energy_imported | 1-0:1.8.0 | kWh | Total imported energy |
| energy_imported_tariff1 | 1-0:1.8.1 | kWh | Imported energy tariff 1 |
| energy_imported_tariff2 | 1-0:1.8.2 | kWh | Imported energy tariff 2 |
| energy_exported | 1-0:2.8.0 | kWh | Total exported energy |
| energy_exported_tariff1 | 1-0:2.8.1 | kWh | Exported energy tariff 1 |
| energy_exported_tariff2 | 1-0:2.8.2 | kWh | Exported energy tariff 2 |
| reactive_energy_imported | 1-0:3.8.0 | kVArh | Total imported reactive energy |
| reactive_energy_exported | 1-0:4.8.0 | kVArh | Total exported reactive energy |
| power_imported | 1-0:1.7.0 | kW | Current imported power |
| power_exported | 1-0:2.7.0 | kW | Current exported power |
| reactive_power_imported | 1-0:3.7.0 | kVAr | Current imported reactive power |
| reactive_power_exported | 1-0:4.7.0 | kVAr | Current exported reactive power |
| power_imported_l1 | 1-0:21.7.0 | kW | Imported power phase L1 |
| power_exported_l1 | 1-0:22.7.0 | kW | Exported power phase L1 |
| power_imported_l2 | 1-0:41.7.0 | kW | Imported power phase L2 |
| power_exported_l2 | 1-0:42.7.0 | kW | Exported power phase L2 |
| power_imported_l3 | 1-0:61.7.0 | kW | Imported power phase L3 |
| power_exported_l3 | 1-0:62.7.0 | kW | Exported power phase L3 |
| reactive_power_imported_l1 | 1-0:23.7.0 | kVAr | Imported reactive power phase L1 |
| reactive_power_exported_l1 | 1-0:24.7.0 | kVAr | Exported reactive power phase L1 |
| reactive_power_imported_l2 | 1-0:43.7.0 | kVAr | Imported reactive power phase L2 |
| reactive_power_exported_l2 | 1-0:44.7.0 | kVAr | Exported reactive power phase L2 |
| reactive_power_imported_l3 | 1-0:63.7.0 | kVAr | Imported reactive power phase L3 |
| reactive_power_exported_l3 | 1-0:64.7.0 | kVAr | Exported reactive power phase L3 |
| voltage_l1 | 1-0:32.7.0 | V | Voltage phase L1 |
| voltage_l2 | 1-0:52.7.0 | V | Voltage phase L2 |
| voltage_l3 | 1-0:72.7.0 | V | Voltage phase L3 |
| current_l1 | 1-0:31.7.0 | A | Current phase L1 |
| current_l2 | 1-0:51.7.0 | A | Current phase L2 |
| current_l3 | 1-0:71.7.0 | A | Current phase L3 |

## Complete Example

Here is a complete example for a [SlimmeLezer+](https://www.zuidwijk.com/product/slimmelezer-plus/).


```yaml
substitutions:
  device_name: slimmelezer
  friendly_name: SlimmeLezer

esphome:
  name: ${device_name}
  friendly_name: ${friendly_name}
  comment: DIY P1 module to read your smart meter
  name_add_mac_suffix: false

esp8266:
  board: d1_mini
  restore_from_flash: true

wifi:
  ssid: my-wifi
  password: !secret wifi_password

  # Optional: Fallback access point
  ap:
    ssid: ${device_name}
    password: !secret fallback_hotspot_password

captive_portal:

logger:
  baud_rate: 0

api:
  encryption:
    key: !secret api_key

ota:
  platform: esphome
  password: !secret ota_password

web_server:
  port: 80

uart:
  baud_rate: 115200
  rx_pin: D7
  rx_buffer_size: 1700

external_components:
  - source: github://erikced/esphome-efs

efs:
  max_telegram_length: 1700

sensor:
  # System sensors
  - platform: uptime
    name: ${friendly_name} Uptime
  - platform: wifi_signal
    name: ${friendly_name} Wi-Fi Signal
    update_interval: 60s

  # Smart meter sensors
  - platform: efs
    energy_imported:
      name: Energy imported
    power_imported:
      name: Power imported
    power_imported_l1:
      name: Power imported L1
    power_imported_l2:
      name: Power imported L2
    power_imported_l3:
      name: Power imported L3
    voltage_l1:
      name: Voltage L1
    voltage_l2:
      name: Voltage L2
    voltage_l3:
      name: Voltage L3
    current_l1:
      name: Current L1
    current_l2:
      name: Current L2
    current_l3:
      name: Current L3

text_sensor:
  - platform: wifi_info
    ip_address:
      name: ${friendly_name} IP Address
    ssid:
      name: ${friendly_name} Wi-Fi SSID
  - platform: version
    name: ESPHome Version
```

