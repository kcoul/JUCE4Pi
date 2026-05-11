# TX/RX Part 2: AI Audio on the Raspberry Pi
## Presentation Outline with Time Allotments
### Audio Developer Conference Japan 2026

---

| # | Section | Time | Running total |
|---|---|---|---|
| — | Title slide + Agenda | ~2 min | 2 min |
| 1 | The Story So Far | ~6 min | 8 min |
| 2 | JUCE on QNX | ~8 min | 16 min |
| 3 | Audio on the NPU | ~5 min | 21 min |
| 4 | Developing Host-First | ~9 min | 30 min |
| 5 | Getting to the Target | ~12 min | 42 min |
| — | The Punchline (no section card — flows from Sec. 5) | ~3 min | 45 min |
| 6 | Lessons & Next Steps | ~3 min | 48 min |
| — | Q&A buffer | ~4 min | ~52 min |

---

## Narrative framing

Two layers of motivation run through the talk:

**Primary (serious):** JUCE on QNX is a real achievement that matters for the QNX Everywhere
initiative. The rest of the work demonstrates what that foundation enables.

**Secondary (retrospective/fun):** The joke — all this engineering just to browse synth patches
hands-free — is the accessible wrapper that gives the technical content a relatable context.
It lands at the end of Section 5 as the emotional payoff, not as the stated goal.

---

## Section Detail

### Section 1 — The Story So Far (~6 min)
- ADC'21 recap: JUCE on Pi, IDE-to-filesystem trick (avoids cross-compile),
  small demo targets (ADC21/ folder)
- ADCx'23 Part 1 recap: host-side AI pipeline (Stem Separation + Audio-to-MIDI),
  the host-first philosophy
- Key message: "I barely figured out how to get code on a Pi — and I was avoiding the hard parts"

### Section 2 — JUCE on QNX (~8 min)
- Why JUCE on QNX is a big deal: QNX Everywhere initiative
- What was non-trivial about the port (JUCE + Tracktion Engine)
- SurgeXT as the real-world stress test (tiny ADC21 targets weren't sufficient)
- SurgeXT running on QNX: Video clip 1
- The MIDI gap: QNX has no MIDI support → SurgeMidiToOscBridge
- Signal chain diagram 1 (development workflow: host/target split)

### Section 3 — Audio on the NPU (~5 min)
- What an NPU is and why it matters for audio inference workloads
- Hailo 10-H in M.2 2280 NVMe form factor — same chip, 3 contexts:
    1. Windows 11 dev laptop
    2. Ubuntu dev laptop
    3. RPi5 via M.2 hat + PCIe
- NPU vendor comparison slide: Hailo vs. Qualcomm
  (Hailo dev accessibility notably better than Qualcomm's)

### Section 4 — Developing Host-First (~9 min)
- Reinforces the philosophy from Section 1 with a concrete example
- Model: Whisper STT (Hailo-supported out of the box)
- whisper.cpp testbench on host CPU first
- Voice → Program + Bank Change MIDI → drives SurgeXT patch library
- Video clip 2: testbench on host CPU
- CPU latency baseline

### Section 5 — Getting to the Target (~12 min)
- Cross-compilation: 2×2 grid (QNX / Ubuntu Linux × Windows / WSL2)
  - QNX SDP: clean, works from Windows or Linux
  - Ubuntu aarch64: WSL2 needed; apt multiarch tricks (DEB822 Architectures: field)
- Same Whisper workload via Hailo API on host NPU first
- Video clip 3: before/after — 10× speed result on host NPU
- Move workload to RPi5 NPU (same Hailo API, different device)
- Signal chain diagram 2 (full target — everything on RPi5)
- [Punchline flows here, no section card]
  "All of this... just so I could browse patches while playing with both hands."
- Video clip 4: the full demo

### Section 6 — Lessons & Next Steps (~3 min)
- Three honest things harder than expected
- Directions this opens up
- Call to action

---

## Notes
- No live demos — all demonstrations are short embedded MP4 clips (H.264)
- Visual aid rule: every claim backed by a diagram, screenshot, or video clip
  (ADC Japan is the first-ever ADC in Japan — mixed-language audience, slides carry extra weight)
- Punchline video must come AFTER the spoken joke, never before
- "Section" terminology used throughout (not "Chapter")
