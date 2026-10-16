#!/usr/bin/env python3
"""FFmpeg 9: decode every frame across producers, both transports, and animated alpha overlays."""
import json
import os
from pathlib import Path
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
import urllib.request
import zlib


def main():
    app = Path(sys.argv[1]).resolve()
    ffmpeg = os.environ.get('FFMPEG_PATH', str(app.parent/'ffmpeg-9.0.2/bin/ffmpeg'))
    ffprobe = os.environ.get('FFPROBE_PATH', str(Path(ffmpeg).with_name('ffprobe')))
    def run(*args):
        return subprocess.check_output([str(a) for a in args], stderr=subprocess.PIPE, timeout=20)
    assert run(ffmpeg, '-version').startswith(b'ffmpeg version 9.0.2'), 'Test requires bundled FFmpeg 9.0.2'
    with tempfile.TemporaryDirectory(prefix='orbis-delivery-') as tmp:
        root = Path(tmp)
        with socket.socket() as available:
            available.bind(('127.0.0.1',0))
            port = available.getsockname()[1]
        process = subprocess.Popen([str(app), '--headless', '--data-dir', str(root/'data'), '--port', str(port)],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            for _ in range(3):
                line = process.stdout.readline().strip()
                if '?token=' in line:
                    token = line.split('?token=')[1]
                    break
            def api(path, body=None):
                request = urllib.request.Request(f'http://127.0.0.1:{port}/api/{path}',
                    data=None if body is None else json.dumps(body).encode(),
                    headers={'Authorization':'Bearer '+token,'Content-Type':'application/json'})
                with urllib.request.urlopen(request, timeout=20) as response:
                    return json.load(response)
            def completed(request):
                live = api('live/start', request)['id']
                for _ in range(160):
                    snapshot = next(item for item in api('live') if item['id'] == live)
                    if snapshot['status'] in ('ENDED','ERROR'): break
                    time.sleep(.1)
                assert snapshot['status'] == 'ENDED', snapshot
                return snapshot
            first, second = root/'silent.mkv', root/'sound.mkv'
            run(ffmpeg,'-v','error','-f','lavfi','-i','color=c=blue:s=320x180:r=15',
                '-t','2','-c:v','ffv1',first)
            run(ffmpeg,'-v','error','-f','lavfi','-i','testsrc2=s=640x360:r=30','-f','lavfi','-i','sine=r=44100',
                '-t','2','-c:v','libx264','-preset','ultrafast','-c:a','aac',second)
            profile = {'title':'Delivery','videoWidth':320,'videoHeight':180,'frameRate':15,
                       'videoBitrate':500,'audioBitrate':64,'preset':'ultrafast','lowLatency':True}
            for transport in ('native','ffmpeg'):
                output = root/(transport+'.flv')
                snapshot = completed({'files':[str(first),str(second)],'profile':profile,'platform':'YouTube',
                    'transport':transport,'outputFile':str(output),'reconnect':False})
                expected = 'native-libavformat' if transport == 'native' else 'ffmpeg-sender'
                assert snapshot['delivery']['transport'] == expected, snapshot
                run(ffmpeg,'-v','error','-xerror','-i',output,'-map','0:v','-map','0:a','-f','null','-')
                info = json.loads(run(ffprobe,'-v','error','-count_frames','-show_streams','-of','json',output))
                video = next(stream for stream in info['streams'] if stream['codec_type'] == 'video')
                audio = next(stream for stream in info['streams'] if stream['codec_type'] == 'audio')
                assert (video['width'],video['height']) == (320,180) and 58 <= int(video['nb_read_frames']) <= 62, video
                assert audio['sample_rate'] == '48000' and audio['channels'] == 2, audio
                packets = json.loads(run(ffprobe,'-v','error','-show_packets','-of','json',output))['packets']
                for stream in (0,1):
                    times = [float(p['dts_time']) for p in packets if p['stream_index'] == stream and 'dts_time' in p]
                    assert times == sorted(times), 'DTS regressed at producer boundary'
            def png(path, rgb):
                def chunk(kind, value):
                    return struct.pack('>I',len(value))+kind+value+struct.pack('>I',zlib.crc32(kind+value))
                row = bytes((*rgb,255))*32 + bytes((0,0,0,0))*32
                path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',64,32,8,6,0,0,0))+
                    chunk(b'IDAT',zlib.compress((b'\0'+row)*32))+chunk(b'IEND',b''))
            png(root/'frame-0.png',(255,0,0)); png(root/'frame-1.png',(0,255,0))
            webp = root/'data/overlays/animated.webp'
            run(ffmpeg,'-v','error','-framerate','2','-i',root/'frame-%d.png','-frames:v','2',
                '-c:v','libwebp_anim','-lossless','1','-loop','1',webp)
            original = webp.read_bytes()
            background = root/'background.mkv'
            run(ffmpeg,'-v','error','-f','lavfi','-i','color=c=blue:s=64x32:r=8','-t','3','-c:v','ffv1',background)
            for encoder, name, options in [
                ('libx265','hevc',[{'key':'x265-params','value':'log-level=error:pools=1:frame-threads=1'}]),
                ('libvpx-vp9','vp9',[{'key':'deadline','value':'realtime'},{'key':'cpu-used','value':'8'}]),
                ('libaom-av1','av1',[{'key':'usage','value':'realtime'},{'key':'cpu-used','value':'8'}]),
            ]:
                codec_output=root/(name+'.flv')
                completed({'files':[str(background),str(background)],'profile':{**profile,'videoWidth':64,'videoHeight':32,
                    'frameRate':8,'videoCodecName':encoder,'videoSettingsOptions':options},
                    'outputFile':str(codec_output),'reconnect':False})
                run(ffmpeg,'-v','error','-xerror','-i',codec_output,'-map','0:v','-map','0:a','-f','null','-')
                codecs=json.loads(run(ffprobe,'-v','error','-count_frames','-show_streams','-of','json',codec_output))['streams']
                video=next(stream for stream in codecs if stream['codec_type']=='video')
                assert video['codec_name']==name and int(video['nb_read_frames'])==48, codecs
            output = root/'alpha.flv'
            scene = {'name':'Animated alpha','width':64,'height':32,'items':[
                {'sourceKind':kind,'sourceTarget':str(path),'x':0,'y':0,'width':64,'height':32,'audioEnabled':False}
                for kind,path in [('FILE',background),('OVERLAY',webp)]]}
            completed({'scene':scene,'profile':{**profile,'videoWidth':64,'videoHeight':32,'frameRate':8},
                       'outputFile':str(output),'reconnect':False})
            rgb = run(ffmpeg,'-v','error','-i',output,'-an','-pix_fmt','rgb24','-f','rawvideo','-')
            assert len(rgb) >= 24*64*32*3, len(rgb)
            def pixel(frame,x):
                offset = (frame*64*32+16*64+x)*3
                return rgb[offset:offset+3]
            for frame in (1,9,17):
                color = pixel(frame,16)
                assert color[0]>180 and color[1]<60, ('red animation loop',frame,list(color))
            for frame in (5,13,21):
                color = pixel(frame,16)
                assert color[1]>180 and color[0]<60, ('green animation loop',frame,list(color))
                color = pixel(frame,48)
                assert color[2]>180 and color[0]<60, ('transparent background',frame,list(color))
            assert webp.read_bytes() == original, 'Original animated WebP was modified'
            print('Both transports: complete decoded audio/video, uniform format, silent source, DTS continuity; HEVC/VP9/AV1; FFmpeg 9 WebP animation/loops/alpha: OK')
        finally:
            process.send_signal(signal.SIGTERM)
            try: process.wait(timeout=8)
            except subprocess.TimeoutExpired: process.kill(); process.wait()

if __name__ == '__main__':
    main()
