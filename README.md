# Componenti ESPHome per il Vimar 03975 / 30815

Componenti esterni per [ESPHome](https://esphome.io) usati per trasformare il comando vocale
Vimar 03975/30815 (nato per Alexa) in un satellite vocale di Home Assistant, con un ESP32-S3
collegato alla scheda audio/UI originale (codec MAX9860, driver LED IS31FL3218).

Versione di ESPHome di riferimento: **2026.10.2**. `i2s_audio` e `voice_assistant` sono copie
dei componenti ufficiali di quella versione con piccole modifiche: con altre versioni di ESPHome
potrebbero non compilare o comportarsi diversamente (vedi i rispettivi README).

## Uso

```yaml
external_components:
  - source: github://MattiaBasso/esphome@<tag>
    components: [i2s_audio, is31fl3218, max9860, voice_assistant]
```

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

# Un canale come uscita singola
output:
  - platform: is31fl3218
    id: ch0
    channel: 0                  # 0-17

# Oppure LED RGB indirizzabili (tre canali ciascuno), compatibili con effetti e partition
light:
  - platform: is31fl3218
    id: ring
    leds:
      - { red: 7, green: 8, blue: 6 }
      - { red: 10, green: 11, blue: 9 }
```

### `i2s_audio`: I²S con full-duplex

Copia del componente ufficiale con l'opzione `full_duplex: true`: microfono e altoparlante
condividono la stessa porta I²S (il microfono è il master dei clock, lo speaker si aggancia come
slave), così la wake word funziona anche durante la riproduzione. Dettagli in
[components/i2s_audio/README.md](components/i2s_audio/README.md).

### `voice_assistant`: timeout della risposta configurabile

Copia del componente ufficiale con l'opzione `response_start_timeout` (default `2s`, come
l'originale). Dettagli in [components/voice_assistant/README.md](components/voice_assistant/README.md).

### `es8311`

Componente di un progetto precedente, non usato dal Vimar.
