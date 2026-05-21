"""
init_demo_patches.py
--------------------
Configures two SurgeXT user patches for the GENISYS AVAS/ESE demo:
  GENISYS/ESE_Normal  -- continuous EV motor drone
  GENISYS/SonicBoom   -- percussive one-shot boom triggered by note-on

Run with SurgeXT open and OSC enabled (default port 53280).

Requires: pip install python-osc

SurgeXT OSC parameter convention (from source):
  vt_int params (osctype, octave, etc.)  -> send raw int cast to float: 0.0, 1.0, 2.0 ...
  vt_float params (pitch, cutoff, etc.)  -> send normalized 0-1 float

Key float-param normalization:
  osc/pitch    (ct_pitch_semi7bp):   range -7..7,   norm = (val+7)/14
  filter/cutoff (ct_freq_audible):   range -60..70, norm = (val+60)/130
  filter/res   (ct_percent):        range  0..1,    direct
  aeg/*        (ct_envtime):        range -8..5,    norm = (val+8)/13
  mixer/*/vol  (ct_amplitude):      range  0..1,    direct
  amp/volume   (ct_amplitude_clip): range  0..1,    direct
"""

import time
import argparse
from pythonosc import udp_client

SURGE_HOST = "127.0.0.1"
SURGE_PORT = 53280
INTER_MSG_DELAY  = 0.02   # seconds between individual param messages
SAVE_DELAY       = 0.30   # seconds before saving (let SurgeXT process all params)


def norm_pitch(semitones):
    """ct_pitch_semi7bp: -7..7 -> 0..1"""
    return (semitones + 7) / 14.0


def norm_cutoff(semitones):
    """ct_freq_audible: -60..70 semitones -> 0..1  (0 semitones ≈ 523 Hz)"""
    return (semitones + 60) / 130.0


def norm_envtime(val):
    """ct_envtime: -8..5 (log-seconds) -> 0..1.  val=-8≈4ms, val=0≈1s, val=5≈32s"""
    return (val + 8) / 13.0


def send_params(client, params):
    """Send a list of (address, value) tuples with small inter-message delays."""
    for addr, val in params:
        client.send_message(addr, float(val))
        time.sleep(INTER_MSG_DELAY)


# ---------------------------------------------------------------------------
# Patch definitions
# ---------------------------------------------------------------------------

def params_ese_normal():
    """
    ESE Normal -- EV motor whine that responds to CC1 (pitch) and CC11 (cutoff).
    Sine osc for clean electric fundamental + Classic osc one octave up for harmonic body.
    Long sustain: sound lives as long as the note is held.
    """
    return [
        # Osc 1: Sine (type=1, vt_int -> raw int)
        ("/param/a/osc/1/type",           1.0),       # Sine
        ("/param/a/osc/1/octave",         0.0),       # no octave shift
        ("/param/a/osc/1/pitch",          norm_pitch(0)),

        # Osc 2: Classic (type=0), one octave up, lower volume
        ("/param/a/osc/2/type",           0.0),       # Classic
        ("/param/a/osc/2/octave",         1.0),       # +1 octave
        ("/param/a/osc/2/pitch",          norm_pitch(0)),

        # Osc 3: off
        ("/param/a/mixer/osc3/mute",      1.0),

        # Mixer
        ("/param/a/mixer/osc1/volume",    0.90),
        ("/param/a/mixer/osc2/volume",    0.25),

        # Filter 1: gentle low-pass, moderate cutoff, light resonance
        ("/param/a/filter/1/cutoff",      norm_cutoff(5)),    # ~600 Hz
        ("/param/a/filter/1/resonance",   0.20),

        # Amp envelope: instant attack, full sustain (continuous drone)
        ("/param/a/aeg/attack",           norm_envtime(-8)),   # ~0 ms
        ("/param/a/aeg/decay",            norm_envtime(-3)),   # short decay
        ("/param/a/aeg/sustain",          1.0),               # FULL sustain
        ("/param/a/aeg/release",          norm_envtime(-5)),   # quick release

        # Scene volume
        ("/param/a/amp/volume",           0.90),
    ]


def params_sonic_boom():
    """
    Sonic Boom -- percussive one-shot: sub-bass thump + harmonic crack.
    Zero sustain so the sound decays completely on each note trigger.
    """
    return [
        # Osc 1: Sine, two octaves down (deep sub-bass boom)
        ("/param/a/osc/1/type",           1.0),       # Sine
        ("/param/a/osc/1/octave",        -2.0),       # -2 octaves (vt_int raw)
        ("/param/a/osc/1/pitch",          norm_pitch(0)),

        # Osc 2: Classic, normal octave (harmonic crack content)
        ("/param/a/osc/2/type",           0.0),       # Classic
        ("/param/a/osc/2/octave",         0.0),
        ("/param/a/osc/2/pitch",          norm_pitch(0)),

        # Osc 3: off
        ("/param/a/mixer/osc3/mute",      1.0),

        # Mixer: emphasise the sub thump
        ("/param/a/mixer/osc1/volume",    1.00),
        ("/param/a/mixer/osc2/volume",    0.50),

        # Filter: open enough to let the sub through
        ("/param/a/filter/1/cutoff",      norm_cutoff(40)),   # fairly open
        ("/param/a/filter/1/resonance",   0.05),

        # Amp envelope: instant attack, short decay, ZERO sustain (one-shot)
        ("/param/a/aeg/attack",           norm_envtime(-8)),   # ~0 ms
        ("/param/a/aeg/decay",            norm_envtime(-3.5)), # ~150-300 ms
        ("/param/a/aeg/sustain",          0.0),               # NO sustain
        ("/param/a/aeg/release",          norm_envtime(-6)),   # very quick

        # Scene volume: maximum for impact
        ("/param/a/amp/volume",           1.00),
    ]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Initialise GENISYS demo patches in SurgeXT")
    parser.add_argument("--host", default=SURGE_HOST)
    parser.add_argument("--port", type=int, default=SURGE_PORT)
    parser.add_argument("--patch", choices=["all", "ese", "boom"], default="all",
                        help="Which patch(es) to initialise")
    args = parser.parse_args()

    client = udp_client.SimpleUDPClient(args.host, args.port)
    print(f"Sending OSC to SurgeXT at {args.host}:{args.port}\n")

    if args.patch in ("all", "ese"):
        print("Configuring ESE_Normal...")
        send_params(client, params_ese_normal())
        time.sleep(SAVE_DELAY)
        client.send_message("/patch/save_user", "GENISYS/ESE_Normal")
        print("  -> saved as GENISYS/ESE_Normal\n")
        time.sleep(SAVE_DELAY)

    if args.patch in ("all", "boom"):
        print("Configuring SonicBoom...")
        send_params(client, params_sonic_boom())
        time.sleep(SAVE_DELAY)
        client.send_message("/patch/save_user", "GENISYS/SonicBoom")
        print("  -> saved as GENISYS/SonicBoom\n")

    print("Done. Reload in SurgeXT if needed (File > Open Patch Bank > User).")


if __name__ == "__main__":
    main()
