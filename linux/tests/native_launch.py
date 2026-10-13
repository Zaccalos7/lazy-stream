#!/usr/bin/env python3
"""Check the installed application's default GUI mode and graceful shutdown."""
import os
import pathlib
import signal
import socket
import subprocess
import sys
import tempfile
import time


def verify(app: pathlib.Path) -> None:
    """Launch real Qt widgets and verify the normal mode does not start a web server."""
    with tempfile.TemporaryDirectory(prefix="orbis-native-launch-") as temporary:
        with socket.socket() as socket_:
            socket_.bind(("127.0.0.1", 0))
            port = socket_.getsockname()[1]
        environment = dict(os.environ, QT_QPA_PLATFORM="offscreen")
        process = subprocess.Popen(
            [str(app), "--data-dir", temporary, "--port", str(port)],
            env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        try:
            database = pathlib.Path(temporary) / "stream.db"
            deadline = time.monotonic() + 10
            while not database.exists() and process.poll() is None and time.monotonic() < deadline:
                time.sleep(0.05)
            time.sleep(0.4)
            assert process.poll() is None, "Native application exited during startup"
            assert database.exists(), "Native application did not initialize its store"
            with socket.socket() as socket_:
                socket_.settimeout(0.2)
                assert socket_.connect_ex(("127.0.0.1", port)) != 0, "GUI mode opened an HTTP port"
            process.send_signal(signal.SIGTERM)
            stdout, stderr = process.communicate(timeout=12)
            assert process.returncode == 0, stderr
            assert "http://" not in stdout and "token=" not in stdout, "GUI mode started browser bootstrap"
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()
    print("Native Qt launch, no HTTP server, graceful shutdown: OK")


if __name__ == "__main__":
    verify(pathlib.Path(sys.argv[1]).resolve())
