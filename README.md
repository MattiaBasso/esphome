# Componenti ESPHome per il Vimar 03975 / 30815

Componenti esterni per [ESPHome](https://esphome.io) usati per trasformare il comando vocale
Vimar 03975/30815 (nato per Alexa) in un satellite vocale di Home Assistant, con un ESP32-S3
collegato alla scheda audio/UI originale (codec MAX9860, driver LED IS31FL3218).

Versione di ESPHome di riferimento: **2026.11.0**.

## Uso

```yaml
external_components:
  - source: github://MattiaBasso/esphome@<tag>
    components: [is31fl3218, max9860, vimar_leds]
```

`i2s_audio` e `voice_assistant` non servono più: da ESPHome 2026.11.0 il Vimar usa quelli ufficiali.
Le copie locali sono nei tag fino a `v2026.11.20.1`.

- `voice_assistant`: `tts_playback_start_timeout` al posto di `response_start_timeout`.
- `i2s_audio`: `full_duplex: true` ufficiale. Il MAX9860 vuole MCLK ≥ 10 MHz, quindi a 16 kHz serve
  `mclk_multiple` 768, che lo schema non accetta (arriva a 512). Si mette 512 su microfono e speaker e si
  imposta il 768 da un `on_boot` che gira prima del setup del bus (priorità 600):

  ```yaml
  esphome:
    on_boot:
      - priority: 700
        then:
          - lambda: |-
              id(i2s_mic)->set_mclk_multiple(I2S_MCLK_MULTIPLE_768);
              id(i2s_spk)->set_mclk_multiple(I2S_MCLK_MULTIPLE_768);
  ```

  L'ufficiale ferma i clock quando microfono e speaker sono fermi tutti e due: il MAX9860 si sgancia e si
  riaggancia alla ripartenza (provato 31 volte su 31).

Conviene indicare sempre un tag (o un commit) e non il branch, così una compilazione usa
esattamente la versione provata.

## Componenti

### `max9860`: codec audio MAX9860 (piattaforma `audio_dac`)

Codec mono a 16 bit (I²C, indirizzo 0x10): DAC per l'altoparlante e ADC per il microfono, con
preamplificatore, PGA e AGC del microfono.

```yaml
audio_dac:
  - platform: max9860
    id: codec
    mclk_frequency: 12.288MHz   # 10-60 MHz
    sample_rate: 16000
    use_microphone: true
    mic_preamp_gain: 20DB       # 0DB, 20DB, 30DB
    mic_pga_gain: 0dB           # 0-20 dB (ignorato se c'è agc_target)
    agc_target: -12dB           # facoltativo: attiva l'AGC del microfono (-18..-3 dB)
    min_volume: -50dB           # volume 0% (il 100% è 0 dB)
    irq_pin: GPIOxx             # facoltativo: uscita IRQ (open drain, attiva bassa)
    on_clip:                    # clipping di ADC/DAC (al massimo una volta al secondo)
      - logger.log: "clip"
    on_pll_unlock:              # PLL digitale sganciato dopo il primo aggancio
      - logger.log: "PLL sganciato"
```

`on_clip` e `on_pll_unlock` richiedono `irq_pin`. Senza `irq_pin` il componente non usa il loop.

### `is31fl3218`: driver LED IS31FL3218

Driver PWM a 18 canali (I²C, indirizzo fisso 0x54). Le modifiche vengono scritte in un solo
burst I²C per ciclo, seguito dal registro di aggiornamento.

```yaml
is31fl3218:
  - id: led_driver
    i2c_id: i2c_ui
    enable_pin: GPIOxx          # facoltativo: SDB (spegnimento hardware, attivo basso)

# LED RGB indirizzabili (tre canali ciascuno, 0-17), compatibili con effetti e partition
light:
  - platform: is31fl3218
    id: ring
    leds:
      - { red: 7, green: 8, blue: 6 }
      - { red: 10, green: 11, blue: 9 }
```

### `vimar_leds`: gamma con tinta costante (solo header)

Funzioni per le lambda degli effetti LED (da usare su una light con `gamma_correct: 1.0`): la gamma
si applica al colore intero invece che a ogni canale, così a bassa luminosità la tinta non cambia.
Basta la chiave vuota nello YAML, senza `esphome: includes:`.

```yaml
vimar_leds:

# nelle lambda: vimar_leds::dim(colore, k, luminosità), vimar_leds::dim_visible(...),
# vimar_leds::base_color(id(una_light))
```
