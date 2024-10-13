import re

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_KILOWATT,
    UNIT_KILOWATT_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE,
    UNIT_VOLT,
)
from . import Efs, CONF_EFS_ID

AUTO_LOAD = ["efs"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_EFS_ID): cv.use_id(Efs),
        cv.Optional("energy_imported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("energy_imported_tariff1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("energy_imported_tariff2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("energy_exported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("energy_exported_tariff1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("energy_exported_tariff2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("reactive_energy_imported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
            accuracy_decimals=3,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("reactive_energy_exported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
            accuracy_decimals=3,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional("power_imported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_exported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_imported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_exported"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_imported_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_exported_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_imported_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_exported_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_imported_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("power_exported_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_imported_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_exported_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_imported_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_exported_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_imported_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("reactive_power_exported_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("voltage_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("voltage_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("voltage_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("current_l1"): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("current_l2"): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional("current_l3"): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)

OBIS_CODES = {
    "energy_imported": "1-0:1.8.0",
    "energy_imported_tariff1": "1-0:1.8.1",
    "energy_imported_tariff2": "1-0:1.8.2",
    "energy_exported": "1-0:2.8.0",
    "energy_exported_tariff1": "1-0:2.8.1",
    "energy_exported_tariff2": "1-0:2.8.2",
    "reactive_energy_imported": "1-0:3.8.0",
    "reactive_energy_exported": "1-0:4.8.0",
    "power_imported": "1-0:1.7.0",
    "power_exported": "1-0:2.7.0",
    "reactive_power_imported": "1-0:3.7.0",
    "reactive_power_exported": "1-0:4.7.0",
    "power_imported_l1": "1-0:21.7.0",
    "power_exported_l1": "1-0:22.7.0",
    "power_imported_l2": "1-0:41.7.0",
    "power_exported_l2": "1-0:42.7.0",
    "power_imported_l3": "1-0:61.7.0",
    "power_exported_l3": "1-0:62.7.0",
    "reactive_power_imported_l1": "1-0:23.7.0",
    "reactive_power_exported_l1": "1-0:24.7.0",
    "reactive_power_imported_l2": "1-0:43.7.0",
    "reactive_power_exported_l2": "1-0:44.7.0",
    "reactive_power_imported_l3": "1-0:63.7.0",
    "reactive_power_exported_l3": "1-0:64.7.0",
    "voltage_l1": "1-0:32.7.0",
    "voltage_l2": "1-0:52.7.0",
    "voltage_l3": "1-0:72.7.0",
    "current_l1": "1-0:31.7.0",
    "current_l2": "1-0:51.7.0",
    "current_l3": "1-0:71.7.0",
}


async def to_code(config):
    hub = await cg.get_variable(config[CONF_EFS_ID])

    sensors = []
    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == sensor.Sensor:
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, "add_sensor")(obis_code_expr(key), sens))
            sensors.append(f"F({key})")


def obis_code_expr(measurement_name):
    numbers = re.split('[-:\.]', OBIS_CODES[measurement_name])
    return cg.RawExpression(f"esphome::efs::ObisCode({' ,'.join(numbers)})")

