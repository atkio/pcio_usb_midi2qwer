# pcio_usb_midi2qwer
usb midi keyboard to qwer keyboard convertor

```
              ┌────────────────────────────────┐           ┌──────────────────────────────────┐       
              │ USB MIDI HOST         RP2040   │           │ USB KEYBOARD DEVICE     RP2040   │       
              │                                │           │                                  │       
              │                                │           │                                  │       
              │                                │           │                                  │       
              │                                │           │                                  │       
MIDI KEYBOARD │ USB                   SERIAL2  │           │ SERIAL2                    USB   │   PC    
         ─────┼───                          RX───────────────TX                           ────┼───────
              │                             TX───────────────RX                               │       
              │                                │           │                                  │       
              │                                │           │                                  │       
              │                                │           │                                  │       
              │                                │           │                                  │       
              └────────────────────────────────┘           └──────────────────────────────────┘       

```


