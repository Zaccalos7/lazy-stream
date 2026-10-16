#!/usr/bin/env python3
"""Exercise the real server, real FFmpeg relay and persisted session state."""
import json
import io
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
        # Un aggiornamento rimuove l'inode eseguito: i successivi FFprobe/FFmpeg
        # devono ancora trovare il runner nella directory dell'app.
        binary = pathlib.Path(sys.argv[1]).resolve()
        running_binary = root/'orbis-stream'
        shutil.copy2(binary, running_binary)
        (root/'orbis-stream-runner').symlink_to(binary.with_name('orbis-stream-runner'))
        resources = binary.parent.parent/'share/orbis-stream/web'
        if not resources.is_dir():
            resources = pathlib.Path(__file__).resolve().parent.parent/'web'
        environment = dict(os.environ)
        # This test removes a copied executable inode. Preserve the SDK/runtime belonging to the
        # original executable, including when exercising an extracted release package.
        for sdk in [binary.parent.parent/'libexec/orbis-stream/ffmpeg', binary.parent/'ffmpeg-9.0.2/bin']:
            if (sdk/'ffmpeg').is_file():
                environment['FFMPEG_PATH'] = str(sdk/'ffmpeg')
                environment['FFPROBE_PATH'] = str(sdk/'ffprobe')
                break
        library = binary.parent.parent/'lib/orbis-stream/ffmpeg'
        if library.is_dir():
            environment['LD_LIBRARY_PATH'] = str(library)+':'+environment.get('LD_LIBRARY_PATH','')
        proc = subprocess.Popen([str(running_binary), '--no-browser', '--data-dir', str(root/'data'), '--port', str(port), '--web-root', str(resources)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=environment)
        try:
            url = ''
            for _ in range(3):
                line = proc.stdout.readline().strip()
                if '?token=' in line:
                    url = line
                    break
            token = url.split('token=')[-1]
            assert len(token) == 64, 'startup token'
            running_binary.unlink()
            base = f'http://127.0.0.1:{port}'
            def api(path, method='GET', body=None, auth=True):
                headers = {'Authorization': 'Bearer '+token} if auth else {}
                if body is not None:
                    headers['Content-Type']='application/json'
                    body=json.dumps(body).encode()
                req = urllib.request.Request(base+'/api/'+path, data=body, headers=headers, method=method)
                try:
                    with urllib.request.urlopen(req, timeout=20) as r:
                        return json.load(r)
                except urllib.error.HTTPError as error:
                    # Keep the original HTTPError for rejection tests, but expose the backend reason on failures.
                    payload = error.read()
                    diagnostic = urllib.error.HTTPError(error.url, error.code, error.msg, error.headers, io.BytesIO(payload))
                    diagnostic.add_note(payload.decode(errors='replace'))
                    raise diagnostic from None
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
            for resource, marker in [('/', b'OrbisStream'),('/app.js',b'bootstrap'),('/resources/ui/ui.it.json',b'startLive'),('/pages/MainMenu.html',b'Pages/MainMenu.cshtml'),('/pages/MainLayoutList.html',b'Pages/MainLayoutList.cshtml'),('/pages/MainPreview.html',b'Pages/MainPreview.cshtml'),('/pages/MainChannelSetting.html',b'Pages/MainChannelSetting.cshtml'),('/pages/MainVideoSetting.html',b'Pages/MainVideoSetting.cshtml'),('/pages/MainLiveHistory.html',b'Pages/MainLiveHistory.cshtml')]:
                with urllib.request.urlopen(urllib.request.Request(base+resource,headers={'Authorization':'Bearer '+token}),timeout=5) as r:
                    assert marker in r.read(), 'installed web resource: '+resource
            for page_name in ['MainLive','_LiveRows','_StartWizardSetting','_StartWizardConfiguration']:
                with urllib.request.urlopen(urllib.request.Request(base+f'/pages/{page_name}.html',headers={'Authorization':'Bearer '+token}),timeout=5) as r:
                    assert f'Pages/{page_name}.cshtml'.encode() in r.read(), 'installed live wizard resource: '+page_name
            defaults = api('profiles')
            assert len(defaults) == 8 and all(p['isActive'] for p in defaults), defaults
            expected_defaults = json.loads(pathlib.Path(__file__).with_name('windows-defaults.json').read_text())
            for expected in expected_defaults:
                actual = next(p for p in defaults if p['platform']==expected['platform'] and p['isDefaultConfiguration']==expected['isDefaultConfiguration'])
                for key in ('videoBitrate','audioBitrate','videoWidth','videoHeight','frameRate','preset'):
                    assert actual.get(key)==expected[key], (expected['platform'],key,actual,expected)
                options={o['key']:o['value'] for o in actual['videoSettingsOptions']}
                assert options['profile:v']==expected['profile'] and options.get('x264-params')==expected['x264-params'], (actual,expected)
            for default in defaults:
                api('profiles/'+str(default['id']), 'PUT', {**default, 'isActive':False})
            p = {'title':'Test','videoWidth':320,'videoHeight':180,'frameRate':25,'videoBitrate':500,'audioBitrate':64,'gopSize':1,'preset':'ultrafast','lowLatency':True}
            with urllib.request.urlopen(urllib.request.Request(base+'/updates?watch=rows', headers={'Authorization':'Bearer '+token}), timeout=3) as updates:
                assert updates.headers['Content-Type']=='text/event-stream'
                def next_change():
                    for _ in range(20):
                        line=updates.readline().decode().strip()
                        if line.startswith('data: '): return int(line[6:])
                    raise AssertionError('No change notification')
                before=next_change()
                profile=api('profiles','POST',p)['id']
                assert next_change()>before, 'SSE did not wake on the saved profile'

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
            # La pagina Storico salva solo la pulizia della prima configurazione attiva.
            api('history/cleanup/settings','POST',{'enabled':True,'intervalMonths':3,'olderThanMonths':0})
            cleanup_channel=api(f'channels/{twitch}')
            assert cleanup_channel['streamKey']=='test-key' and cleanup_channel['streamUrl']=='rtmp://live.twitch.tv/app'
            assert cleanup_channel['autoCleanupEnabled'] and cleanup_channel['autoCleanupOlderThanMonths']==0
            api('history/cleanup/settings','POST',{'enabled':False,'intervalMonths':1,'olderThanMonths':24})
            disabled=api(f'channels/{twitch}')
            assert not disabled['autoCleanupEnabled'] and disabled['autoCleanupIntervalMonths']==3 and disabled['autoCleanupOlderThanMonths']==0
            try:
                api('history/cleanup','POST',{'months':-2,'async':True})
                raise AssertionError('invalid cleanup threshold accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400
            # Unsupported providers cannot be saved through HTTP or restored by import.
            try:
                api('channels','POST',{'channelName':'Removed provider','platform':'TikTok',
                    'streamUrl':'rtmp://example.test/live','streamKey':'test'})
                raise AssertionError('removed provider accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400 and 'Piattaforma non supportata' in e.read().decode()
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
            assert history['channelName']=='Active profile' and history['platform']=='Custom'
            saved_channel=api(f'channels/{channel}')
            api(f'channels/{channel}','PUT',{**saved_channel,'channelName':'Renamed channel','platform':'Twitch'})
            assert api(f'history/{live}')['channelName']=='Active profile', 'history retained channel identity'
            rows=api('state')['liveRows']
            persisted_row=next(row for row in rows if row['id']==live)
            assert persisted_row['isPlaylist'] and persisted_row['total']==2 and persisted_row['channelName']=='Active profile'
            with urllib.request.urlopen(urllib.request.Request(base+f'/api/history/{live}/video/0',headers={'Authorization':'Bearer '+token,'Range':'bytes=0-99'})) as r:
                assert r.status==206 and len(r.read())==100,'history video range playback'
            previews=api('scan','POST',{'path':str(root),'preview':True})
            assert str(clip) in [v['videoPath'] for v in previews]
            assert previews==api('scan','POST',{'path':str(root),'preview':True}), 'repeat scan remains read-only'
            replay_output=root/'replay.flv'
            replay=api('live/start','POST',{'files':[str(clip),str(clip)],'profileId':profile,'startIndex':1,'offsetSeconds':.5,'loop':False,'outputFile':str(replay_output)})['id']
            for _ in range(100):
                replay_state=next(state for state in api('live') if state['id']==replay)
                if replay_state['status'] in ('ENDED','ERROR'): break
                time.sleep(.05)
            assert replay_state['status']=='ENDED', replay_state
            replay_info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_format','-of','json',str(replay_output)],text=True))
            assert 1 < float(replay_info['format']['duration']) < 2, replay_info
            try:
                api('live/start','POST',{'files':[str(clip)],'profileId':profile,'startIndex':1,'outputFile':str(root/'invalid.flv')})
                raise AssertionError('invalid playlist start index accepted')
            except urllib.error.HTTPError as e:
                assert e.code==400
            assert 'streamKey' not in json.dumps(api('export')) or 'secret' not in json.dumps(api('export'))
            # Mixed mute, takeover and resume go through the persistent publisher.
            output2=root/'switch.flv'
            live2=api('live/start','POST',{'files':[str(clip)],'profileId':profile,'loop':True,'outputFile':str(output2)})['id']
            for _ in range(160):
                switched=next(s for s in api('live') if s['id']==live2)
                if switched['status'] in ('LIVE','ERROR'): break
                time.sleep(.05)
            assert switched['status']=='LIVE', switched
            def delivered(predicate):
                for _ in range(200):
                    current=next(s for s in api('live') if s['id']==live2)
                    assert current['status'] not in ('ERROR','ENDED'), current
                    if predicate(current): return current
                    time.sleep(.05)
                raise AssertionError(('Command not delivered',current))
            api(f'live/{live2}/volume','POST',{'index':0,'volume':30,'audioEnabled':True})
            delivered(lambda s: s['scene']['items'][0].get('volume')==30)
            api(f'live/{live2}/takeover','POST',{'scene':{'name':'Takeover','items':[{'sourceKind':'FILE','sourceTarget':str(clip),'audioEnabled':True}]},'durationSeconds':1})
            delivered(lambda s: s.get('takeover',{}).get('scene',{}).get('name')=='Takeover')
            api(f'live/{live2}/resume','POST',{})
            delivered(lambda s: not s.get('takeover',{}).get('scene') and s['offsetSeconds']>.25)
            api(f'live/{live2}/stop','POST',{})
            assert api('history/'+str(live2))['status']=='STOPPED'
            # Verify the media actually delivered, independent of codec compression or CPU speed.
            decoded=json.loads(subprocess.check_output(['ffprobe','-v','error','-count_frames','-show_streams','-of','json',str(output2)],text=True))
            assert {'video','audio'} <= {s['codec_type'] for s in decoded['streams']}, decoded
            assert all(int(s.get('nb_read_frames',0))>0 for s in decoded['streams']), decoded
            # Il worker elimina snapshot datati e restituisce conteggi coerenti senza toccare una live.
            old={'status':'ENDED','channelId':channel,'channelName':'Old channel','platform':'Custom','startedAt':'2020-01-01T00:00:00Z','request':{'files':['one.mp4','two.mp4']}}
            old_id=api('history','POST',old)['id']
            protected=api('history','POST',{**old,'status':'LIVE'})['id']
            try:
                api(f'history/{protected}','DELETE')
                raise AssertionError('active persisted history deleted')
            except urllib.error.HTTPError as e:
                assert e.code==400
            api('history/cleanup','POST',{'months':0,'channelId':channel,'async':True})
            for _ in range(100):
                cleanup=api('history/cleanup')
                if not cleanup['running']: break
                time.sleep(.01)
            assert not cleanup['running'] and not cleanup['error'],cleanup
            assert cleanup['totalVideos']==2 and cleanup['deletedVideos']==2 and cleanup['deletedLives']==1,cleanup
            assert api(f'history/{protected}')['status']=='LIVE'
            assert old_id not in [h['id'] for h in api('history')]
            api(f'history/{protected}','PUT',{**old,'status':'ENDED'})
            api(f'history/{protected}','DELETE')
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
            # Una connessione TCP accettata senza handshake RTMP non è una live.
            with socket.socket() as pending:
                pending.bind(('127.0.0.1',0))
                pending.listen(1)
                pending.settimeout(5)
                waiting=api('live/start','POST',{'files':[str(clip)],'profileId':profile,'loop':True,'streamUrl':f'rtmp://127.0.0.1:{pending.getsockname()[1]}/live','streamKey':'waiting-secret','reconnect':False})['id']
                connection,_=pending.accept()
                try:
                    time.sleep(.7)
                    connecting=next(s for s in api('live') if s['id']==waiting)
                    assert connecting['status']=='STARTING' and connecting['outputKbps']==0, connecting
                    api(f'live/{waiting}/stop','POST',{})
                finally:
                    connection.close()
            # Gli errori del server devono essere leggibili senza esporre la chiave.
            with socket.socket() as refused:
                refused.bind(('127.0.0.1',0))
                refused_port=refused.getsockname()[1]
                failed=api('live/start','POST',{'files':[str(clip)],'profileId':profile,'streamUrl':f'rtmp://127.0.0.1:{refused_port}/live','streamKey':'private-stream-key','reconnect':False})['id']
                statuses=[]
                for _ in range(100):
                    rejected=next(s for s in api('live') if s['id']==failed)
                    statuses.append(rejected['status'])
                    if rejected['status']=='ERROR':break
                    time.sleep(.05)
                assert rejected['status']=='ERROR' and 'Connection refused' in rejected['error'],rejected
                assert 'LIVE' not in statuses and rejected['outputKbps']==0,rejected
                assert 'private-stream-key' not in rejected['error'] and 'rtmp://' not in rejected['error'],rejected
                assert api(f'history/{failed}')['error']==rejected['error']
                with urllib.request.urlopen(urllib.request.Request(base+f'/api/logs/{failed}', headers={'Authorization':'Bearer '+token})) as response:
                    logs = response.read().decode()
                assert '.session.log' in logs and '.publisher.log' in logs and '[error]' in logs, logs
                assert 'Connection refused' in logs and 'private-stream-key' not in logs, logs
                for log in (root/'data'/'logs').glob(f'{failed}*.log'):
                    assert 'private-stream-key' not in log.read_text(), log
                    assert log.stat().st_mode & 0o077 == 0, log
                # A pending reconnect must stay protected from inactive-history cleanup.
                retrying=api('live/start','POST',{'files':[str(clip)],'profileId':profile,'streamUrl':f'rtmp://127.0.0.1:{refused_port}/live','streamKey':'private-stream-key'})['id']
                for _ in range(100):
                    reconnecting=next(s for s in api('live') if s['id']==retrying)
                    if reconnecting['status']=='RECONNECTING': break
                    time.sleep(.05)
                assert reconnecting['status']=='RECONNECTING',reconnecting
                assert api(f'history/{retrying}')['status']=='RECONNECTING'
                api(f'live/{retrying}/stop','POST',{})
                assert api(f'history/{retrying}')['status']=='STOPPED'
            # Riavvia il server per esercitare lo scheduler reale senza aspettare un intervallo artificiale.
            scheduled_channel=api(f'channels/{channel}')
            api(f'channels/{channel}','PUT',{**scheduled_channel,'autoCleanupEnabled':True,'autoCleanupIntervalMonths':1,'autoCleanupOlderThanMonths':0,'lastCleanupEpoch':0})
            scheduled=api('history','POST',{**old,'channelId':channel})['id']
            proc.send_signal(signal.SIGTERM)
            proc.wait(timeout=8)
            proc=subprocess.Popen([sys.argv[1],'--no-browser','--data-dir',str(root/'data'),'--port',str(port)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            for _ in range(3):
                line=proc.stdout.readline()
                if '?token=' in line:
                    token=line.strip().split('token=')[-1]
                    break
            for _ in range(100):
                try:
                    restarted=api('state')
                    if scheduled not in [item['id'] for item in restarted['history']]: break
                except urllib.error.URLError:
                    pass
                time.sleep(.05)
            assert scheduled not in [item['id'] for item in restarted['history']], 'scheduled cleanup at startup'
            assert api(f'channels/{channel}')['lastCleanupEpoch']>0
            assert live in [row['id'] for row in restarted['liveRows']], 'live table retained finished histories after restart'
            print('HTTP/SQLite, FFmpeg playlist/replay/RTMP, timestamps, mixer, stop status, history cleanup/scheduler: OK')
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
