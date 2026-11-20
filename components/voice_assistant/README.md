# voice_assistant (copia locale)

Copia del componente `voice_assistant` di **ESPHome 2026.10.2** con un'unica modifica:

- nuova opzione `response_start_timeout` (default `2s`, come l'originale): quanto
  l'assistente aspetta che il media player cominci a riprodurre la risposta TTS prima
  di considerarla finita. In ESPHome è fisso a 2000 ms (`start_playback_timeout_`).

Perché serve: qui microfono e altoparlante condividono la porta I²S (MAX9860, un solo
clock). Se la sintesi vocale di HA impiega più di 2 s, l'assistente dà la risposta per
finita, e in una conversazione continua riapre il microfono prima che la risposta suoni:
così la risposta si perde.

Quando si aggiorna ESPHome: ricopiare i tre file della nuova versione e riapplicare la
modifica (cercare "Vimar" nei sorgenti), oppure passare alla versione ufficiale.

## Stato in ESPHome 2026.11.0

L'ufficiale ha `tts_playback_start_timeout` (esphome#18707; default 15 s da esphome#19408).
Passare all'ufficiale cambia però anche altre parti del flusso, perché la 2026.11.0 contiene
circa 220 righe di modifiche rispetto alla 2026.10.2 di questa copia:

- un watchdog di 2 s durante la riproduzione, separato dall'attesa dell'avvio;
- `RESPONSE_FINISHED` resta in attesa mentre il media player annuncia la risposta;
- se l'avvio non arriva in tempo, il turno successivo della conversazione viene saltato;
- il microfono si ferma su `STT_END` anche senza `STT_VAD_END`;
- lo stop durante la risposta passa da `cancel_response_()`.

Per passare: togliere `voice_assistant` da `external_components`, scrivere
`tts_playback_start_timeout: 30s` al posto di `response_start_timeout: 30s`, e provare la
conversazione continua e lo stop. Da seguire: esphome#22848 (risposta fermata prima dell'avvio,
aperta).
