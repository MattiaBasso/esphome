# i2s_audio (copia locale)

Copia del componente `i2s_audio` di **ESPHome 2026.10.2** con una sola aggiunta: il bus
I²S può funzionare in **full-duplex**. Così microfono (wake word) e altoparlante
lavorano insieme sulla stessa porta del MAX9860, e "Alexa" si sente anche durante la
musica o il TTS.

## Opzioni nuove

```yaml
i2s_audio:
  - id: i2s_bus
    full_duplex: true        # default false = comportamento originale (lock, half-duplex)
    i2s_lrclk_pin: ...       # obbligatori con full_duplex
    i2s_bclk_pin: ...
```

`mclk_multiple` accetta anche `768`: MCLK 12,288 MHz a 16 kHz.

## Come funziona con `full_duplex: true`

- **RX master permanente.** Il canale del microfono viene creato una volta sola (al primo
  avvio del microfono o dello speaker) e non viene mai cancellato. Fermare il microfono
  non tocca il driver; alla ripartenza i campioni vecchi vengono scartati.
- **TX slave permanente.** Al primo avvio lo speaker crea il TX, e IDF lo aggancia
  all'RX (BCLK/WS in comune, log "TX agganciato all'RX master"). Fermare lo speaker
  disabilita soltanto il canale. Alla ripartenza il canale viene riusato e lo slot
  mode (mono/stereo) viene riapplicato a ogni avvio con
  `i2s_channel_reconfig_std_slot`, perché segue i canali del flusso (TTS mono, musica
  stereo).
- Sample rate e bit per slot devono restare quelli del primo avvio; un formato diverso
  viene rifiutato con un errore.
- Il lock del bus non si usa.

## Perché il TX non si cancella mai

Prova su scheda (`duplex-test.yaml`, 7 giri, 2026-11-14):

- se si cancella il master, lo slave resta senza clock;
- dopo la cancellazione del TX slave, in 3 giri su 7 il codec resta sganciato (ULK
  persistente, probabilmente si perde l'MCLK);
- disable/enable è stato sempre OK.

Dettagli in `PROGETTO.md` e in `docs/superpowers/specs/2026-11-14-i2s-full-duplex-design.md`.

## Limiti

- Niente cancellazione d'eco: il microfono sente l'altoparlante (circa −35 dBFS per un
  tono a −12 dBFS). La wake word funziona comunque a volume moderato.
- Le modifiche sono marcate "Copia locale" nei sorgenti.

## Aggiornare ESPHome

Ricopiare i file della nuova versione di `i2s_audio` e riapplicare le modifiche. Sono in
`__init__.py`, `i2s_audio.h`, `microphone/i2s_audio_microphone.{h,cpp}` e
`speaker/i2s_audio_speaker{.h,.cpp,_standard.cpp}`. Tutti gli altri file sono identici a 2026.10.2.
Con 2026.11.0 è cambiata anche l'API dei DAC dello speaker (`audio_dacs_`, `has_audio_dac_()`).

## Stato in ESPHome 2026.11.0

L'ufficiale ora ha `full_duplex: true` (esphome#19959). Il design è simile a questo: RX e TX non
vengono mai cancellati e l'RX guida i clock. La copia resta per due motivi:

- `mclk_multiple` ufficiale arriva a 512, quindi a 16 kHz l'MCLK massimo è 8,192 MHz, sotto i
  10 MHz che il MAX9860 richiede. ESP-IDF ha `I2S_MCLK_MULTIPLE_768`, ma nessuna PR o issue upstream
  lo chiede (verificato il 2026-11-20).
- L'implementazione è diversa: i canali vengono creati nel setup del bus, l'RX si spegne quando
  microfono e speaker sono fermi e il formato è fisso. Il passaggio va provato sulla scheda.

Da seguire: esphome#21107 (`keep_tx_active`) ed esphome#22164 (`keep_alive` dei clock), tutte e due
aperte.
