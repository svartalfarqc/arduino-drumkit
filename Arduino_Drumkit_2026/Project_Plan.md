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