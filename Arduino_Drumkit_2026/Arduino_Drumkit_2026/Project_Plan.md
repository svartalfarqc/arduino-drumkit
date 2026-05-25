# 🥁 Arduino Drumkit Build — Project Plan

**Toy:** First Act Discovery children's drum kit  
**Goal:** MIDI electronic drumkit with interactive LED modes  
**Pace:** ~1 hour per week  
**Estimated completion:** 10–12 weeks

---

## Overview

You're repurposing a 6-pad children's drum kit into a fully functional MIDI drum controller. The toy already contains piezoelectric sensors and LEDs in each pad — you're keeping the shell and sensors, gutting the original electronics, and dropping in an Arduino to handle everything.

---

## Hardware Inventory

### What you already have (in the toy)
| Component | Count | Notes |
|-----------|-------|-------|
| Piezoelectric sensors | 6 | One per drum pad (yellow/orange discs) |
| Small LED PCBs | 6 | One per pad — check voltage before reusing |
| 2-button switch module | 1 | Spring-contact switches in the central unit |
| Plastic shell + pads | 1 set | Foam-topped pads, sturdy housing |

### What you'll need
| Component | Notes |
|-----------|-------|
| Arduino Uno (or Nano) | Already have one |
| 1MΩ resistors × 6 | Pull-down for each piezo (critical for noise reduction) |
| 220Ω or 330Ω resistors × 10 | Current limiting for LEDs |
| 10kΩ resistors × 2 | Pull-down for each button |
| Small signal diodes × 6 (optional) | e.g., 1N4148 — protect analog pins from piezo voltage spikes |
| Jumper wires / hookup wire | Various lengths |
| USB cable | For MIDI-over-USB to PC |
| Breadboard or perfboard | For connections |
| Hot glue / mounting tape | To secure Arduino inside enclosure |

### Optional upgrades
- **loopMIDI** (free, Windows) — creates a virtual MIDI port so the USB serial signal is seen as MIDI by your DAW
- **Hairless MIDI** (free) — bridges serial to MIDI on older setups
- **GarageBand / REAPER / any DAW** — receives MIDI and plays drum sounds

---

## Wiring Diagram

```
ARDUINO UNO
─────────────────────────────────────────────────────
  A0  ──┬── Piezo Pad 0 (+)         [Piezo –] → GND
  A1  ──┤   Piezo Pad 1 (+)         1MΩ between each Ax and GND
  A2  ──┤   Piezo Pad 2 (+)         Optional: 1N4148 diode in series with piezo
  A3  ──┤   Piezo Pad 3 (+)
  A4  ──┤   Piezo Pad 4 (+)
  A5  ──┘   Piezo Pad 5 (+)

  D3  ──[220Ω]── Pad LED 0 (+)      [LED –] → GND
  D5  ──[220Ω]── Pad LED 1 (+)
  D6  ──[220Ω]── Pad LED 2 (+)
  D9  ──[220Ω]── Pad LED 3 (+)
  D10 ──[220Ω]── Pad LED 4 (+)
  D11 ──[220Ω]── Pad LED 5 (+)

  D7  ──[220Ω]── Indicator LED 1 (MSB)
  D8  ──[220Ω]── Indicator LED 2
  D12 ──[220Ω]── Indicator LED 3
  D13 ──[220Ω]── Indicator LED 4 (LSB)

  D2  ──── Button 1 (Mode) ──── 5V
           [10kΩ] from D2 to GND

  D4  ──── Button 2 (Alt) ──── 5V
           [10kΩ] from D4 to GND
```

> **Pad LED voltage check:** Measure the forward voltage of the existing toy LEDs before wiring. If they're 3V LEDs (common in toys), a 220Ω resistor from a 5V Arduino pin gives ~(5-3)/0.02 = 100mA — too much. Use 150Ω for blue/white, 220Ω for red/green/yellow.

---

## Button Scheme

| Button | Mode HIT | Mode LEARN |
|--------|----------|------------|
| **Button 1** (pin 2) | Cycle LED mode → LEARN | Cycle LED mode → HIT |
| **Button 2** (pin 4) | Cycle MIDI channel (0–15) | Reset game + score |

### About the 2-button module (the "lone input")
The spring-contact switch assembly visible in pad 1 is likely a **single DPDT or 2-pole push switch** — two independent momentary contacts in one plastic housing. Wire each contact between its Arduino pin and 5V, and add a 10kΩ pull-down to GND on each pin. These become your Button 1 and Button 2.

---

## LED Modes

### Mode 0 — HIT 💥
Each pad has its own LED. When you hit a pad, its LED flashes for ~25 loop cycles then fades out. The 4 indicator LEDs show the current MIDI channel in binary (0000 = ch.1, 0001 = ch.2 ... 1111 = ch.16).

**Good for:** Normal playing, checking that all sensors work.

### Mode 1 — LEARN 🎯
A randomly chosen pad lights up. Hit it to score a point. Miss it within 5 seconds and the LED blinks rapidly (your hint). Hit the wrong pad and you get a single blink on the correct pad. The 4 indicator LEDs show your score (0–15) in binary.

**Good for:** Kids learning which pad is which, or warming up between sessions.

### Suggested future Mode 2 — CHASE (not yet coded)
Pads light up one by one in a tempo-locked sequence (e.g., using `millis()` and a BPM variable). You follow the pattern. Essentially a rhythm trainer. Great next project once Mode 1 is solid.

---

## Phase Plan

### Phase 1 — Teardown & Inventory (Week 1–2)
**Goal:** Know exactly what you're working with.

