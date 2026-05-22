"""
deploy.py
Deploys the VREngine binary (built by build.sh) to the target via SFTP.

    python deploy.py --target-ip 192.168.1.100
    python deploy.py --target-ip user@192.168.1.100 --deploy-path /opt/vrengine
"""

import os
import getpass
import argparse

try:
    import paramiko
except ImportError:
    raise SystemExit("pip install paramiko")

_BINARY = os.path.join(os.path.dirname(__file__), "build", "aarch64-linux", "VREngine")

def main():
    parser = argparse.ArgumentParser(description="Deploy VREngine to RPi5 target")
    parser.add_argument("--target-ip", required=True,
                        help="Target as user@host or host")
    parser.add_argument("--deploy-path", default="~",
                        help='Destination directory on target (default: ~/)')
    args = parser.parse_args()

    if not os.path.exists(_BINARY):
        raise SystemExit(f"Binary not found: {_BINARY}\nRun build.sh first.")

    user, _, host = args.target_ip.rpartition('@')
    password = getpass.getpass(f"Password for {args.target_ip}: ")

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(host, username=user or None, password=password)

    deploy_dir = args.deploy_path
    if deploy_dir == '~':
        _, stdout, _ = client.exec_command('echo $HOME')
        deploy_dir = stdout.read().decode().strip()

    remote_path = deploy_dir.rstrip('/') + '/VREngine'

    client.exec_command(f'mkdir -p "{deploy_dir}"')

    sftp = client.open_sftp()
    sftp.put(_BINARY, remote_path)
    sftp.chmod(remote_path, 0o755)
    sftp.close()
    client.close()

    print(f"Deployed to {args.target_ip}:{remote_path}")
    print(f"Run it from a terminal on the target: {remote_path}")

if __name__ == "__main__":
    main()
