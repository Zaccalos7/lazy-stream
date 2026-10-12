#!/usr/bin/env python3
"""Exercise the real server, real FFmpeg relay and persisted session state."""
import json
import os
import pathlib
import re
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.request

def main():
    if not shutil.which('ffmpeg') or not shutil.which('ffprobe'):
        raise RuntimeError('FFmpeg and FFprobe required for integration tests')
    with tempfile.TemporaryDirectory(prefix='orbis-linux-test-') as tmp:
        root = pathlib.Path(tmp)
        with socket.socket() as sock:
            sock.bind(('127.0.0.1', 0))
            port = sock.getsockname()[1]
        proc = subprocess.Popen([sys.argv[1], '--no-browser', '--data-dir', str(root/'data'), '--port', str(port)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            url = ''
            for _ in range(3):
                line = proc.stdout.readline().strip()
                if '?token=' in line:
                    url = line
                    break
            token = url.split('token=')[-1]
            assert len(token) == 64, 'startup token'
            base = f'http://127.0.0.1:{port}'
            def api(path, method='GET', body=None, auth=True):
                headers = {'Authorization': 'Bearer '+token} if auth else {}
                if body is not None:
                    headers['Content-Type']='application/json'
                    body=json.dumps(body).encode()
                req = urllib.request.Request(base+'/api/'+path, data=body, headers=headers, method=method)
                with urllib.request.urlopen(req, timeout=20) as r:
                    return json.load(r)
            for _ in range(60):
                try:
                    api('state')
                    break
                except (OSError, urllib.error.URLError):
                    time.sleep(.1)
            try:
                api('state', auth=False)
                raise AssertionError('unauthorized API accepted')
            except urllib.error.HTTPError as e:
                assert e.code == 401
            for resource, marker in [('/', b'OrbisStream'),('/app.js',b'bootstrap'),('/resources/ui/ui.it.json',b'startLive'),('/pages/MainMenu.html',b'Pages/MainMenu.cshtml'),('/pages/MainLayoutList.html',b'Pages/MainLayoutList.cshtml'),('/pages/MainPreview.html',b'Pages/MainPreview.cshtml'),('/pages/MainChannelSetting.html',b'Pages/MainChannelSetting.cshtml'),('/pages/MainVideoSetting.html',b'Pages/MainVideoSetting.cshtml')]:
                with urllib.request.urlopen(urllib.request.Request(base+resource,headers={'Authorization':'Bearer '+token}),timeout=5) as r:
                    assert marker in r.read(), 'installed web resource: '+resource
            p = {'title':'Test','videoWidth':320,'videoHeight':180,'frameRate':25,'videoBitrate':500,'audioBitrate':64,'gopSize':1,'preset':'ultrafast','lowLatency':True}
            profile=api('profiles','POST',p)['id']
            clip=root/'clip.mp4'
            subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i','testsrc2=size=320x180:rate=25','-f','lavfi','-i','sine=frequency=440:sample_rate=48000','-t','2','-c:v','libx264','-preset','ultrafast','-c:a','aac',str(clip)],check=True,timeout=20)
            # CheckSlots/CheckLayout devono valere anche per richieste HTTP senza editor.
            layout={'name':'Layout API','isLayout':True,'width':320,'height':180,'items':[]}
            try:
                api('scenes','POST',layout)
                raise AssertionError('empty layout accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400 and 'almeno uno slot' in e.read().decode()
            slot={'sourceKind':'FILE','sourceTarget':'/not/a/real/file.mp4','x':0,'y':0,'width':320,'height':180}
            layout['items']=[slot]
            layout_id=api('scenes','POST',layout)['id']
            assert api(f'scenes/{layout_id}')['items'][0]['sourceTarget']=='', 'slot target cleared'
            invalid={**layout,'items':[{**slot,'x':1}]}
            try:
                api('scenes','POST',invalid)
                raise AssertionError('outside-canvas slot accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400 and 'canvas' in e.read().decode()
            scene={**layout,'name':'Scena API','isLayout':False,'items':[{**slot,'sourceTarget':' "'+str(clip)+'" '} ]}
            scene_id=api(f'scenes/{layout_id}','PUT',scene)['id']
            assert scene_id!=layout_id, 'layout converted in place'
            assert api(f'scenes/{scene_id}')['items'][0]['sourceTarget']==str(clip), 'file path normalization'
            assert api(f'scenes/{layout_id}')['isLayout'], 'original layout preserved'
            duplicated={**scene,'items':scene['items']*2}
            try:
                api('scenes','POST',duplicated)
                raise AssertionError('duplicate scene source accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400 and 'duplicata' in e.read().decode()
            # platformStreamName identifica la piattaforma su Windows; il nome del canale apre il player.
            twitch=api('channels','POST',{'channelName':'examplechannel','platform':'Twitch','platformStreamName':'twitch','streamUrl':'rtmp://live.twitch.tv/app','streamKey':'test-key'})['id']
            player=api(f'platform/{twitch}')
            assert player['publicUrl']=='https://www.twitch.tv/examplechannel', player
            # I primi database Linux usavano platformStreamName come handle: restano leggibili.
            legacy=api('channels','POST',{'channelName':'Etichetta','platform':'Twitch','platformStreamName':'oldhandle','streamUrl':'rtmp://live.twitch.tv/app','streamKey':'test-key'})['id']
            assert api(f'platform/{legacy}')['publicUrl']=='https://www.twitch.tv/oldhandle'
            # A TikTok ingest is temporary even if the channel is labelled Custom.
            temporary=api('channels','POST',{'channelName':'Temporary key','platform':'Custom','streamUrl':'rtmp://push.tiktokcdn.com/live','streamKey':'do-not-store','profileId':profile})['id']
            saved=api('channels/'+str(temporary))
            assert saved['streamKey']=='asked-at-every-live:Temporary key', saved
            try:
                api('live/start','POST',{'channelId':temporary,'files':[str(clip)]})
                raise AssertionError('TikTok live accepted a missing fresh key')
            except urllib.error.HTTPError as e:
                assert e.code==400 and 'Chiave richiesta' in e.read().decode()
            output=root/'output.flv'
            video=api('videos','POST',{'name':'Clip','videoPath':str(clip),'enabled':True})['id']
            with urllib.request.urlopen(urllib.request.Request(base+f'/api/video/{video}/play',headers={'Authorization':'Bearer '+token,'Range':'bytes=0-99'})) as r:
                assert r.status==206 and len(r.read())==100, 'range playback'
            proxy=api('proxy','POST',{'target':str(clip)})
            for _ in range(100):
                jobs=api('proxy')
                job=next(j for j in jobs if j['name']==proxy['name'])
                if job['status'] in ('READY','ERROR'):
                    break
                time.sleep(.1)
            assert job['status']=='READY',job
            tuned=api('tune','POST',p)
            assert tuned['preset'] in ('fast','veryfast','ultrafast') and tuned['speed']>0
            with urllib.request.urlopen(urllib.request.Request(base+'/api/snapshot?kind=FILE&target='+str(clip),headers={'Authorization':'Bearer '+token})) as r:
                assert r.headers['Content-Type']=='image/jpeg'
                assert r.read().startswith(b'\xff\xd8'), 'source thumbnail'
            selected=api(f'profiles/{profile}')
            api(f'profiles/{profile}','PUT',{**selected,'isActive':True})
            channel=api('channels','POST',{'channelName':'Active profile','platform':'Custom','streamUrl':'rtmp://127.0.0.1/live','streamKey':'test-key','profileId':0})['id']
            live=api('live/start','POST',{'channelId':channel,'files':[str(clip),str(clip)],'loop':False,'outputFile':str(output)})['id']
            states=[]
            for _ in range(160):
                states=api('live')
                current=next(s for s in states if s['id']==live)
                if current['status'] in ('ENDED','ERROR'):
                    break
                time.sleep(.1)
            assert current['status']=='ENDED', current
            info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-show_format','-of','json',str(output)],text=True))
            assert {s['codec_type'] for s in info['streams']}=={'video','audio'}, info
            assert 3.5 < float(info['format']['duration']) < 5.5, info['format']
            packets=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_packets','-of','json',str(output)],text=True))['packets']
            for stream in (0,1):
                times=[float(p['dts_time']) for p in packets if p['stream_index']==stream and 'dts_time' in p]
                assert times==sorted(times), 'timestamps regressed'
            history=api('history/'+str(live))
            assert history['status']=='ENDED'
            assert history['request']['profile']['id']==profile, 'active profile selected'
            assert 'streamKey' not in json.dumps(api('export')) or 'secret' not in json.dumps(api('export'))
            # Mixed mute, takeover and resume go through the persistent publisher.
            output2=root/'switch.flv'
            live2=api('live/start','POST',{'files':[str(clip)],'profileId':profile,'loop':True,'outputFile':str(output2)})['id']
            time.sleep(.6)
            api(f'live/{live2}/volume','POST',{'index':0,'volume':30,'audioEnabled':True})
            api(f'live/{live2}/takeover','POST',{'scene':{'name':'Takeover','items':[{'sourceKind':'FILE','sourceTarget':str(clip),'audioEnabled':True}]},'durationSeconds':1})
            time.sleep(1.3)
            api(f'live/{live2}/resume','POST',{})
            time.sleep(.8)
            api(f'live/{live2}/stop','POST',{})
            assert api('history/'+str(live2))['status']=='ENDED'
            assert output2.stat().st_size>5000
            # Publish through a real RTMP connection to a local FFmpeg ingest.
            with socket.socket() as sock:
                sock.bind(('127.0.0.1',0))
                ingest_port=sock.getsockname()[1]
            recorded=root/'rtmp.flv'
            ingest=subprocess.Popen(['ffmpeg','-v','error','-y','-listen','1','-i',f'rtmp://127.0.0.1:{ingest_port}/live/test-key','-c','copy','-f','flv',str(recorded)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            try:
                time.sleep(.3)
                channel=api('channels','POST',{'channelName':'RTMP test','streamUrl':f'rtmp://127.0.0.1:{ingest_port}/live','streamKey':'test-key','isActive':True,'profileId':profile})['id']
                live3=api('live/start','POST',{'channelId':channel,'files':[str(clip),str(clip)],'loop':False})['id']
                for _ in range(160):
                    current=next(s for s in api('live') if s['id']==live3)
                    if current['status'] in ('ENDED','ERROR'):
                        break
                    time.sleep(.1)
                assert current['status']=='ENDED', current
                ingest.wait(timeout=8)
                assert recorded.stat().st_size>10000
                streams=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(recorded)],text=True))['streams']
                assert {s['codec_type'] for s in streams}=={'audio','video'}
            finally:
                if ingest.poll() is None:
                    ingest.terminate()
                    ingest.wait(timeout=3)
            print('HTTP authentication, persistence, FFmpeg playlist, continuous FLV timestamps, mixer, takeover, resume, stop: OK')
        finally:
            proc.send_signal(signal.SIGTERM)
            try:
                proc.wait(timeout=8)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
            if proc.returncode not in (0,-signal.SIGTERM):
                print(proc.stderr.read(),file=sys.stderr)
if __name__=='__main__':
    main()
