import { spawn } from 'node:child_process';
import { mkdtemp, rm, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { createServer } from 'node:net';
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const temp = await mkdtemp(join(tmpdir(), 'orbis-browser-'));
async function freePort() {
  const s = createServer();
  await new Promise((r) => s.listen(0, '127.0.0.1', r));
  const p = s.address().port;
  await new Promise((r) => s.close(r));
  return p;
}
const port = await freePort(),
  debug = await freePort();
const app = spawn(process.argv[2] || 'linux/build/orbis-stream', [
  '--no-browser',
  '--port',
  String(port),
  '--data-dir',
  join(temp, 'data'),
]);
let out = '';
app.stdout.on('data', (d) => (out += d));
app.stderr.on('data', (d) => process.stderr.write(d));
let chrome, ws;
try {
  for (let i = 0; i < 100 && !out.includes('?token='); i++) await sleep(100);
  const token = out.match(/token=([a-f0-9]{64})/)?.[1];
  if (!token) throw Error('No app token');
  const url = `http://127.0.0.1:${port}/?token=${token}`;
  chrome = spawn(
    'google-chrome',
    [
      '--headless',
      '--no-sandbox',
      '--disable-gpu',
      '--remote-allow-origins=*',
      `--remote-debugging-port=${debug}`,
      `--user-data-dir=${join(temp, 'chrome')}`,
      '--window-size=1440,1000',
      url,
    ],
    { stdio: 'ignore' },
  );
  let tabs;
  for (let i = 0; i < 100; i++) {
    try {
      tabs = await (await fetch(`http://127.0.0.1:${debug}/json`)).json();
      if (tabs.some((t) => t.type === 'page')) break;
    } catch {}
    await sleep(100);
  }
  const tab = tabs.find((t) => t.type === 'page');
  ws = new WebSocket(tab.webSocketDebuggerUrl);
  await new Promise((resolve, reject) => {
    ws.onopen = resolve;
    ws.onerror = reject;
  });
  let id = 0;
  const waiting = new Map(),
    errors = [];
  ws.onmessage = (e) => {
    const m = JSON.parse(e.data);
    if (m.id) {
      const cb = waiting.get(m.id);
      waiting.delete(m.id);
      m.error ? cb.reject(Error(m.error.message)) : cb.resolve(m.result);
    }
    if (m.method === 'Runtime.exceptionThrown') errors.push(m.params.exceptionDetails.text);
  };
  const send = (method, params = {}) =>
    new Promise((resolve, reject) => {
      const n = ++id;
      waiting.set(n, { resolve, reject });
      ws.send(JSON.stringify({ id: n, method, params }));
    });
  const evaluate = async (expression) => {
    const r = await send('Runtime.evaluate', {
      expression,
      awaitPromise: true,
      returnByValue: true,
    });
    if (r.exceptionDetails)
      throw Error(r.exceptionDetails.exception?.description || r.exceptionDetails.text);
    return r.result.value;
  };
  await send('Runtime.enable');
  await send('Page.enable');
  for (let i = 0; i < 100; i++) {
    if (await evaluate(`document.querySelector('#content .card')!==null`)) break;
    await sleep(100);
  }
  if (
    !(await evaluate(
      `['Panoramica','Pannello'].includes(document.querySelector('#title').textContent)`,
    ))
  )
    throw Error('Dashboard not rendered');
  const navCount = await evaluate(`document.querySelectorAll('#navigation button').length`);
  if (navCount !== 7)
    throw Error(
      `Windows navigation: expected 7 entries, got ${navCount}; ` +
        `notice=${await evaluate(`document.querySelector('#notice').textContent`)}; ` +
        `errors=${errors.join(', ')}`,
    );
  if (
    !(await evaluate(
      `document.querySelector('[data-windows-source="Pages/MainMenu.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainMenu template');
  await evaluate(`document.querySelectorAll('#navigation button')[1].click()`);
  if (
    !(await evaluate(
      `document.querySelector('[data-windows-source="Pages/MainLiveMenu.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainLiveMenu template');
  await evaluate(`document.querySelectorAll('#navigation button')[2].click()`);
  await sleep(100);
  if (
    !(await evaluate(
      `location.pathname==='/orbis/mainPreview' && document.querySelector('[data-windows-source="Pages/MainPreview.cshtml"]')!==null && !document.querySelector('[data-preview-empty]').hidden`,
    ))
  )
    throw Error('Windows MainPreview empty state');
  await evaluate(`document.querySelectorAll('#navigation button')[6].click()`);
  await sleep(200);
  if (
    !(await evaluate(
      `document.querySelector('[data-windows-source="Pages/MainSetting.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainSetting template');
  await evaluate(
    `[...document.querySelectorAll('#content .card')].find(c=>c.textContent.includes('Settaggi Canale')).click()`,
  );
  if (!(await evaluate(`location.pathname==='/orbis/mainChannelSetting'`)))
    throw Error('Windows channel route');
  if (
    !(await evaluate(
      `document.querySelector('[data-windows-source="Pages/MainChannelSetting.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainChannelSetting template');
  await evaluate(`document.querySelector('[data-new-channel]').click()`);
  await sleep(100);
  await evaluate(
    `document.querySelector('#modal [name=channelName]').value='Canale test';document.querySelector('#modal [name=streamUrl]').value='rtmp://127.0.0.1/live';document.querySelector('#modal [name=streamKey]').value='test-key';document.querySelector('#modal-footer .primary').click()`,
  );
  await sleep(350);
  if (!(await evaluate(`document.querySelector('#content').textContent.includes('Canale test')`)))
    throw Error('Channel CRUD UI');
  await evaluate(`document.querySelector('[data-new-channel]').click()`);
  if (
    !(await evaluate(
      `(()=>{const p=document.querySelector('#modal [name=platform]');p.value='YouTube';p.dispatchEvent(new Event('change'));return document.querySelector('#modal [name=streamUrl]').value==='rtmps://a.rtmps.youtube.com/live2'})()`,
    ))
  )
    throw Error('Windows YouTube ingest preset');
  if (
    !(await evaluate(
      `(()=>{const p=document.querySelector('#modal [name=platform]');p.value='TikTok';p.dispatchEvent(new Event('change'));return document.querySelector('#modal [name=streamKey]').closest('label').hidden})()`,
    ))
  )
    throw Error('Windows TikTok temporary key field');
  await evaluate(`document.querySelector('#modal [value=cancel]').click()`);
  if (
    !(await evaluate(
      `(()=>{const s=document.querySelector('[name=platformFilter]');s.value='TikTok';s.dispatchEvent(new Event('change'));return document.querySelector('#content [data-platform="Custom"]').hidden})()`,
    ))
  )
    throw Error('Windows platform filter');
  await evaluate(
    `(()=>{const s=document.querySelector('[name=platformFilter]');s.value='Custom';s.dispatchEvent(new Event('change'))})()`,
  );
  await evaluate(
    `[...document.querySelector('#content [data-platform="Custom"]').querySelectorAll('button')].find(b=>b.textContent.includes('Disattiva')).click()`,
  );
  await sleep(250);
  if (
    !(await evaluate(
      `document.querySelector('#content [data-platform="Custom"]').dataset.active==='false'`,
    ))
  )
    throw Error('Windows channel toggle');
  await evaluate(
    `[...document.querySelector('#content [data-platform="Custom"]').querySelectorAll('button')].find(b=>b.textContent.includes('Attiva')).click()`,
  );
  await sleep(250);
  await evaluate(`document.querySelectorAll('#navigation button')[6].click()`);
  await evaluate(
    `[...document.querySelectorAll('#content .card')].find(c=>c.textContent.includes('Video e Audio')).click()`,
  );
  if (
    !(await evaluate(
      `location.pathname==='/orbis/mainVideoSetting' && document.querySelector('[data-windows-source="Pages/MainVideoSetting.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainVideoSetting template');
  await evaluate(`document.querySelector('[data-new-profile]').click()`);
  await evaluate(
    `[...document.querySelectorAll('#modal .preset-bar button')].find(b=>b.textContent==='Twitch').click()`,
  );
  if (!(await evaluate(`document.querySelector('#modal [name=title]').value==='Twitch'`)))
    throw Error('Windows profile preset');
  await evaluate(`document.querySelector('#modal-footer .primary').click()`);
  await sleep(300);
  if (
    !(await evaluate(
      `document.querySelector('#content .row-card')?.textContent.includes('Twitch')`,
    ))
  )
    throw Error('Profile save UI');
  await evaluate(
    `[...document.querySelector('#content .row-card').querySelectorAll('button')].find(b=>b.textContent.includes('Attiva')).click()`,
  );
  await sleep(250);
  if (
    !(await evaluate(
      `[...document.querySelector('#content .row-card').querySelectorAll('button')].some(b=>b.textContent.includes('Disattiva'))`,
    ))
  )
    throw Error('Profile active toggle');
  await evaluate(`document.querySelectorAll('#navigation button')[3].click()`);
  await sleep(150);
  if (!(await evaluate(`location.pathname==='/orbis/mainLayoutList'`)))
    throw Error('Windows layout route');
  if (
    !(await evaluate(
      `document.querySelector('[data-windows-source="Pages/MainLayoutList.cshtml"]')!==null`,
    ))
  )
    throw Error('Windows MainLayoutList template');
  await evaluate(
    `[...document.querySelectorAll('#content button')].find(b=>b.textContent==='Nuovo layout').click()`,
  );
  await sleep(100);
  await evaluate(
    `[...document.querySelectorAll('#modal .toolbar button')].find(b=>b.textContent==='Slot').click()`,
  );
  await sleep(100);
  if (!(await evaluate(`document.querySelectorAll('#modal .tile').length===1`)))
    throw Error('Canvas slot creation');
  const box = await evaluate(
    `(()=>{const b=document.querySelector('#modal .tile').getBoundingClientRect();return {x:b.x+30,y:b.y+30};})()`,
  );
  await send('Input.dispatchMouseEvent', {
    type: 'mousePressed',
    x: box.x,
    y: box.y,
    button: 'left',
    clickCount: 1,
  });
  await send('Input.dispatchMouseEvent', {
    type: 'mouseMoved',
    x: box.x + 35,
    y: box.y + 20,
    buttons: 1,
  });
  await send('Input.dispatchMouseEvent', {
    type: 'mouseReleased',
    x: box.x + 35,
    y: box.y + 20,
    button: 'left',
    clickCount: 1,
  });
  if (!(await evaluate(`parseFloat(document.querySelector('#modal .tile').style.left)>0`)))
    throw Error('Canvas dragging');
  await evaluate(`document.querySelector('#modal-footer .primary').click()`);
  await sleep(300);
  if (!(await evaluate(`document.querySelector('#content').textContent.includes('Nuovo layout')`)))
    throw Error('Layout persistence UI');
  await evaluate(`document.querySelectorAll('#navigation button')[5].click()`);
  await sleep(300);
  if (!(await evaluate(`document.querySelectorAll('.manual article').length>5`)))
    throw Error('Localized manual not rendered');
  await evaluate(`document.querySelectorAll('#navigation button')[0].click()`);
  await sleep(100);
  const screenshot = await send('Page.captureScreenshot', { format: 'png' });
  await writeFile(
    process.argv[3] || join(temp, 'screenshot.png'),
    Buffer.from(screenshot.data, 'base64'),
  );
  if (errors.length) throw Error('Browser errors: ' + errors.join(', '));
  console.log(
    'Browser: dashboard, channel CRUD, canvas slot, drag, save, localized manual, screenshot: OK',
  );
} finally {
  ws?.close();
  chrome?.kill('SIGTERM');
  app.kill('SIGTERM');
  await sleep(500);
  await rm(temp, { recursive: true, force: true });
}
