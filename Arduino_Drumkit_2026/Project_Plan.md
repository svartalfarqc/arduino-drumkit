# 🥁 Arduino Drumkit Build — Project Plan

**Toy:** First Act Discovery children's drum kit  
**Goal:** MIDI electronic drumkit with interactive LED modes  
**Pace:** ~1 hour per week  
**Estimated total:** 12–15 weeks

---

## What's Inside the Toy

| Component | Count | Notes |
|-----------|-------|-------|
| Piezoelectric sensors | 5 | One per drum pad (yellow/orange discs) |
| LED PCBs | 5 | One per pad — small green boards |
| Slider + 2-button module | 1 | In the top-left section of the unit |
| Plastic shell + foam pads | 1 set | Sturdy housing, reusable as-is |

---

## Build Phases

---

### Phase 1 — Piezos + MIDI (Weeks 1–4)
**Goal:** Hit a pad, hear a drum sound on your computer.

**Hardware needed:**
- Arduino Uno (or Nano)
- 1MΩ resistors × 5 (pull-down, one per piezo)
- Optional but recommended: 1N4148 diodes × 5 (protect analog pins from voltage spikes on hard hits)
- USB cable + computer

**Wiring:**
Arduino A0–A4  →  Piezo (+) wires, one per pad
GND            →  Piezo (–) wires, all shared
1MΩ resistor   →  Between each Ax pin and GND

**Software setup:**
- Install [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) (virtual MIDI port, free, Windows)
- Install [Hairless MIDI](https://projectgus.github.io/hairless-midiserial/) (bridges Arduino serial → MIDI)
- Open a DAW (GarageBand, REAPER, etc.) and load a drum VST

**Tasks:**
- [ ] Open all 5 pads, photograph wiring before touching anything
- [ ] Identify and label the 2 wires on each piezo
- [ ] Wire piezos to A0–A4 with 1MΩ pull-downs
- [ ] Upload Phase 1 code
- [ ] Launch loopMIDI + Hairless MIDI, confirm signal reaches DAW
- [ ] Tune `PadCutOff[]` per pad (open Serial Monitor, add a temporary `Serial.print(hitavg)` to see raw values, set cutoff just above idle noise ~50–80)
- [ ] Tune `MaxPlayTime[]` if notes retrigger or cut off early
- [ ] All 5 pads trigger correct MIDI notes reliably

> **Tip:** Set `VelocityFlag = false` while testing so every hit plays at full volume. Turn it back on once the basics work.

---

### Phase 2 — Pad LEDs (Weeks 5–7)
**Goal:** Each pad's LED flashes when that pad is hit.

**Hardware needed:**
- Measure the existing LED PCB forward voltage (multimeter → diode mode) before wiring
- 220Ω resistors (red/green/yellow LEDs) or 150Ω (blue/white LEDs) × 5
- If existing LED PCBs are awkward to reuse: standard 5mm LEDs as replacements

**Wiring:**
Arduino D3, D5, D6, D9, D10  →  LED (+), one per pad (all PWM-capable)
GND                           →  LED (–), all shared
Resistor in series on each    →  Between pin and LED (+)

**Tasks:**
- [ ] Test existing LED PCBs with a 3V coin cell — confirm they light up
- [ ] Wire LEDs to PWM pins with appropriate resistors
- [ ] Add LED flash logic to code (pad hit → LED on for ~25 loop cycles → off)
- [ ] Confirm startup flash (all LEDs blink once on boot = wiring check)
- [ ] All 5 pad LEDs respond correctly to hits

> **Note:** Pins 3, 5, 6, 9, 10 are PWM-capable, which means later you can use `analogWrite()` for brightness control or fade-out effects instead of hard on/off — no rewiring needed.

---

### Phase 3 — Slider + Button Module (Weeks 8–9)
**Goal:** Understand and wire the existing slider + 2-button input. Decide what each does.

**Hardware needed:**
- 10kΩ resistors × 2 (pull-down for each button)
- 3 wires for the potentiometer (5V, GND, wiper → analog pin)

**Wiring:**
Slider wiper   →  A5
Slider ends    →  5V and GND
Button 1       →  D2, with 10kΩ to GND
Button 2       →  D4, with 10kΩ to GND

**Tasks:**
- [ ] Open the slider module, identify the 3 potentiometer pins and 2 button pins
- [ ] Wire slider to A5, read values in Serial Monitor across full travel (expect 0–1023)
- [ ] Wire both buttons, confirm edge detection (LOW→HIGH on press) in Serial Monitor
- [ ] Decide what slider and buttons will control (see Phase 4 ideas below)
- [ ] Add 4 indicator LEDs (pins 7, 8, 12, 13) for binary display if desired

---

### Phase 4 — Projects & Modes (Weeks 10–15)
**Goal:** Use the full hardware for interactive features.

At this point you have: 5 piezos, 5 pad LEDs, a slider, 2 buttons, and optionally 4 indicator LEDs. Here are natural next steps — pick what interests you most:

---

#### Project A — HIT mode (simple, good starting point)
Pad LEDs flash on hit. Buttons cycle MIDI channel. Slider adjusts velocity sensitivity.
- Slider left = subtle, slider right = hot
- 4 indicator LEDs show MIDI channel in binary (0000–1111)

---

#### Project B — LEARN mode (kid-friendly)
A random pad lights up. The player must hit it within a time window.
- Correct hit → score goes up (shown on indicator LEDs), new pad lights
- Wrong pad → correct one blinks as a hint
- Miss timeout → pad blinks rapidly, timer resets
- Slider controls difficulty (timeout: 1–8 seconds)
- Button 2 resets the score

---

#### Project C — FOLLOW mode (rhythm trainer)
Pads light up in a sequence locked to a BPM. Player follows the pattern.
- Slider controls tempo
- Button 1 cycles difficulty (sequence length)
- Button 2 starts/stops

---

#### Project D — FREE PLAY with effects
No game — but LEDs do something musical:
- Pad brightness = velocity (soft hit = dim, hard hit = bright)
- LEDs fade out slowly instead of cutting off (PWM fade loop)
- Slider controls fade speed

---

## Quick Reference — Final Wiring Summary

| Arduino Pin | Connected to |
|-------------|-------------|
| A0–A4 | Piezo sensors (one per pad) |
| A5 | Slider wiper |
| D2 | Button 1 |
| D4 | Button 2 |
| D3, D5, D6, D9, D10 | Pad LEDs (PWM) |
| D7, D8, D12, D13 | Indicator LEDs (optional) |
| 5V / GND | Shared power rail |

## Component Checklist

| Component | Value | Qty | Purpose |
|-----------|-------|-----|---------|
| Resistor | 1MΩ | 5 | Piezo pull-down |
| Resistor | 10kΩ | 2 | Button pull-down |
| Resistor | 150–220Ω | 9 | LED current limiting |
| Diode | 1N4148 | 5 | Piezo spike protection (optional) |
| LED | 5mm any color | 4 | Indicator LEDs (optional) |