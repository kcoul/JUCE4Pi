"""
install_ice.py
Writes ICE.fxp to the SurgeXT user Patches folder.
No OSC or running SurgeXT required — pure filesystem write.

Local (run on target):
    python install_ice.py --user-patch-dir "/home/user/Documents/Surge XT/Patches"

Remote (run on host, copies via SCP):
    python install_ice.py --user-patch-dir "/home/user/Documents/Surge XT/Patches" --target-ip 192.168.1.100
    python install_ice.py --user-patch-dir "/home/user/Documents/Surge XT/Patches" --target-ip user@192.168.1.100

Restart SurgeXT after running to see the patch under User Patches > QNX.
"""

import os
import getpass
import struct
import argparse

try:
    import paramiko
except ImportError:
    paramiko = None

_PREAMBLE = b'sub3W\x85' + b'\x00' * 26  # 32-byte SurgeXT chunk preamble

_XML = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>'
    '<patch revision="9">'
    '<meta name="ICE" category="QNX" comment="" author="QNX" />'
    '<parameters>'
    '<volume_FX1 type="2" value="1.000000" />'
    '<volume_FX2 type="2" value="1.000000" />'
    '<volume type="2" value="0.000000" />'
    '<scene_active type="0" value="0" />'
    '<scenemode type="0" value="0" />'
    '<splitkey type="0" value="60" />'
    '<fx_disable type="0" value="0" />'
    '<polylimit type="0" value="8" />'
    '<fx_bypass type="0" value="0" />'
    '<a_osc1_type type="0" value="1" />'
    '<a_osc1_octave type="0" value="-2" />'
    '<a_osc1_pitch type="2" value="0.000000" />'
    '<a_osc1_param0 type="2" value="0.000000" />'
    '<a_osc1_param1 type="2" value="0.500000" />'
    '<a_osc1_param2 type="2" value="0.500000" />'
    '<a_osc1_param3 type="2" value="0.000000" />'
    '<a_osc1_param4 type="2" value="0.000000" />'
    '<a_osc1_param5 type="2" value="0.200000" />'
    '<a_osc1_param6 type="0" value="1" />'
    '<a_osc1_keytrack type="0" value="1" />'
    '<a_osc1_retrigger type="0" value="0" />'
    '<a_osc2_type type="0" value="0" />'
    '<a_osc2_octave type="0" value="0" />'
    '<a_osc2_pitch type="2" value="0.000000" />'
    '<a_osc2_param0 type="2" value="0.000000" />'
    '<a_osc2_param1 type="2" value="0.500000" />'
    '<a_osc2_param2 type="2" value="0.500000" />'
    '<a_osc2_param3 type="2" value="0.000000" />'
    '<a_osc2_param4 type="2" value="0.000000" />'
    '<a_osc2_param5 type="2" value="0.200000" />'
    '<a_osc2_param6 type="0" value="1" />'
    '<a_osc2_keytrack type="0" value="1" />'
    '<a_osc2_retrigger type="0" value="0" />'
    '<a_osc3_type type="0" value="0" />'
    '<a_osc3_octave type="0" value="0" />'
    '<a_osc3_pitch type="2" value="0.000000" />'
    '<a_osc3_param0 type="2" value="0.000000" />'
    '<a_osc3_param1 type="2" value="0.500000" />'
    '<a_osc3_param2 type="2" value="0.500000" />'
    '<a_osc3_param3 type="2" value="0.000000" />'
    '<a_osc3_param4 type="2" value="0.000000" />'
    '<a_osc3_param5 type="2" value="0.200000" />'
    '<a_osc3_param6 type="0" value="1" />'
    '<a_osc3_keytrack type="0" value="1" />'
    '<a_osc3_retrigger type="0" value="0" />'
    '<a_level_o1 type="2" value="1.000000" />'
    '<a_mute_o1 type="0" value="0" />'
    '<a_solo_o1 type="0" value="0" />'
    '<a_route_o1 type="0" value="1" />'
    '<a_level_o2 type="2" value="0.500000" />'
    '<a_mute_o2 type="0" value="0" />'
    '<a_solo_o2 type="0" value="0" />'
    '<a_route_o2 type="0" value="1" />'
    '<a_level_o3 type="2" value="1.000000" />'
    '<a_mute_o3 type="0" value="1" />'
    '<a_solo_o3 type="0" value="0" />'
    '<a_route_o3 type="0" value="1" />'
    '<a_level_ring12 type="2" value="1.000000" />'
    '<a_mute_ring12 type="0" value="1" />'
    '<a_level_ring23 type="2" value="1.000000" />'
    '<a_mute_ring23 type="0" value="1" />'
    '<a_level_noise type="2" value="1.000000" />'
    '<a_mute_noise type="0" value="1" />'
    '<a_filter1_type type="0" value="0" />'
    '<a_filter1_subtype type="0" value="0" />'
    '<a_filter1_cutoff type="2" value="40.000000" />'
    '<a_filter1_resonance type="2" value="0.050000" />'
    '<a_filter1_envmod type="2" value="0.000000" />'
    '<a_filter1_keytrack type="2" value="0.000000" />'
    '<a_filter2_type type="0" value="0" />'
    '<a_filter2_subtype type="0" value="0" />'
    '<a_filter2_cutoff type="2" value="3.000000" />'
    '<a_filter2_resonance type="2" value="0.000000" />'
    '<a_env1_attack type="2" value="-8.000000" />'
    '<a_env1_attack_shape type="0" value="1" />'
    '<a_env1_decay type="2" value="-3.500000" />'
    '<a_env1_decay_shape type="0" value="0" />'
    '<a_env1_sustain type="2" value="0.000000" />'
    '<a_env1_release type="2" value="-6.000000" />'
    '<a_env1_release_shape type="0" value="2" />'
    '<a_volume type="2" value="1.000000" />'
    '<a_vca_level type="2" value="0.000000" />'
    '<a_vca_velsense type="2" value="0.000000" />'
    '</parameters>'
    '<stepsequences />'
    '<customcontroller>'
    '<entry i="0" bipolar="0" v="0.000000" label="-" />'
    '<entry i="1" bipolar="0" v="0.000000" label="-" />'
    '<entry i="2" bipolar="0" v="0.000000" label="-" />'
    '<entry i="3" bipolar="0" v="0.000000" label="-" />'
    '<entry i="4" bipolar="0" v="0.000000" label="-" />'
    '<entry i="5" bipolar="0" v="0.000000" label="-" />'
    '<entry i="6" bipolar="0" v="0.000000" label="-" />'
    '<entry i="7" bipolar="0" v="0.000000" label="-" />'
    '</customcontroller>'
    '<modwheel s0="0.000000" s1="0.000000" />'
    '</patch>'
)

