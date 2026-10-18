#!/usr/bin/env python3
"""Exercise configured interfaces, dotenv precedence and HTTP access controls."""
import http.client
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import tempfile
import time


def main():
    app = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="orbis-network-") as temporary:
        root = Path(temporary)
        environment = {key: value for key, value in os.environ.items()
                       if not key.startswith("LAZY_STREAM_")}
        environment["XDG_CONFIG_HOME"] = str(root / "config")
        (root / ".env").write_text("LAZY_STREAM_HOST=127.0.0.3\nLAZY_STREAM_PORT=1201\n")
        explicit = root / "network.env"
        explicit.write_text('# LAN interface simulated using a separate loopback IP\n'
                            'export LAZY_STREAM_HOST="127.0.0.2"\nLAZY_STREAM_PORT=1202\n')
        with socket.socket() as available:
            available.bind(("127.0.0.2", 0))
            port = available.getsockname()[1]
        process = subprocess.Popen([app, "--headless", "--env-file", str(explicit),
                                    "--port", str(port), "--data-dir", str(root / "data")],
                                   cwd=root, env=environment, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True)
        try:
            process.stdout.readline()
            url = process.stdout.readline().strip()
            assert url.startswith(f"http://127.0.0.2:{port}/?token="), url
            token = url.split("token=")[1]

            def request(path, host=None, authorization=None, cookie=None):
                headers = {}
                if host:
                    headers["Host"] = host
                if authorization:
                    headers["Authorization"] = "Bearer " + authorization
                if cookie:
                    headers["Cookie"] = cookie
                connection = http.client.HTTPConnection("127.0.0.2", port, timeout=5)
                connection.request("GET", path, headers=headers)
                response = connection.getresponse()
                body = response.read()
                result = response.status, dict(response.getheaders()), body
                connection.close()
                return result

            for _ in range(100):
                try:
                    status, _, _ = request("/api/bootstrap")
                    break
                except ConnectionError:
                    time.sleep(.05)
            assert status == 401
            status, _, body = request("/api/bootstrap", authorization=token)
            assert status == 200 and json.loads(body)["token"] == token
            assert request("/api/state", host="attacker.example", authorization=token)[0] == 400
            assert request("/api/state", host=f"127.0.0.1:{port}", authorization=token)[0] == 400
            status, headers, _ = request("/?token=" + token)
            assert status == 303 and headers["Location"] == "/"
            cookie = headers["Set-Cookie"].split(";")[0]
            assert request("/api/state", cookie=cookie)[0] == 200
            assert request("/app.js")[0] == 200
        finally:
            process.send_signal(signal.SIGTERM)
            try:
                process.wait(timeout=8)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            assert process.returncode == 0, process.stderr.read()
        # Process exports override the file; CLI overrides both.
        environment["LAZY_STREAM_HOST"] = "127.0.0.1"
        assert subprocess.run([app, "--env-file", str(explicit), "--host", "invalid", "--headless"],
                              cwd=root, env=environment, capture_output=True).returncode != 0
        explicit.write_text("LAZY_STREAM_HOST=0.0.0.0\n")
        environment.pop("LAZY_STREAM_HOST")
        rejected = subprocess.run([app, "--env-file", str(explicit), "--headless"], cwd=root,
                                  env=environment, capture_output=True, text=True)
        assert rejected.returncode != 0 and "IP locale concreto" in rejected.stderr
        print("Dotenv, configured interface, CLI priority, session cookie, authentication and Host: OK")


if __name__ == "__main__":
    main()
