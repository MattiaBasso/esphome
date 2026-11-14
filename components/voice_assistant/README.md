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
modifica (cercare "Vimar" nei sorgenti), oppure rimuovere la copia se l'opzione arriva
nella versione ufficiale.