- [ ] Open all 6 drum pads (4 screws each typically)
- [ ] Photograph each pad's wiring before disconnecting anything
- [ ] Identify the 2 wires on each piezo and each LED
- [ ] Measure LED forward voltage with a multimeter (diode test mode)
- [ ] Identify the 2-button switch and test continuity when pressed
- [ ] Decide which LED PCBs to reuse vs. replace with standard 5mm LEDs
- [ ] Inventory your extra components and confirm the resistor values you have

**Deliverable:** A photo + notes doc of what's inside.

---

### Phase 2 — Piezo + Basic MIDI (Week 3–4)
**Goal:** Get MIDI notes playing on a computer when you hit the pads.

- [ ] Wire piezos to A0–A5 with 1MΩ pull-down resistors
- [ ] Upload the original `Arduino_Drumkit.ino` (smaller scope to test first)
- [ ] Install loopMIDI + Hairless MIDI (or use a MIDI-over-USB library like MIDIUSB)
- [ ] Open a DAW, load a drum VST (or use Ugritone Kult Drums II), and test
- [ ] Adjust `PadCutOff[]` values per pad (some piezos are more sensitive than others)
- [ ] Adjust `MaxPlayTime[]` if notes are retriggering

**Deliverable:** All 6 pads trigger MIDI notes reliably.

> **Tip:** Set `VelocityFlag = false` initially so all hits play at 127. Once that works, turn it back on.

---

### Phase 3 — Buttons + Indicator LEDs (Week 5)
**Goal:** Both buttons work, indicator LEDs display the MIDI channel.

- [ ] Wire both buttons with 10kΩ pull-downs
- [ ] Wire 4 indicator LEDs (pins 7, 8, 12, 13) with 220Ω resistors
- [ ] Test: pressing Button 2 increments the MIDI channel, shown on indicator LEDs
- [ ] Test: gougoune mode works via original code's alt button behavior

**Deliverable:** Buttons and indicator LEDs functional.

---

### Phase 4 — Pad LEDs + HIT Mode (Week 6–7)
**Goal:** Each pad flashes its own LED when hit.

- [ ] Wire 6 pad LEDs to pins 3, 5, 6, 9, 10, 11 with appropriate resistors
- [ ] Upload `Arduino_Drumkit_Enhanced.ino`
- [ ] Confirm startup flash (all 6 LEDs blink once on boot)
- [ ] Play drums — verify each pad's LED flashes on hit
- [ ] Tune `LED_ON_CYCLES` to taste (higher = longer flash)

**Deliverable:** HIT mode fully working.

---

### Phase 5 — LEARN Mode (Week 8)
**Goal:** The game works — hit the lit pad, score goes up, timeout blinks.

- [ ] Press Button 1 to switch to LEARN mode
- [ ] Verify a random pad lights up on start
- [ ] Test correct hit: LED off, score increments on indicator LEDs, new pad lights
- [ ] Test wrong hit: correct pad blinks once as a hint
- [ ] Test timeout: pad blinks after 5 seconds, steady again after blink animation
- [ ] Test Button 2 in LEARN mode: score resets to 0, all pads blink, new target

**Deliverable:** Full LEARN mode game working.

---

### Phase 6 — Polish & Enclosure (Week 9–10)
**Goal:** It looks and feels like a finished instrument.

- [ ] Decide where to mount the Arduino (inside the central console housing is ideal)
- [ ] Route all wires cleanly — zip ties / cable channels
- [ ] Mount 4 indicator LEDs somewhere visible (drill small holes in the enclosure)
- [ ] Label the 2 buttons ("MODE" and "SET/RESET")
- [ ] Test for robustness: bang on the pads hard, check for false triggers
- [ ] Consider hot-gluing the Arduino in place
- [ ] Optional: add a power LED (basic 5mm LED on the 5V pin) so you know it's on

---

## Tips & Notes

**Piezo wiring:**
Adding a 1N4148 diode in series (anode toward the pin) between each piezo and its analog pin is strongly recommended. Hard hits can generate voltage spikes well above 5V that can damage the Arduino over time.

**MIDI over USB:**
The simplest approach is **Hairless MIDI + loopMIDI** on Windows. Alternatively, flash a MIDIUSB-capable bootloader onto the Uno so it appears as a native MIDI device — no extra software needed, but requires a second Arduino or programmer to reflash.

**`PadCutOff[]` tuning:**
Each piezo will be slightly different. Open Arduino Serial Monitor, add `Serial.print(hitavg)` temporarily, and tap each pad to see its analog range. Set the cutoff just above the idle noise floor (usually 0–50 for undisturbed pads).

**`MaxPlayTime[]` tuning:**
If a single hit is triggering multiple notes, increase this value. If the note seems to cut off before it should ring, decrease it.

**LED brightness:**
Pins 3, 5, 6, 9, 10, 11 are all PWM-capable. You can use `analogWrite(padLedPins[i], brightness)` instead of `digitalWrite` to control brightness — useful for a fade-out effect on pad hits instead of a hard cutoff.

**Future mode idea — CHASE:**
Add a BPM variable and light pads in sequence using `millis()`. The player must hit each pad in tempo. Great for rhythm training and surprisingly fun.

---

## Quick Reference

| Mode | Button 1 | Button 2 | Indicator LEDs | Pad LEDs |
|------|----------|----------|----------------|----------|
| HIT | Switch to LEARN | Next MIDI channel | MIDI channel (0–15) | Flash on hit |
| LEARN | Switch to HIT | Reset score | Score (0–15) | Target pad lit |