def _build_fxp(patch_name: str, xml: str) -> bytes:
    xml_bytes = xml.encode('utf-8')
    chunk = _PREAMBLE + xml_bytes
    name_bytes = (patch_name.encode('utf-8') + b'\x00' * 28)[:28]
    header = (
        b'CcnK'
        + struct.pack('>I', 0)
        + b'FPCh'
        + struct.pack('>I', 1)
        + b'cjs3'
        + struct.pack('>I', 1)
        + struct.pack('>I', 1)
        + name_bytes
        + struct.pack('>I', len(chunk))
    )
    return header + chunk

def _deploy_local(fxp_bytes: bytes, user_patch_dir: str) -> None:
    out_dir = os.path.join(user_patch_dir, "QNX", "Pads")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "ICE.fxp")
    with open(out_path, 'wb') as f:
        f.write(fxp_bytes)
    print(f"Written: {out_path}")


def _deploy_remote(fxp_bytes: bytes, user_patch_dir: str, target: str) -> None:
    if paramiko is None:
        raise SystemExit("pip install paramiko  (required for remote deploy)")

    user, _, host = target.rpartition('@')
    password = getpass.getpass(f"Password for {target}: ")

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(host, username=user or None, password=password)

    remote_dir  = user_patch_dir.rstrip('/') + "/QNX/Pads"
    remote_path = remote_dir + "/ICE.fxp"

    client.exec_command(f'mkdir -p "{remote_dir}"')

    sftp = client.open_sftp()
    with sftp.open(remote_path, 'wb') as f:
        f.write(fxp_bytes)
    sftp.close()
    client.close()

    print(f"Copied to {target}:{remote_path}")


def main():
    parser = argparse.ArgumentParser(description="Install ICE.fxp to SurgeXT User Patches > QNX")
    parser.add_argument("--user-patch-dir", required=True,
                        help='SurgeXT user Patches folder on target, e.g. "/home/user/Documents/Surge XT/Patches"')
    parser.add_argument("--target-ip",
                        help='Target IP or user@host for remote deploy via SCP (omit to write locally)')
    args = parser.parse_args()

    fxp = _build_fxp("ICE", _XML)

    if args.target_ip:
        _deploy_remote(fxp, args.user_patch_dir, args.target_ip)
    else:
        _deploy_local(fxp, args.user_patch_dir)

    print("Restart SurgeXT to see it under User Patches > QNX.")

if __name__ == "__main__":
    main()
