"""Helper per gli effetti dell'anello LED del Vimar: gamma con tinta costante (vedi vimar_leds.h).

Componente solo-header: con ``vimar_leds:`` nello YAML, ``vimar_leds.h`` entra nella build ed è disponibile
nelle lambda (``vimar_leds::dim()``, ``vimar_leds::dim_visible()``, ...), senza ``esphome: includes:``.
"""

import esphome.config_validation as cv
from esphome.types import ConfigType

CODEOWNERS = ["@MattiaBasso"]
DEPENDENCIES = ["light"]

CONFIG_SCHEMA = cv.Schema({})


async def to_code(config: ConfigType) -> None:
    pass
