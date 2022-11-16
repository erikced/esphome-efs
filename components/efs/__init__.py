import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import uart
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_RECEIVE_TIMEOUT,
)

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@erikced"]

AUTO_LOAD = ["sensor", "text_sensor"]

CONF_CRC_CHECK = "crc_check"
CONF_MAX_TELEGRAM_LENGTH = "max_telegram_length"
CONF_RECEIVE_TIMEOUT = "receive_timeout"

efs_ns = cg.esphome_ns.namespace("efs")
Efs = efs_ns.class_("Efs", cg.Component, uart.UARTDevice)


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Efs),
            cv.Optional(CONF_CRC_CHECK, default=True): cv.boolean,
            cv.Optional(CONF_MAX_TELEGRAM_LENGTH, default=1500): cv.int_,
            cv.Optional(CONF_RECEIVE_TIMEOUT, default=200): cv.int_,
        }
    ).extend(uart.UART_DEVICE_SCHEMA),
    cv.only_with_arduino,
)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component, config[CONF_CRC_CHECK])
    cg.add(var.set_max_telegram_length(config[CONF_MAX_TELEGRAM_LENGTH]))
    await cg.register_component(var, config)
