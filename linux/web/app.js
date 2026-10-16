'use strict';
let deckActive = false;
let token = '',
  data = {},
  page = 'dashboard',
  watched = 0,
  modalOpen = false,
  modalGeneration = 0,
  language = localStorage.getItem('orbis-language') || 'it',
  bundle = {},
  timer,
  historySearchTimer,
  sourceCatalog = [];
/** Seleziona elementi e costruisce il DOM; i testi diventano nodi testuali per evitare interpretazione HTML. */
const $ = (s) => document.querySelector(s),
  el = (tag, attrs = {}, ...children) => {
    const e = document.createElement(tag);
    for (const [k, v] of Object.entries(attrs)) {
      if (k.startsWith('on')) e.addEventListener(k.slice(2), v);
      else if (k === 'class') e.className = v;
      else if (k === 'text') e.textContent = v;
      else if (k === 'checked') e.checked = v;
      else e.setAttribute(k, v);
    }
    for (const c of children.flat()) {
      if (c !== undefined && c !== null)
        e.append(c instanceof Node ? c : document.createTextNode(String(c)));
    }
    return e;
  };
// I percorsi riprendono le pagine Windows; tab interni mantengono accessibili gli strumenti Linux.
const routes = {
  dashboard: '/orbis/mainMenu',
  liveMenu: '/orbis/mainLiveMenu',
  live: '/orbis/mainLive',
  preview: '/orbis/mainPreview',
  layoutList: '/orbis/mainLayoutList',
  scenes: '/orbis/mainLayout',
  history: '/orbis/mainLiveHistory',
  activity: '/orbis/mainTaskManager',
  manual: '/orbis/manual',
  settingsMenu: '/orbis/mainSetting',
  channels: '/orbis/mainChannelSetting',
  profiles: '/orbis/mainVideoSetting',
  playlist: '/orbis/mainLive?tab=playlist',
  overlays: '/orbis/mainLayout?tab=overlays',
  buttons: '/orbis/mainPreview?tab=buttons',
  settings: '/orbis/mainSetting?tab=advanced',
};
const names = {
  dashboard: 'Pannello',
  liveMenu: 'Live',
  live: 'Gestione Live',
  preview: 'Anteprima live',
  layoutList: 'Layout',
  playlist: 'Playlist',
  scenes: 'Scene e layout',
  overlays: 'Overlay',
  buttons: 'Pulsanti scena',
  channels: 'Canali',
  profiles: 'Profili video',
  history: 'Storico',
  activity: 'Attività',
  settingsMenu: 'Impostazioni',
  settings: 'Altre impostazioni',
  manual: 'Manuale',
};
const sidebarPages = [
  'dashboard',
  'liveMenu',
  'preview',
  'layoutList',
  'activity',
  'manual',
  'settingsMenu',
];
const pageTemplates = new Map();
const searchByPage = Object.create(null);
const channelFilters = { platform: '', active: '' };
/** Carica i file HTML che corrispondono alle schermate Windows prima del primo rendering. */
async function loadPageTemplates() {
  await Promise.all(
    [
      'MainMenu',
      'MainLiveMenu',
      'MainSetting',
      'MainLayoutList',
      'MainPreview',
      'MainChannelSetting',
      'MainVideoSetting',
      'MainLiveHistory',
      'MainLive',
      '_LiveRows',
      '_StartWizardSetting',
      '_StartWizardConfiguration',
    ].map(async (name) => {
      const response = await fetch(`/pages/${name}.html`);
      if (!response.ok) throw Error(`Pagina ${name} non disponibile`);
      const template = document.createElement('template');
      // Sono file HTML statici inclusi nel pacchetto; i dati utente sono sempre inseriti come testo.
      template.innerHTML = await response.text();
      pageTemplates.set(name, template);
    }),
  );
}
/** Inserisce la pagina originale tradotta e collega i suoi link alla navigazione locale. */
function renderWindowsPage(root, name) {
  const template = pageTemplates.get(name);
  if (!template) return false;
  const fragment = template.content.cloneNode(true);
  fragment.querySelectorAll('[data-i18n]').forEach((node) => {
    node.textContent = t(node.dataset.i18n, node.textContent);
  });
  fragment.querySelectorAll('[data-go]').forEach((link) => {
    link.addEventListener('click', (event) => {
      if (event.button || event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
      event.preventDefault();
      navigate(link.dataset.go);
    });
  });
  fragment.querySelectorAll('[data-start]').forEach((link) => {
    link.addEventListener('click', (event) => {
      event.preventDefault();
      startWizard();
    });
  });
  fragment.querySelectorAll('[data-playlist-start]').forEach((control) => {
    control.addEventListener('click', () => playlistStartWizard());
  });
  fragment.querySelectorAll('[data-new-layout]').forEach((control) => {
    control.addEventListener('click', () =>
      compose({ name: 'Nuovo layout', width: 1280, height: 720, isLayout: true, items: [] }),
    );
  });
  root.append(fragment);
  return true;
}
/** Converte la URL nella vista corrispondente per mantenere funzionanti ricarica e cronologia. */
function pageFromLocation() {
  const tab = new URLSearchParams(location.search).get('tab');
  return (
    Object.entries(routes).find(([key, route]) => {
      const url = new URL(route, location.origin);
      return url.pathname === location.pathname && url.searchParams.get('tab') === tab;
    })?.[0] || (names[location.hash.slice(1)] ? location.hash.slice(1) : 'dashboard')
  );
}
/** Aggiorna pagina e indirizzo insieme, così i pulsanti Indietro/Avanti seguono lo stesso flusso. */
function navigate(key) {
  clearTimeout(historySearchTimer);
  page = key;
  window.history.pushState({ page: key }, '', routes[key]);
  render();
}
window.addEventListener('popstate', () => {
  clearTimeout(historySearchTimer);
  page = pageFromLocation();
  render();
});
/** Risolve una traduzione con fallback, così le nuove voci restano leggibili anche nei dizionari incompleti. */
function t(key, fallback) {
  return bundle[key] || fallback;
}
/** Mostra un messaggio temporaneo senza interrompere il flusso di lavoro con una finestra bloccante. */
function notify(message) {
  $('#notice').textContent = message;
  $('#notice').style.display = 'block';
  clearTimeout(timer);
  timer = setTimeout(() => ($('#notice').style.display = 'none'), 6000);
}
/** Centralizza JSON, token ed errori HTTP per mantenere uniforme il comportamento delle richieste. */
async function api(path, method = 'GET', body) {
  const options = { method, headers: { 'X-Orbis-Token': token } };
  if (body !== undefined) {
    options.headers['Content-Type'] = 'application/json';
    options.body = JSON.stringify(body);
  }
  const response = await fetch('/api/' + path, options);
  const result = response.status === 204 ? null : await response.json();
  if (!response.ok) throw Error(result?.message || response.statusText);
  return result;
}
/** Gestisce gli errori delle azioni e aggiorna lo stato dopo il successo, evitando gestori ripetuti nei pulsanti. */
async function action(fn) {
  try {
    await fn();
    await refresh();
  } catch (e) {
    notify(e.message);
    if (modalOpen) {
      $('#modal [data-modal-error]')?.remove();
      $('#modal-body').prepend(
        el('p', { class: 'infobar error', role: 'alert', 'data-modal-error': '' }, e.message),
      );
    }
  }
}
/** Condivide errori e refresh e blocca il doppio clic mentre un'azione è in corso. */
const button = (label, fn, cls = '') => {
  const control = el('button', { type: 'button', class: cls }, label);
  control.addEventListener('click', async () => {
    control.disabled = true;
    await action(fn);
    if (control.isConnected) control.disabled = false;
  });
  return control;
};
/** Costruisce campi e selettori con la stessa struttura DOM per uniformare i moduli. */
function field(label, name, value = '', type = 'text', options) {
  const input =
    type === 'select'
      ? el(
          'select',
          { name },
          options.map(([v, label]) => el('option', { value: v }, label)),
        )
      : el(type === 'textarea' ? 'textarea' : 'input', {
          name,
          type: type === 'textarea' ? undefined : type,
        });
  input.value = value ?? '';
  return el('label', {}, label, input);
}
/** Costruisce una casella con etichetta associata per rendere riconoscibile il valore booleano. */
function check(label, name, value) {
  return el(
    'label',
    { class: 'check' },
    el('input', { type: 'checkbox', name, checked: !!value }),
    label,
  );
}
/** Legge il campo del modulo aperto, centralizzando l’accesso ai valori inseriti. */
function value(name) {
  return document.querySelector(`#modal [name="${name}"]`)?.value ?? '';
}
/** Legge lo stato booleano senza confonderlo con il valore testuale del controllo. */
function checked(name) {
  return document.querySelector(`#modal [name="${name}"]`)?.checked ?? false;
}
/** Converte il campo in numero; se è vuoto usa il valore predefinito per i parametri opzionali. */
function number(name, fallback = 0) {
  const s = value(name);
  return s === '' ? fallback : Number(s);
}
/** Apre un modulo con salvataggio comune, così ogni editor condivide il ciclo di apertura e chiusura. */
function openModal(title, content, save, label = t('save', 'Salva')) {
  ++modalGeneration;
  modalOpen = true;
  $('#modal-title').textContent = title;
  $('#modal-body').replaceChildren(content);
  $('#modal-footer').replaceChildren(button(t('cancel', 'Annulla'), closeModal));
  if (save) $('#modal-footer').append(button(label, save, 'primary'));
  $('#modal').showModal();
}
/** Chiude l’editor e ripristina lo stato della pagina per consentire nuovamente gli aggiornamenti completi. */
function closeModal() {
  ++modalGeneration;
  $('#modal').close();
  modalOpen = false;
  deckActive = false;
}
$('#modal').addEventListener('close', () => {
  if (!$('#modal').open) {
    ++modalGeneration;
    modalOpen = false;
    deckActive = false;
  }
});
const navKeys = {
  dashboard: 'dashboard',
  liveMenu: 'liveManagement',
  live: 'liveManagement',
  preview: 'livePreviewTitle',
  layoutList: 'layouts',
  playlist: 'startPlaylistButton',
  scenes: 'sceneStrip',
  overlays: 'overlays',
  buttons: 'sceneDeck',
  channels: 'channelSettings',
  profiles: 'videoSettings',
  history: 'liveHistory',
  activity: 'taskManager',
  settings: 'settings',
  settingsMenu: 'settings',
  manual: 'manual',
};
/** Ricostruisce la navigazione dalla lingua e dalla pagina attive per mantenere coerente la selezione. */
function renderNav() {
  const icons = ['◈', '●', '▣', '▧', '↗', '?', '⚙'];
  const activeGroup =
    {
      live: 'liveMenu',
      playlist: 'liveMenu',
      history: 'liveMenu',
      scenes: 'layoutList',
      overlays: 'layoutList',
      buttons: 'preview',
      channels: 'settingsMenu',
      profiles: 'settingsMenu',
      settings: 'settingsMenu',
    }[page] || page;
  $('#navigation').replaceChildren(
    ...sidebarPages.map((key, i) =>
      button(
        icons[i] + '  ' + t(navKeys[key], names[key]),
        () => navigate(key),
        activeGroup === key ? 'active' : '',
      ),
    ),
  );
}
/** Instrada il rendering alla vista corrente, separando la navigazione dai singoli pannelli. */
function render() {
  renderNav();
  $('#title').textContent = t(navKeys[page], names[page] || page);
  $('#start').hidden = !['liveMenu', 'live', 'preview'].includes(page);
  $('#stop-all').hidden = page !== 'live';
  $('#live-count').textContent =
    (data.live || []).filter((s) => s.status === 'LIVE' || s.status === 'STARTING').length +
    ' dirette attive';
  const content = $('#content');
  content.replaceChildren();
  const background = data.preferences?.find((p) => p.id === 1)?.backgroundMediaName;
  content.style.backgroundImage =
    page === 'dashboard' && background
      ? 'linear-gradient(#0005,#0005),url(/api/media/images/' + encodeURIComponent(background) + ')'
      : '';
  content.style.backgroundSize = 'cover';
  (
    ({
      dashboard,
      liveMenu,
      live,
      preview,
      layoutList,
      playlist,
      scenes,
      overlays,
      buttons,
      channels,
      profiles,
      history: historyPage,
      activity,
      settings,
      settingsMenu,
      manual,
    })[page] || dashboard
  )(content);
  applyRowFilters();
}
/** Combina ricerca e filtri Canali dopo ogni refresh per non perdere le scelte durante il polling. */
function applyRowFilters() {
  const query = (searchByPage[page] || '').toLowerCase();
  document.querySelectorAll('#content [data-search]').forEach((node) => {
    const textMatches = node.dataset.search.toLowerCase().includes(query);
    const channelMatches =
      page !== 'channels' ||
      ((!channelFilters.platform || node.dataset.platform === channelFilters.platform) &&
        (!channelFilters.active || node.dataset.active === channelFilters.active));
    node.hidden = !textMatches || !channelMatches;
  });
}
/** Aggiorna i dati senza ricostruire il modulo aperto, preservando le modifiche ancora non salvate. */
async function refresh() {
  data = await api('state');
  // Il polling non deve sostituire il campo mentre l'utente sta scrivendo un filtro.
  const editingHistory =
    page === 'history' &&
    document.activeElement?.matches('input, select') &&
    document.activeElement?.closest('[data-history-filters], [data-history-cleanup-settings]');
  if (!modalOpen && !editingHistory) render();
  else updatePreviews();
  if (page === 'history') updateCleanupProgress();
}
/** Mostra uno stato vuoto esplicito, così l’assenza di dati non appare come un errore di caricamento. */
function empty(root, text) {
  root.append(el('div', { class: 'empty' }, text));
}
/** Riepiloga configurazione e dirette per offrire un accesso rapido alle operazioni principali. */
function dashboard(root) {
  if (renderWindowsPage(root, 'MainMenu')) return;
  root.append(el('p', {}, t('dashboardDescription', 'Seleziona una sezione.')));
  sectionCards(root, 'dashboardSectionLive', 'Streaming', [
    ['live', 'liveManagement', 'liveManagementDescription'],
    ['preview', 'preview', 'previewDescription'],
    ['layoutList', 'layouts', 'layoutsDescription'],
    ['history', 'liveHistory', 'liveHistoryDescription'],
  ]);
  sectionCards(root, 'dashboardSectionSettings', 'Impostazioni', [
    ['profiles', 'videoAndAudio', 'additionalVideoOptionsDescription'],
    ['channels', 'channelSettings', 'channelSettingsDescription'],
  ]);
  sectionCards(root, 'dashboardSectionSystem', 'Sistema e monitoraggio', [
    ['activity', 'taskManager', 'taskManagerDescription'],
  ]);
  sectionCards(root, 'manual', 'Manuale', [['manual', 'manualTitle', 'manualDescription']]);
}
/** Riproduce i gruppi di accesso Windows usando le traduzioni e i percorsi delle sue pagine. */
function sectionCards(root, heading, fallback, entries) {
  root.append(el('h3', { class: 'section' }, t(heading, fallback)));
  root.append(
    el(
      'div',
      { class: 'grid' },
      ...entries.map(([key, title, description]) =>
        el(
          'article',
          { class: 'card' },
          el('h3', {}, t(title, names[key])),
          el('p', {}, t(description, '')),
          button(t('open', 'Apri'), () => navigate(key)),
        ),
      ),
    ),
  );
}
/** Offre la stessa scelta Live/Anteprima/Layout/Storico del menu Windows. */
function liveMenu(root) {
  if (renderWindowsPage(root, 'MainLiveMenu')) return;
  root.append(el('p', {}, t('liveManagementDescription', 'Gestisci le dirette.')));
  sectionCards(root, 'dashboardSectionLive', 'Streaming', [
    ['live', 'liveManagement', 'liveManagementDescription'],
    ['preview', 'preview', 'previewDescription'],
    ['layoutList', 'layouts', 'layoutsDescription'],
    ['history', 'liveHistory', 'liveHistoryDescription'],
  ]);
}
/** Riunisce i profili e i canali come nella pagina Impostazioni Windows. */
function settingsMenu(root) {
  if (renderWindowsPage(root, 'MainSetting')) return;
  root.append(el('p', {}, t('selectSectionMessage', 'Seleziona la sezione da configurare.')));
  sectionCards(root, 'settings', 'Impostazioni', [
    ['profiles', 'videoAndAudio', 'additionalVideoOptionsDescription'],
    ['channels', 'channelSettings', 'channelSettingsDescription'],
  ]);
  root.append(button('Altre impostazioni', () => navigate('settings')));
}
/** Elenca i canali e le relative azioni usando lo stato già caricato, evitando una richiesta per ogni riga. */
function channels(root) {
  if (renderWindowsPage(root, 'MainChannelSetting')) {
    const platform = root.querySelector('[name=platformFilter]');
    const active = root.querySelector('[name=activeFilter]');
    const searchInput = root.querySelector('[name=channelSearch]');
    platform.value = channelFilters.platform;
    active.value = channelFilters.active;
    searchInput.value = searchByPage[page] || '';
    platform.addEventListener('change', () => {
      channelFilters.platform = platform.value;
      applyRowFilters();
    });
    active.addEventListener('change', () => {
      channelFilters.active = active.value;
      applyRowFilters();
    });
    searchInput.addEventListener('input', () => {
      searchByPage[page] = searchInput.value;
      applyRowFilters();
    });
    root
      .querySelector('[data-new-channel]')
      .addEventListener('click', () =>
        editChannel({ isActive: true, platform: 'Custom', profileId: 0 }),
      );
    const list = root.querySelector('[data-channel-list]');
    for (const channel of data.channels || []) {
      const enabled = channel.isActive !== false;
      list.append(
        el(
          'article',
          {
            class: 'row-card',
            'data-search': `${channel.channelName} ${channel.description || ''} ${channel.streamUrl || ''}`,
            'data-platform': channel.platform || 'Custom',
            'data-active': String(enabled),
          },
          el(
            'div',
            { class: 'grow' },
            el('h3', {}, `${channel.channelName} · ${channel.platform || 'Custom'}`),
            ...(channel.description ? [el('div', { class: 'caption' }, channel.description)] : []),
            el('div', { class: 'caption' }, channel.streamUrl),
          ),
          el(
            'div',
            { class: 'actions' },
            el(
              'span',
              { class: 'badge' },
              enabled ? t('active', 'Attiva') : t('deactive', 'Disattiva'),
            ),
            button(t('seeDetails', 'Dettagli'), () => editChannel(channel)),
            button(enabled ? t('deactivateButton', 'Disattiva') : t('activeButton', 'Attiva'), () =>
              api(`channels/${channel.id}`, 'PUT', { ...channel, isActive: !enabled }),
            ),
          ),
        ),
      );
    }
    root.querySelector('[data-channel-empty]').hidden = !!data.channels?.length;
    return;
  }
  const platformFilter = field('Piattaforma', 'platformFilter', channelFilters.platform, 'select', [
    ['', 'Tutte'],
    ...['Custom', 'Twitch', 'YouTube', 'Kick', 'Facebook', 'TikTok'].map((platform) => [
      platform,
      platform,
    ]),
  ]);
  const activeFilter = field('Stato', 'activeFilter', channelFilters.active, 'select', [
    ['', 'Tutti'],
    ['true', t('active', 'Attivi')],
    ['false', t('deactive', 'Disattivi')],
  ]);
  platformFilter.querySelector('select').addEventListener('change', (event) => {
    channelFilters.platform = event.target.value;
    applyRowFilters();
  });
  activeFilter.querySelector('select').addEventListener('change', (event) => {
    channelFilters.active = event.target.value;
    applyRowFilters();
  });
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Nuovo canale', () =>
        editChannel({ isActive: true, platform: 'Custom', profileId: 0 }),
      ),
      platformFilter,
      activeFilter,
      search(),
    ),
  );
  const grid = el('div', { class: 'grid' });
  for (const c of data.channels || [])
    grid.append(
      el(
        'article',
        {
          class: 'card',
          'data-search': c.channelName + ' ' + (c.description || ''),
          'data-platform': c.platform || 'Custom',
          'data-active': String(c.isActive !== false),
        },
        el('h3', {}, c.channelName),
        el('span', { class: 'badge' }, c.platform || 'Custom'),
        el(
          'span',
          { class: 'badge' },
          c.isActive === false ? t('deactive', 'Disattivo') : t('active', 'Attivo'),
        ),
        el('p', {}, c.description || c.videoFolder || 'Scena configurata'),
        el(
          'div',
          { class: 'actions' },
          button('Modifica', () => editChannel(c)),
          button(
            c.isActive === false ? t('activeButton', 'Attiva') : t('deactivateButton', 'Disattiva'),
            () => api('channels/' + c.id, 'PUT', { ...c, isActive: c.isActive === false }),
          ),
          ...(c.isActive === false ? [] : [button('Avvia', () => startWizard(c.id))]),
          button('Elimina', () => remove('channels', c.id), 'danger'),
        ),
      ),
    );
  root.append(grid);
  if (!data.channels?.length) empty(root, 'Aggiungi il primo canale.');
}
/** Replica la scelta della chiave temporanea da LiveStreamKeys.cs usando nome e dominio ingest. */
function asksForKey(channel) {
  if (
    [channel.platform, channel.platformStreamName].some(
      (value) => value?.trim().toLowerCase() === 'tiktok',
    )
  )
    return true;
  try {
    const host = new URL(channel.streamUrl).hostname.toLowerCase();
    return [
      'tiktokcdn.com',
      'tiktokcdn-eu.com',
      'tiktokcdn-us.com',
      'tiktokv.com',
      'tiktok.com',
    ].some((domain) => host === domain || host.endsWith('.' + domain));
  } catch {
    return false;
  }
}
/** Prepara il modulo del canale e invia i campi al server, mantenendo la validazione finale nel backend. */
function editChannel(c) {
  // Gli ingest iniziali vengono da MainChannelSettingModel.Platforms; gli account possono sostituirli.
  const ingestPresets = {
    Twitch: 'rtmp://live.twitch.tv/app',
    YouTube: 'rtmps://a.rtmps.youtube.com/live2',
    Kick: 'rtmps://fa723fc1b171.global-contribute.live-video.net/app',
    Facebook: 'rtmps://rtmp-api.facebook.com:443/rtmp',
    TikTok: 'rtmp://push-rtmp-l11-va01.tiktokcdn.com/stage',
  };
  const knownPlatforms = ['Custom', 'Twitch', 'YouTube', 'Kick', 'Facebook', 'TikTok'];
  const initialPlatform =
    c.platform ||
    knownPlatforms.find((name) => name.toLowerCase() === c.platformStreamName?.toLowerCase()) ||
    'Custom';
  const legacyPublicName =
    c.platformStreamName &&
    !knownPlatforms.some((name) => name.toLowerCase() === c.platformStreamName.toLowerCase())
      ? c.platformStreamName
      : '';
  const platformField = field(
    'Piattaforma',
    'platform',
    initialPlatform,
    'select',
    knownPlatforms.map((name) => [name, name]),
  );
  const urlField = field('Server RTMP/RTMPS', 'streamUrl', c.streamUrl);
  const keyField = field(
    t('key', 'Chiave'),
    'streamKey',
    c.streamKey?.startsWith('asked-at-every-live:') ? '' : c.streamKey,
    'password',
  );
  const keyHint = el(
    'p',
    { class: 'caption' },
    t('keyEachLiveHint', 'TikTok chiede una chiave nuova a ogni live.'),
  );
  const advanced = el(
    'details',
    {},
    el('summary', {}, 'Opzioni Linux e compatibilità'),
    field('Nome pubblico alternativo', 'publicName', c.publicName || legacyPublicName),
    field('Cartella video Linux', 'videoFolder', c.videoFolder),
    field('Scena', 'scenePkid', c.scenePkid || 0, 'select', [
      [0, 'Playlist dalla cartella'],
      ...(data.scenes || []).filter((s) => !s.isLayout).map((s) => [s.id, s.name]),
    ]),
    field('Profilo', 'profileId', c.profileId || 0, 'select', [
      [0, 'Profilo attivo'],
      ...(data.profiles || []).map((p) => [p.id, p.title]),
    ]),
    check('Canale abilitato', 'isActive', c.isActive !== false),
    check(
      'Pulizia automatica',
      'autoCleanupEnabled',
      c.autoCleanupEnabled ?? c.autoCleanupOlderThanMonths > 0,
    ),
    field(
      'Pulizia: età in mesi (0: ieri, -1: tutto lo storico terminato)',
      'autoCleanupOlderThanMonths',
      c.autoCleanupOlderThanMonths || 0,
      'number',
    ),
    field(
      'Intervallo pulizia automatica (mesi)',
      'autoCleanupIntervalMonths',
      c.autoCleanupIntervalMonths || 1,
      'number',
    ),
  );
  const fields = el(
    'div',
    { class: 'fields' },
    platformField,
    field(t('channelName', 'Nome canale'), 'channelName', c.channelName),
    urlField,
    keyField,
    keyHint,
    field(t('description', 'Descrizione'), 'description', c.description, 'textarea'),
    advanced,
  );
  const platformSelect = platformField.querySelector('select');
  const urlInput = urlField.querySelector('input');
  const keyInput = keyField.querySelector('input');
  const syncPlatform = (changed) => {
    if (changed && (!urlInput.value || Object.values(ingestPresets).includes(urlInput.value)))
      urlInput.value = ingestPresets[platformSelect.value] || '';
    const temporary = asksForKey({ platform: platformSelect.value, streamUrl: urlInput.value });
    keyField.hidden = temporary;
    keyHint.hidden = !temporary;
    keyInput.required = !temporary;
  };
  platformSelect.addEventListener('change', () => syncPlatform(true));
  urlInput.addEventListener('input', () => syncPlatform(false));
  syncPlatform(false);
  openModal(c.id ? 'Modifica canale' : 'Nuovo canale', fields, async () => {
    const v = { ...c };
    for (const k of [
      'channelName',
      'platform',
      'streamUrl',
      'streamKey',
      'publicName',
      'description',
      'videoFolder',
    ])
      v[k] = value(k);
    v.platformStreamName = v.platform.toLowerCase();
    if (!v.streamUrl) v.streamUrl = ingestPresets[v.platform] || '';
    if (!asksForKey(v) && !v.streamKey) throw Error('La chiave di stream è richiesta');
    for (const k of [
      'scenePkid',
      'profileId',
      'autoCleanupOlderThanMonths',
      'autoCleanupIntervalMonths',
    ])
      v[k] = number(k);
    v.isActive = checked('isActive');
    v.autoCleanupEnabled = checked('autoCleanupEnabled');
    await api('channels' + (c.id ? '/' + c.id : ''), c.id ? 'PUT' : 'POST', v);
    closeModal();
  });
  if (c.id)
    $('#modal-footer').prepend(
      button(
        t('delete', 'Elimina'),
        async () => {
          if (confirm('Eliminare questo canale?')) {
            await api(`channels/${c.id}`, 'DELETE');
            closeModal();
          }
        },
        'danger',
      ),
    );
}
/** Mostra i profili riutilizzabili per condividere impostazioni di codifica tra canali e video. */
function profiles(root) {
  if (renderWindowsPage(root, 'MainVideoSetting')) {
    const visible = (data.profiles || []).filter(
      (profile) => !profile.isDefaultConfiguration && !profile.title?.startsWith('Default '),
    );
    root.querySelector('[data-new-profile]').addEventListener('click', () =>
      editProfile({
        title: 'Nuovo profilo',
        videoWidth: 1280,
        videoHeight: 720,
        frameRate: 30,
        videoBitrate: 4500,
        audioBitrate: 128,
        gopSize: 2,
        preset: 'veryfast',
        lowLatency: true,
      }),
    );
    root.querySelector('[data-encoders]').addEventListener('click', () =>
      action(async () => {
        const result = await api('encoders');
        openModal('Encoder FFmpeg', el('pre', {}, result.text));
      }),
    );
    const searchInput = root.querySelector('[data-profile-search]');
    searchInput.value = searchByPage[page] || '';
    searchInput.addEventListener('input', () => {
      searchByPage[page] = searchInput.value;
      applyRowFilters();
    });
    const list = root.querySelector('[data-profile-list]');
    for (const profile of visible)
      list.append(
        el(
          'article',
          { class: 'row-card', 'data-search': profile.title },
          el(
            'div',
            { class: 'grow' },
            el('h3', {}, profile.title),
            el(
              'div',
              { class: 'caption' },
              `${profile.videoCodecName || 'libx264'} · ${profile.videoBitrate || '—'} kbps · ${profile.audioBitrate || '—'} kbps audio`,
            ),
          ),
          el(
            'div',
            { class: 'actions' },
            ...(profile.isActive
              ? [el('span', { class: 'badge' }, t('activeButton', 'Attivo'))]
              : []),
            button(t('editModalTitle', 'Modifica'), () => editProfile(profile)),
            button(t('duplicate', 'Duplica'), () =>
              api('profiles', 'POST', {
                ...profile,
                id: undefined,
                pkid: undefined,
                title: `${profile.title} - ${t('copySuffix', 'copia')}`,
                isActive: false,
                isDefaultConfiguration: false,
              }),
            ),
            button(
              profile.isActive ? t('deactivateButton', 'Disattiva') : t('activeButton', 'Attiva'),
              () =>
                api(`profiles/${profile.id}`, 'PUT', { ...profile, isActive: !profile.isActive }),
            ),
          ),
        ),
      );
    root.querySelector('[data-profile-empty]').hidden = visible.length > 0;
    return;
  }
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Nuovo profilo', () =>
        editProfile({
          title: 'Nuovo profilo',
          videoWidth: 1280,
          videoHeight: 720,
          frameRate: 30,
          videoBitrate: 4500,
          audioBitrate: 128,
          gopSize: 2,
          preset: 'veryfast',
          lowLatency: true,
        }),
      ),
      button('Encoder installati', async () => {
        const r = await api('encoders');
        openModal('Encoder FFmpeg', el('pre', {}, r.text));
      }),
      search(),
    ),
  );
  const grid = el('div', { class: 'grid' });
  for (const p of data.profiles || [])
    grid.append(
      el(
        'article',
        { class: 'card', 'data-search': p.title },
        el('h3', {}, p.title),
        el('p', {}, `${p.videoWidth} × ${p.videoHeight} · ${p.frameRate} fps`),
        el('span', { class: 'badge' }, `${p.videoCodecName || 'libx264'} · ${p.videoBitrate} kbps`),
        el(
          'div',
          { class: 'actions' },
          button('Modifica', () => editProfile(p)),
          button('Qualità Auto', async () => {
            notify('Benchmark x264 in corso…');
            const tuned = await api('tune', 'POST', p);
            await api('profiles/' + p.id, 'PUT', {
              ...p,
              preset: tuned.preset,
              quality: tuned.quality,
            });
            notify('Preset ' + tuned.preset + ' · ' + tuned.speed.toFixed(1) + '×');
          }),
          button('Duplica', () =>
            editProfile({ ...p, id: undefined, pkid: undefined, title: p.title + ' copia' }),
          ),
          button('Elimina', () => remove('profiles', p.id), 'danger'),
        ),
      ),
    );
  root.append(grid);
}
/** Modifica i parametri di codifica in un unico modulo per rendere esplicite le opzioni inviate a FFmpeg. */
function editProfile(p) {
  let chosenPlatform = p.platform || 'Custom';
  const fields = el(
    'div',
    { class: 'fields' },
    field('Titolo', 'title', p.title),
    field(
      'Encoder H.264',
      'videoCodecName',
      p.videoCodecName || 'libx264',
      'select',
      ['libx264', 'h264_nvenc', 'h264_qsv'].map((x) => [x, x]),
    ),
    ...['videoWidth', 'videoHeight', 'frameRate', 'videoBitrate', 'audioBitrate', 'gopSize'].map(
      (k, i) =>
        field(
          [
            'Larghezza',
            'Altezza',
            'FPS',
            'Bitrate video (kbps)',
            'Bitrate AAC (kbps)',
            'GOP (secondi)',
          ][i],
          k,
          p[k],
          'number',
        ),
    ),
    field(
      'Preset x264',
      'preset',
      p.preset || 'veryfast',
      'select',
      ['ultrafast', 'superfast', 'veryfast', 'faster', 'fast', 'medium', 'slow'].map((x) => [x, x]),
    ),
    check('Bassa latenza', 'lowLatency', p.lowLatency),
    check('Adatta bitrate in caso di rallentamento', 'adaptiveBitrate', p.adaptiveBitrate),
    field(
      'Opzioni encoder aggiuntive JSON (crf, profile:v, level, bf, refs)',
      'videoSettingsOptions',
      JSON.stringify(p.videoSettingsOptions || [], null, 2),
      'textarea',
    ),
  );
  const presets = el('div', { class: 'toolbar preset-bar' });
  for (const platform of ['Twitch', 'YouTube', 'Kick', 'Facebook', 'TikTok'])
    presets.append(
      button(platform, () => {
        const source = (data.profiles || []).find(
          (candidate) => candidate.platform === platform && candidate.title?.startsWith('Default '),
        );
        if (!source) throw Error(`Preset ${platform} non disponibile`);
        chosenPlatform = platform;
        for (const key of [
          'videoCodecName',
          'videoWidth',
          'videoHeight',
          'frameRate',
          'videoBitrate',
          'audioBitrate',
          'gopSize',
          'preset',
        ])
          fields.querySelector(`[name=${key}]`).value = source[key] ?? '';
        fields.querySelector('[name=lowLatency]').checked = source.lowLatency !== false;
        if (fields.querySelector('[name=title]').value === 'Nuovo profilo')
          fields.querySelector('[name=title]').value = platform;
      }),
    );
  openModal('Profilo video e audio', el('div', {}, presets, fields), async () => {
    const v = {
      ...p,
      platform: chosenPlatform,
      isDefaultConfiguration: false,
      title: value('title'),
      videoCodecName: value('videoCodecName'),
      preset: value('preset'),
      lowLatency: checked('lowLatency'),
      adaptiveBitrate: checked('adaptiveBitrate'),
      videoSettingsOptions: JSON.parse(value('videoSettingsOptions')),
    };
    for (const k of [
      'videoWidth',
      'videoHeight',
      'frameRate',
      'videoBitrate',
      'audioBitrate',
      'gopSize',
    ])
      v[k] = number(k);
    await api('profiles' + (p.id ? '/' + p.id : ''), p.id ? 'PUT' : 'POST', v);
    closeModal();
  });
  if (p.id && !p.isDefaultConfiguration)
    $('#modal-footer').prepend(
      button(
        t('delete', 'Elimina'),
        async () => {
          if (confirm('Eliminare questo profilo?')) {
            await api(`profiles/${p.id}`, 'DELETE');
            closeModal();
          }
        },
        'danger',
      ),
    );
}
/** Filtra localmente gli elementi data-search, così la ricerca non richiede nuove chiamate al server. */
function search() {
  const input = el('input', {
    placeholder: 'Cerca…',
    type: 'search',
    oninput: (e) => {
      searchByPage[page] = e.target.value;
      applyRowFilters();
    },
  });
  input.value = searchByPage[page] || '';
  return input;
}
/** Delega la cancellazione alla stessa API delle altre operazioni per condividere autenticazione ed errori. */
async function remove(k, id) {
  if (confirm('Eliminare questo elemento?')) await api(k + '/' + id, 'DELETE');
}
/** Mostra ordine, abilitazione e profili dei video per preparare la sequenza prima della diretta. */
function playlist(root) {
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Importa cartella', () => {
        openModal(
          'Importa video dalla cartella',
          el(
            'div',
            { class: 'fields' },
            field('Percorso assoluto Linux', 'path'),
            field(
              'Canale',
              'channelId',
              data.channels?.[0]?.id || 0,
              'select',
              (data.channels || []).map((c) => [c.id, c.channelName]),
            ),
          ),
          async () => {
            await api('scan', 'POST', { path: value('path'), channelId: number('channelId') });
            closeModal();
          },
        );
      }),
      button('Aggiungi file', () =>
        editVideo({
          enabled: true,
          channelId: data.channels?.[0]?.id,
          order: (data.videos || []).length,
        }),
      ),
      search(),
    ),
  );
  const table = el(
    'table',
    {},
    el(
      'thead',
      {},
      el(
        'tr',
        {},
        ...['Video', 'Canale', 'Abilitato', 'Ripresa', 'Azioni'].map((x) => el('th', {}, x)),
      ),
    ),
  );
  const body = el('tbody');
  const rows = [...(data.videos || [])].sort((a, b) => (a.order ?? a.id) - (b.order ?? b.id));
  for (const [i, v] of rows.entries()) {
    const name = data.channels?.find((c) => c.id === v.channelId)?.channelName || '—';
    body.append(
      el(
        'tr',
        { 'data-search': v.name || v.videoPath },
        el('td', { class: 'path' }, v.name || v.videoPath),
        el('td', {}, name),
        el(
          'td',
          {},
          el('input', {
            type: 'checkbox',
            checked: v.enabled !== false,
            onchange: (e) =>
              action(() => api('videos/' + v.id, 'PUT', { ...v, enabled: e.target.checked })),
          }),
        ),
        el('td', {}, (v.offsetSeconds || 0) + ' s'),
        el(
          'td',
          {},
          button('Modifica', () => editVideo(v)),
          button('Anteprima', () =>
            openModal(
              v.name || 'Video',
              el('video', { src: '/api/video/' + v.id + '/play', controls: '', class: 'preview' }),
            ),
          ),
          button('Proxy', async () => {
            const job = await api('proxy', 'POST', { target: v.videoPath });
            notify('Proxy: ' + job.status);
          }),
          button('↑', async () => {
            if (i) {
              await api('videos/' + v.id, 'PUT', { ...v, order: i - 1 });
              await api('videos/' + rows[i - 1].id, 'PUT', { ...rows[i - 1], order: i });
            }
          }),
          button('↓', async () => {
            if (i < rows.length - 1) {
              await api('videos/' + v.id, 'PUT', { ...v, order: i + 1 });
              await api('videos/' + rows[i + 1].id, 'PUT', { ...rows[i + 1], order: i });
            }
          }),
          button('Elimina', () => remove('videos', v.id), 'danger'),
        ),
      ),
    );
  }
  table.append(body);
  root.append(table);
}
/** Modifica il record del video senza ricopiare il media, separando metadati e file locale. */
function editVideo(v) {
  openModal(
    'Video della playlist',
    el(
      'div',
      { class: 'fields' },
      field('Nome', 'name', v.name),
      field('Percorso assoluto', 'videoPath', v.videoPath),
      field(
        'Canale',
        'channelId',
        v.channelId,
        'select',
        (data.channels || []).map((c) => [c.id, c.channelName]),
      ),
      field('Profilo per video', 'profileId', v.profileId || 0, 'select', [
        [0, 'Profilo della diretta'],
        ...(data.profiles || []).map((p) => [p.id, p.title]),
      ]),
      field('Riprendi da (secondi)', 'offsetSeconds', v.offsetSeconds || 0, 'number'),
      check('Abilitato', 'enabled', v.enabled !== false),
    ),
    async () => {
      await api('probe?target=' + encodeURIComponent(value('videoPath')));
      const out = {
        ...v,
        name: value('name') || value('videoPath').split('/').pop(),
        videoPath: value('videoPath'),
        channelId: number('channelId'),
        profileId: number('profileId'),
        offsetSeconds: number('offsetSeconds'),
        enabled: checked('enabled'),
      };
      await api('videos' + (v.id ? '/' + v.id : ''), v.id ? 'PUT' : 'POST', out);
      closeModal();
    },
  );
}
/** Mostra solo gli scheletri di layout come MainLayoutList.cshtml su Windows. */
function layoutList(root) {
  if (renderWindowsPage(root, 'MainLayoutList')) {
    const list = root.querySelector('[data-layout-list]');
    const layouts = (data.scenes || []).filter((scene) => scene.isLayout);
    if (!layouts.length) empty(list, t('noDataPresent', 'Nessun layout salvato'));
    for (const layout of layouts)
      list.append(
        el(
          'article',
          { class: 'card' },
          el('h3', {}, layout.name),
          ...(layout.description ? [el('p', {}, layout.description)] : []),
          el(
            'div',
            { class: 'actions' },
            button(t('seeDetails', 'Apri'), () => compose(structuredClone(layout))),
            button('Usa layout', () =>
              compose({
                ...structuredClone(layout),
                id: undefined,
                pkid: undefined,
                isLayout: false,
                name: layout.name + ' · scena',
              }),
            ),
            button(t('delete', 'Elimina'), () => remove('scenes', layout.id), 'danger'),
          ),
        ),
      );
    return;
  }
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Scene', () => navigate('scenes')),
      button('Overlay', () => navigate('overlays')),
    ),
  );
  scenes(root, true);
}
/** Elenca scene e layout riutilizzabili per aprire l’editor o avviare una composizione salvata. */
function scenes(root, layoutsOnly = false) {
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      ...(layoutsOnly
        ? []
        : [
            button('Nuova scena', () =>
              compose({ name: 'Nuova scena', width: 1280, height: 720, items: [] }),
            ),
          ]),
      button('Nuovo layout', () =>
        compose({ name: 'Nuovo layout', width: 1280, height: 720, isLayout: true, items: [] }),
      ),
      search(),
    ),
  );
  const grid = el('div', { class: 'grid' });
  for (const s of (data.scenes || []).filter((scene) => !layoutsOnly || scene.isLayout))
    grid.append(
      el(
        'article',
        { class: 'card', 'data-search': s.name },
        el('h3', {}, s.name),
        el('span', { class: 'badge' }, s.isLayout ? 'Layout' : 'Scena'),
        el('p', {}, `${s.width || 1280} × ${s.height || 720} · ${s.items.length} sorgenti`),
        el(
          'div',
          { class: 'actions' },
          button('Componi', () => compose(structuredClone(s))),
          button('Duplica', () =>
            compose({
              ...structuredClone(s),
              id: undefined,
              pkid: undefined,
              name: s.name + ' copia',
            }),
          ),
          button(s.isLayout ? 'Usa layout' : 'Avvia', () =>
            s.isLayout
              ? compose({
                  ...structuredClone(s),
                  id: undefined,
                  pkid: undefined,
                  isLayout: false,
                  name: s.name + ' · scena',
                })
              : startWizard(0, s.id),
          ),
          button('Elimina', () => remove('scenes', s.id), 'danger'),
        ),
      ),
    );
  root.append(grid);
}
/** Modifica la bozza di scena ricevuta e la salva esplicitamente, mantenendo separati editor e persistenza. */
function compose(scene, options = {}) {
  let selected = -1,
    drag = null;
  const canvas = el('div', { class: 'canvas', tabindex: 0 });
  canvas.style.aspectRatio = `${scene.width}/${scene.height}`;
  const inspector = el('div', { class: 'inspector' });
  const settings = el(
    'div',
    { class: 'fields' },
    field('Nome', 'name', scene.name),
    field('Descrizione', 'description', scene.description),
    field('Canvas: larghezza', 'width', scene.width, 'number'),
    field('Canvas: altezza', 'height', scene.height, 'number'),
    check('Layout: salva slot vuoti e overlay', 'isLayout', scene.isLayout),
  );
  const thumbs = new Map();
  /** Riprende keepInside di composer.js: drag e frecce devono produrre rettangoli salvabili nel canvas. */
  function keepInside(item) {
    item.width = Math.min(scene.width, Math.max(16, Math.round(item.width)));
    item.height = Math.min(scene.height, Math.max(16, Math.round(item.height)));
    item.x = Math.min(scene.width - item.width, Math.max(0, Math.round(item.x)));
    item.y = Math.min(scene.height - item.height, Math.max(0, Math.round(item.y)));
  }
  /** Memorizza le anteprime per sorgente per evitare una nuova elaborazione FFmpeg a ogni movimento del mouse. */
  function thumbnail(item) {
    if (!item.sourceTarget || item.sourceKind === 'MICROPHONE' || item.sourceKind === 'OVERLAY')
      return '';
    const key = item.sourceKind + item.sourceTarget;
    if (!thumbs.has(key)) {
      thumbs.set(key, '');
      fetch(
        '/api/snapshot?kind=' +
          encodeURIComponent(item.sourceKind) +
          '&target=' +
          encodeURIComponent(item.sourceTarget),
      )
        .then((r) => (r.ok ? r.blob() : null))
        .then((blob) => {
          if (blob && modalOpen) {
            thumbs.set(key, URL.createObjectURL(blob));
            draw();
          }
        })
        .catch(() => {});
    }
    return thumbs.get(key);
  }
  $('#modal').addEventListener(
    'close',
    () => {
      for (const url of thumbs.values()) if (url) URL.revokeObjectURL(url);
      thumbs.clear();
    },
    { once: true },
  );
  canvas.addEventListener('dragover', (e) => e.preventDefault());
  canvas.addEventListener('drop', (e) => {
    e.preventDefault();
    action(async () => {
      for (const file of e.dataTransfer.files) {
        const response = await fetch(
          '/api/media/scene-media?name=' + encodeURIComponent(file.name),
          { method: 'POST', headers: { 'X-Orbis-Token': token }, body: file },
        );
        const media = await response.json();
        if (!response.ok) throw Error(media.message);
        scene.items.push({
          sourceKind: 'FILE',
          sourceTarget: media.path,
          label: file.name,
          x: 0,
          y: 0,
          width: scene.width,
          height: scene.height,
          audioEnabled: true,
          volume: 100,
        });
        selected = scene.items.length - 1;
      }
      draw();
      inspect();
    });
  });
  /** Ridisegna le sorgenti in coordinate percentuali; la cattura del puntatore sul canvas sopravvive alla ricostruzione dei tile. */
  function draw() {
    canvas.replaceChildren();
    scene.items.forEach((item, i) => {
      if (item.sourceKind === 'MICROPHONE') return;
      const tile = el(
        'div',
        {
          class: 'tile' + (i === selected ? ' selected' : ''),
          onpointerdown: (e) => {
            selected = i;
            drag = {
              i,
              x: e.clientX,
              y: e.clientY,
              ox: item.x,
              oy: item.y,
              ow: item.width,
              oh: item.height,
              resize: e.target.classList.contains('handle'),
            };
            canvas.setPointerCapture(e.pointerId);
            inspect();
            draw();
            e.preventDefault();
          },
        },
        el('span', {}, item.label || item.sourceTarget?.split('/').pop() || 'Slot'),
        el('div', { class: 'handle' }),
      );
      Object.assign(tile.style, {
        left: (item.x / scene.width) * 100 + '%',
        top: (item.y / scene.height) * 100 + '%',
        width: (item.width / scene.width) * 100 + '%',
        height: (item.height / scene.height) * 100 + '%',
      });
      const thumb = thumbnail(item);
      if (thumb) tile.prepend(el('img', { src: thumb }));
      if (item.sourceKind === 'OVERLAY' && item.mediaName)
        tile.prepend(
          el('img', { src: '/api/media/overlays/' + encodeURIComponent(item.mediaName) }),
        );
      canvas.append(tile);
    });
  }
  /** Sincronizza i campi con la sorgente selezionata per modificare numericamente la stessa geometria del canvas. */
  function inspect() {
    inspector.replaceChildren(
      el('strong', {}, 'Sorgenti'),
      ...scene.items.map((x, i) =>
        button(i + 1 + '. ' + (x.label || x.sourceKind), () => {
          selected = i;
          draw();
          inspect();
        }),
      ),
    );
    if (selected < 0) return;
    const item = scene.items[selected];
    for (const [key, label] of [
      ['label', 'Nome'],
      ['sourceTarget', 'Percorso / dispositivo'],
      ['x', 'X'],
      ['y', 'Y'],
      ['width', 'Larghezza'],
      ['height', 'Altezza'],
      ['volume', 'Volume 0–200%'],
    ]) {
      const f = field(
        label,
        key,
        item[key] ?? (key === 'volume' ? 100 : ''),
        ['x', 'y', 'width', 'height', 'volume'].includes(key) ? 'number' : 'text',
      );
      f.querySelector('input').addEventListener('input', (e) => {
        item[key] = ['x', 'y', 'width', 'height', 'volume'].includes(key)
          ? Number(e.target.value)
          : e.target.value;
        draw();
      });
      inspector.append(f);
    }
    const au = check('Audio abilitato', 'tileAudio', item.audioEnabled);
    au.querySelector('input').addEventListener(
      'change',
      (e) => (item.audioEnabled = e.target.checked),
    );
    inspector.append(
      au,
      el(
        'div',
        { class: 'actions' },
        button('Centra', () => {
          item.x = Math.round((scene.width - item.width) / 2);
          item.y = Math.round((scene.height - item.height) / 2);
          draw();
          inspect();
        }),
        button('Porta avanti', () => {
          scene.items.splice(selected, 1);
          scene.items.push(item);
          selected = scene.items.length - 1;
          draw();
          inspect();
        }),
        button(
          'Rimuovi',
          () => {
            scene.items.splice(selected, 1);
            selected = -1;
            draw();
            inspect();
          },
          'danger',
        ),
      ),
    );
  }
  canvas.addEventListener('pointermove', (e) => {
    if (!drag) return;
    const box = canvas.getBoundingClientRect(),
      dx = ((e.clientX - drag.x) * scene.width) / box.width,
      dy = ((e.clientY - drag.y) * scene.height) / box.height,
      item = scene.items[drag.i];
    if (drag.resize) {
      item.width = Math.max(16, Math.round(drag.ow + dx));
      item.height = Math.max(16, Math.round(drag.oh + dy));
    } else {
      item.x = Math.round(drag.ox + dx);
      item.y = Math.round(drag.oy + dy);
    }
    keepInside(item);
    draw();
  });
  canvas.addEventListener('pointerup', () => {
    drag = null;
    inspect();
  });
  canvas.addEventListener('pointercancel', () => {
    drag = null;
    inspect();
  });
  canvas.addEventListener('keydown', (e) => {
    if (selected < 0) return;
    const item = scene.items[selected],
      step = e.shiftKey ? 10 : 1;
    if (e.key === 'Delete') {
      scene.items.splice(selected, 1);
      selected = -1;
    } else if (e.key === 'ArrowLeft') item.x -= step;
    else if (e.key === 'ArrowRight') item.x += step;
    else if (e.key === 'ArrowUp') item.y -= step;
    else if (e.key === 'ArrowDown') item.y += step;
    else return;
    if (selected >= 0) keepInside(item);
    e.preventDefault();
    draw();
    inspect();
  });
  /** Crea la sorgente del tipo scelto usando i dispositivi disponibili, mantenendo comune il modello della scena. */
  async function addSource(kind) {
    let target = '',
      label = kind;
    if (kind === 'FILE') {
      target = prompt('Percorso assoluto del file sul computer Linux:') || '';
      if (!target) return;
      await api('probe?target=' + encodeURIComponent(target));
    } else if (kind === 'OVERLAY') {
      const list = await api('media/overlays');
      if (!list.length) throw Error('Carica prima un file nella libreria overlay');
      const name = prompt(
        list.map((v, i) => `${i + 1}: ${v.name}`).join('\n') + '\nNumero overlay:',
      );
      const file = list[Number(name) - 1];
      if (!file) return;
      target = file.path;
      label = file.name;
      scene.items.push({
        sourceKind: kind,
        sourceTarget: target,
        mediaName: file.name,
        label,
        x: 0,
        y: 0,
        width: scene.width,
        height: scene.height,
        audioEnabled: false,
      });
      selected = scene.items.length - 1;
      draw();
      inspect();
      return;
    } else if (kind !== 'SLOT') {
      const available = sourceCatalog.filter((s) => s.kind === kind);
      target =
        prompt(
          available.map((s) => s.label + ': ' + s.target).join('\n') + '\nDispositivo:',
          available[0]?.target || '',
        ) || '';
      if (!target) return;
    }
    scene.items.push({
      sourceKind: kind === 'SLOT' ? 'FILE' : kind,
      sourceTarget: target,
      label,
      x: 0,
      y: 0,
      width: scene.width,
      height: scene.height,
      audioEnabled: kind !== 'SLOT',
      volume: 100,
    });
    selected = scene.items.length - 1;
    draw();
    inspect();
  }
  const toolbar = el(
    'div',
    { class: 'toolbar' },
    ...['FILE', 'SCREEN', 'CAMERA', 'MICROPHONE', 'OVERLAY', 'SLOT'].map((k, i) =>
      button(['File', 'Schermo X11', 'Camera', 'Microfono', 'Overlay', 'Slot'][i], () =>
        addSource(k),
      ),
    ),
    button('JSON avanzato', () => {
      const text = prompt('Scena JSON:', JSON.stringify(scene));
      if (text) {
        const parsed = JSON.parse(text);
        Object.assign(scene, parsed);
        draw();
        inspect();
      }
    }),
  );
  for (const k of ['width', 'height'])
    settings.querySelector(`[name=${k}]`).addEventListener('change', (e) => {
      scene[k] = Number(e.target.value);
      if (Number.isFinite(scene[k]) && scene[k] >= 16) scene.items.forEach(keepInside);
      canvas.style.aspectRatio = `${scene.width}/${scene.height}`;
      draw();
    });
  openModal(
    'Compositore',
    el(
      'div',
      {},
      settings,
      el(
        'p',
        {},
        'Trascina e ridimensiona le sorgenti. Frecce: spostamento; Shift: 10 pixel; Canc: rimuovi.',
      ),
      toolbar,
      el('div', { class: 'composer' }, canvas, inspector),
    ),
    async () => {
      scene.name = value('name');
      scene.description = value('description');
      scene.isLayout = checked('isLayout');
      const saved = await api(
        'scenes' + (scene.id ? '/' + scene.id : ''),
        scene.id ? 'PUT' : 'POST',
        scene,
      );
      scene.id = saved.id;
      if (options.onSaved) await options.onSaved(scene);
      closeModal();
    },
    options.onSaved ? t('startLiveButton', 'Avvia Live') : t('save', 'Salva'),
  );
  if (options.onBack) $('#modal-footer').prepend(button(t('back', 'Indietro'), options.onBack));
  draw();
  inspect();
}
/** Usa il selettore del browser e restituisce il file scelto per condividere il caricamento tra librerie. */
async function pickFile(accept) {
  return new Promise((resolve) => {
    const picker = $('#file-picker');
    picker.accept = accept;
    picker.value = '';
    picker.onchange = () => resolve(picker.files[0]);
    picker.oncancel = () => resolve(null);
    picker.click();
  });
}
/** Invia i byte del file al server locale, perché il browser non espone un percorso del filesystem utilizzabile dal motore. */
async function upload(type) {
  const file = await pickFile('image/*,video/*');
  if (!file) return;
  const response = await fetch('/api/media/' + type + '?name=' + encodeURIComponent(file.name), {
    method: 'POST',
    headers: { 'X-Orbis-Token': token },
    body: file,
  });
  const r = await response.json();
  if (!response.ok) throw Error(r.message);
  return r;
}
/** Mostra i media caricati e le azioni della libreria per riutilizzarli nelle scene. */
function overlays(root) {
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Carica overlay', () => upload('overlays')),
    ),
  );
  const grid = el('div', { class: 'grid' });
  root.append(grid);
  api('media/overlays')
    .then((list) => {
      for (const v of list) {
        const media = v.name.endsWith('.webm')
          ? el('video', {
              src: '/api/media/overlays/' + v.name,
              controls: '',
              loop: '',
              class: 'preview',
            })
          : el('img', { src: '/api/media/overlays/' + v.name, class: 'preview', alt: v.name });
        grid.append(
          el(
            'article',
            { class: 'card' },
            media,
            el('p', {}, v.name),
            el('small', {}, (v.bytes / 1024 / 1024).toFixed(2) + ' MB'),
            button('Elimina', () => remove('media/overlays', v.name), 'danger'),
          ),
        );
      }
      if (!list.length) empty(grid, 'Carica PNG, GIF, WebP o WebM con trasparenza.');
    })
    .catch((e) => notify(e.message));
}
/** Elenca i comandi scena separatamente dalle scene, consentendo più modalità di attivazione dello stesso contenuto. */
function buttons(root) {
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Nuovo pulsante', () =>
        editButton({
          position: (data.buttons || []).length,
          label: 'Intermezzo',
          displayMode: 'fullscreen',
          durationSeconds: 0,
        }),
      ),
    ),
  );
  const grid = el('div', { class: 'grid' });
  for (const b of data.buttons || [])
    grid.append(
      el(
        'article',
        { class: 'card' },
        el('h3', {}, `${b.position + 1}. ${b.label}`),
        el('p', {}, b.hotkey || 'Nessuna scorciatoia'),
        el('span', { class: 'badge' }, b.displayMode),
        el(
          'div',
          { class: 'actions' },
          button('Modifica', () => editButton(b)),
          button('Elimina', () => remove('buttons', b.id), 'danger'),
        ),
      ),
    );
  root.append(grid);
}
/** Configura modalità, durata e scorciatoia del comando per rendere ripetibile il suo utilizzo in regia. */
function editButton(b) {
  const content = el(
    'div',
    { class: 'fields' },
    field('Etichetta', 'label', b.label),
    field('Posizione (0–5)', 'position', b.position, 'number'),
    field('File della libreria', 'mediaName', b.mediaName),
    button('Carica video / immagine', async () => {
      const media = await upload('scene-media');
      if (media) document.querySelector('#modal [name=mediaName]').value = media.name;
    }),
    field('Scorciatoia (es. Ctrl+Digit1)', 'hotkey', b.hotkey),
    field('Modalità', 'displayMode', b.displayMode, 'select', [
      ['fullscreen', 'Schermo intero'],
      ['in_scene', 'Overlay nella scena'],
    ]),
    field(
      'Posizione overlay',
      'placement',
      b.placement || 'bottom-right',
      'select',
      ['bottom-right', 'bottom-left', 'top-right', 'top-left', 'center', 'custom'].map((x) => [
        x,
        x,
      ]),
    ),
    field(
      'Durata secondi (0: fino a ripresa / fine clip)',
      'durationSeconds',
      b.durationSeconds,
      'number',
    ),
    ...['x', 'y', 'width', 'height'].map((k, i) =>
      field(['X', 'Y', 'Larghezza', 'Altezza'][i], k, b[k] ?? (i > 1 ? 320 : 0), 'number'),
    ),
  );
  openModal('Pulsante scena', content, async () => {
    const v = { ...b };
    for (const k of ['label', 'mediaName', 'hotkey', 'displayMode', 'placement']) v[k] = value(k);
    for (const k of ['position', 'durationSeconds', 'x', 'y', 'width', 'height']) v[k] = number(k);
    await api('buttons' + (b.id ? '/' + b.id : ''), b.id ? 'PUT' : 'POST', v);
    closeModal();
  });
}
/** Riprende _StartWizardSetting.cshtml: la prima scelta contiene solo i profili attivi. */
function wizardSettings(draft) {
  const root = el('div');
  renderWindowsPage(root, '_StartWizardSetting');
  const profiles = (data.profiles || []).filter((profile) => profile.isActive);
  if (!profiles.some((profile) => profile.id === draft.profileId))
    draft.profileId = profiles[0]?.id || 0;
  for (const profile of profiles) {
    const input = el('input', {
      type: 'radio',
      name: 'settingId',
      value: profile.id,
      checked: profile.id === draft.profileId,
    });
    input.addEventListener('change', () => {
      draft.profileId = profile.id;
    });
    root
      .querySelector('[data-wizard-settings]')
      .append(
        el(
          'label',
          { class: 'row-card choice' },
          input,
          el(
            'span',
            {},
            el('strong', {}, profile.title),
            el(
              'small',
              {},
              ` · ${profile.videoCodecName || 'libx264'} · ${profile.videoBitrate} kbps · ${profile.videoWidth}×${profile.videoHeight} · ${profile.frameRate} fps`,
            ),
          ),
        ),
      );
  }
  root.querySelector('[data-wizard-no-settings]').hidden = profiles.length > 0;
  root.querySelector('[data-wizard-open-profiles]').addEventListener('click', () => {
    closeModal();
    navigate('profiles');
  });
  return root;
}
/** Riprende _StartWizardConfiguration.cshtml: canali attivi e chiave temporanea per TikTok. */
function wizardConfiguration(draft) {
  const root = el('div');
  renderWindowsPage(root, '_StartWizardConfiguration');
  const channels = (data.channels || []).filter((channel) => channel.isActive !== false);
  if (!channels.some((channel) => channel.id === draft.channelId))
    draft.channelId = channels[0]?.id || 0;
  const keyLabel = root.querySelector('[data-wizard-live-key]');
  const keyInput = keyLabel.querySelector('input');
  keyInput.value = draft.streamKey || '';
  const updateKey = () => {
    const temporary = asksForKey(channels.find((channel) => channel.id === draft.channelId) || {});
    keyLabel.hidden = !temporary;
    keyInput.disabled = !temporary;
    keyInput.required = temporary;
    if (!temporary) keyInput.value = '';
  };
  for (const channel of channels) {
    const input = el('input', {
      type: 'radio',
      name: 'configurationId',
      value: channel.id,
      checked: channel.id === draft.channelId,
    });
    input.addEventListener('change', () => {
      draft.channelId = channel.id;
      draft.streamKey = '';
      keyInput.value = '';
      updateKey();
    });
    root
      .querySelector('[data-wizard-configurations]')
      .append(
        el(
          'label',
          { class: 'row-card choice' },
          input,
          el(
            'span',
            {},
            el('strong', {}, channel.channelName),
            el('small', {}, ` · ${channel.platform || 'Custom'} · ${channel.streamUrl}`),
            el('p', {}, channel.description || ''),
          ),
        ),
      );
  }
  root.querySelector('[data-wizard-no-configurations]').hidden = channels.length > 0;
  root.querySelector('[data-wizard-open-channels]').addEventListener('click', () => {
    closeModal();
    navigate('channels');
  });
  updateKey();
  return root;
}
/** Valida le due scelte prima di cambiare dialogo, conservando una chiave temporanea solo in memoria. */
function wizardRequest(draft) {
  if (!draft.profileId || !draft.channelId)
    throw Error('Scegli un profilo attivo e una configurazione canale attiva');
  const channel = data.channels?.find((item) => item.id === draft.channelId);
  draft.streamKey = value('streamKey');
  if (asksForKey(channel || {}) && !draft.streamKey.trim())
    throw Error('La chiave TikTok di questa diretta è richiesta');
  return { channelId: draft.channelId, profileId: draft.profileId, streamKey: draft.streamKey };
}
/** Sequenza Windows: profilo, destinazione, compositore; Indietro conserva tutte le scelte. */
function startWizard(channelId = 0, sceneId = 0) {
  if (page !== 'live') navigate('live');
  const draft = {
    channelId,
    profileId: data.channels?.find((channel) => channel.id === channelId)?.profileId,
    streamKey: '',
  };
  let scene = sceneId ? structuredClone(data.scenes?.find((item) => item.id === sceneId)) : null;
  const settingsStep = () => {
    openModal(
      t('startNewLiveTitle', 'Nuova diretta'),
      wizardSettings(draft),
      configurationStep,
      t('next', 'Avanti'),
    );
    $('#modal-footer .primary').disabled = !draft.profileId;
  };
  const configurationStep = () => {
    const content = wizardConfiguration(draft);
    content.append(
      field('Scena salvata', 'sceneId', scene?.id || 0, 'select', [
        [0, 'Nuova composizione'],
        ...(data.scenes || []).filter((item) => !item.isLayout).map((item) => [item.id, item.name]),
      ]),
    );
    openModal(
      t('startNewLiveTitle', 'Nuova diretta'),
      content,
      () => {
        const request = wizardRequest(draft);
        const chosen = number('sceneId');
        if (chosen && chosen !== scene?.id)
          scene = structuredClone(data.scenes.find((item) => item.id === chosen));
        if (!scene) {
          const profile = data.profiles.find((item) => item.id === draft.profileId);
          const portrait = asksForKey(data.channels.find((item) => item.id === draft.channelId));
          const width = Number(profile.videoWidth) || 1280,
            height = Number(profile.videoHeight) || 720;
          scene = {
            name: 'Nuova diretta',
            width: portrait ? Math.min(width, height) : width,
            height: portrait ? Math.max(width, height) : height,
            items: [],
            isLayout: false,
          };
        }
        compose(scene, {
          onBack: configurationStep,
          onSaved: async (saved) => {
            const started = await api('live/start', 'POST', {
              ...request,
              sceneId: saved.id,
              loop: true,
            });
            watched = started.id;
            navigate('live');
          },
        });
      },
      t('composeNext', 'Componi'),
    );
    $('#modal-footer').prepend(
      button(t('back', 'Indietro'), () => {
        draft.streamKey = value('streamKey');
        settingsStep();
      }),
    );
    $('#modal-footer .primary').disabled = !draft.channelId;
  };
  settingsStep();
}
/** Avvio playlist Windows con gli stessi due pannelli e la cartella; scansione e validazione precedono il countdown. */
function playlistStartWizard() {
  const draft = {};
  const settings = wizardSettings(draft),
    configuration = wizardConfiguration(draft);
  const channel = data.channels?.find((item) => item.id === draft.channelId);
  const content = el(
    'div',
    {},
    settings,
    configuration,
    field(t('playlistFolder', 'Cartella video'), 'videoFolder', channel?.videoFolder || ''),
    el(
      'details',
      {},
      el('summary', {}, 'Opzioni playlist'),
      check('Ripeti playlist', 'loop', false),
      field('Countdown annullabile (secondi)', 'countdown', 3, 'number'),
    ),
  );
  openModal(
    t('startPlaylistTitle', 'Avvia playlist'),
    content,
    async () => {
      const request = wizardRequest(draft);
      const folder = value('videoFolder').trim();
      if (!folder) throw Error('Scegli la cartella dei video');
      const files = await api('scan', 'POST', { path: folder, preview: true });
      if (!files.length) throw Error('La cartella non contiene video riproducibili');
      request.files = files.map((file) => file.videoPath);
      request.loop = checked('loop');
      const seconds = Math.min(60, Math.max(0, Math.floor(number('countdown'))));
      const body = $('#modal-body'),
        footer = $('#modal-footer');
      const generation = modalGeneration;
      const oldBody = [...body.childNodes],
        oldFooter = [...footer.childNodes];
      footer.replaceChildren(button(t('cancel', 'Annulla'), closeModal));
      for (
        let remaining = seconds;
        remaining > 0 && modalOpen && generation === modalGeneration;
        remaining--
      ) {
        body.replaceChildren(
          el(
            'div',
            { class: 'empty' },
            el('div', { class: 'metric' }, remaining),
            el('p', {}, 'Avvio della playlist…'),
          ),
        );
        await new Promise((resolve) => setTimeout(resolve, 1000));
      }
      if (!modalOpen || generation !== modalGeneration) return;
      try {
        const started = await api('live/start', 'POST', request);
        watched = started.id;
        closeModal();
        navigate('live');
      } catch (error) {
        body.replaceChildren(...oldBody);
        footer.replaceChildren(...oldFooter);
        throw error;
      }
    },
    t('startPlaylistButton', 'Avvia playlist'),
  );
  $('#modal-footer .primary').disabled = !draft.profileId || !draft.channelId;
}
/** Modifica solo i parametri che il produttore FFmpeg Linux può applicare durante una diretta. */
function editLiveParameters(session) {
  const profile = session.profile || {};
  openModal(
    t('videoParameters', 'Parametri video'),
    el(
      'div',
      { class: 'fields' },
      field('Encoder H.264', 'videoCodecName', profile.videoCodecName || 'libx264', 'select', [
        ['libx264', 'x264'],
        ['h264_nvenc', 'NVENC'],
        ['h264_qsv', 'QSV'],
      ]),
      field(t('width', 'Larghezza'), 'videoWidth', profile.videoWidth || 1280, 'number'),
      field(t('height', 'Altezza'), 'videoHeight', profile.videoHeight || 720, 'number'),
      field(t('frameRate', 'Frame rate'), 'frameRate', profile.frameRate || 30, 'number'),
      field(
        t('bitrate', 'Bitrate') + ' kbps',
        'videoBitrate',
        profile.videoBitrate || 4500,
        'number',
      ),
      field(
        t('audioBitrate', 'Bitrate audio') + ' kbps',
        'audioBitrate',
        profile.audioBitrate || 128,
        'number',
      ),
    ),
    async () => {
      const updated = {
        ...profile,
        videoCodecName: value('videoCodecName'),
        videoWidth: number('videoWidth'),
        videoHeight: number('videoHeight'),
        frameRate: number('frameRate'),
        videoBitrate: number('videoBitrate'),
        audioBitrate: number('audioBitrate'),
      };
      await api(`live/${session.id}/parameters`, 'POST', updated);
      closeModal();
    },
  );
}
/** Chiede lo stato della pagina pubblica solo quando l'utente apre il player della piattaforma. */
async function showPlatformPlayer(session) {
  const player = await api('platform/' + session.channelId);
  const content = el('div', {}, el('p', {}, player.reason));
  if (player.embed)
    content.append(
      el('iframe', {
        src: player.embed,
        width: '100%',
        height: '450',
        allow: 'autoplay; fullscreen',
        allowfullscreen: '',
        title: 'Diretta pubblica',
      }),
    );
  if (player.publicUrl)
    content.append(
      el(
        'a',
        { href: player.publicUrl, target: '_blank', rel: 'noopener noreferrer' },
        t('openLivePage', 'Apri pagina pubblica'),
      ),
    );
  if (player.air === 'UNVERIFIED' && !sessionStorage.getItem('orbis-air-' + session.id))
    content.append(
      button('Chiudi avviso per questa diretta', () => {
        sessionStorage.setItem('orbis-air-' + session.id, '1');
        closeModal();
      }),
    );
  openModal(t('liveNow', 'Live ora'), content);
}
/** Riempie MainPreview.html con la sessione scelta, mantenendo URL, selettore e comandi Windows. */
function renderPreviewTemplate(root, running) {
  const view = root.querySelector('[data-preview]');
  if (!running.length) return;
  const requested = Number(new URLSearchParams(location.search).get('live'));
  const selected = running.find((session) => session.id === (requested || watched)) || running[0];
  watched = selected.id;
  const channel = data.channels?.find((item) => item.id === selected.channelId);
  root.querySelector('[data-preview-empty]').hidden = true;
  root.querySelector('[data-preview-actions]').hidden = false;
  view.hidden = false;
  view.dataset.pkid = selected.id;
  const picker = root.querySelector('[data-live-picker]');
  picker.hidden = running.length <= 1;
  if (!picker.hidden) {
    const select = picker.querySelector('select');
    select.replaceChildren(
      ...running.map((session) =>
        el(
          'option',
          { value: session.id },
          data.channels?.find((item) => item.id === session.channelId)?.channelName ||
            `Live ${session.id}`,
        ),
      ),
    );
    select.value = String(selected.id);
    select.addEventListener('change', () => {
      watched = Number(select.value);
      window.history.replaceState({}, '', `${routes.preview}?live=${watched}`);
      render();
    });
  }
  const frame = view.querySelector('[data-preview-frame]');
  frame.dataset.liveFrame = selected.id;
  frame.src = `/api/live/${selected.id}/frame?ts=${Date.now()}`;
  const profile = selected.profile || {};
  view.querySelector('[data-preview-name]').textContent = selected.scene?.name || 'Playlist';
  view.querySelector('[data-preview-channel]').textContent = channel?.channelName || '';
  view.querySelector('[data-preview-position]').textContent =
    `${selected.position + 1}/${selected.total} · ${Math.floor(selected.offsetSeconds || 0)} s`;
  view.querySelector('[data-preview-resolution]').textContent =
    `${profile.videoWidth || '—'} × ${profile.videoHeight || '—'}`;
  view.querySelector('[data-preview-fps]').textContent = `${profile.frameRate || '—'} fps`;
  view.querySelector('[data-preview-codec]').textContent = profile.videoCodecName || '—';
  view.querySelector('[data-preview-bitrate]').textContent =
    `${Math.round(selected.outputKbps || 0)} / ${profile.videoBitrate || '—'} kbps`;
  root.querySelector('[data-preview-stop]').addEventListener('click', () =>
    action(async () => {
      if (confirm(t('stopLive', 'Fermare la diretta?')))
        await api(`live/${selected.id}/stop`, 'POST', {});
    }),
  );
  root.querySelector('[data-preview-spot]').addEventListener('click', () =>
    action(async () => {
      const target = prompt(t('spotFile', 'Percorso video spot:'));
      if (target)
        await api(`live/${selected.id}/spot`, 'POST', {
          scene: {
            name: 'Spot',
            items: [{ sourceKind: 'FILE', sourceTarget: target, audioEnabled: true }],
          },
        });
    }),
  );
  root.querySelector('[data-preview-public]').hidden = !selected.channelId;
  root
    .querySelector('[data-preview-public]')
    .addEventListener('click', () => action(() => showPlatformPlayer(selected)));
  view
    .querySelector('[data-preview-deck-open]')
    .addEventListener('click', () => livePanel(selected));
  view
    .querySelector('[data-preview-parameters]')
    .addEventListener('click', () => editLiveParameters(selected));
  view
    .querySelector('[data-preview-next]')
    .addEventListener('click', () => action(() => api(`live/${selected.id}/next`, 'POST', {})));
  const deck = view.querySelector('[data-scene-strip-grid]');
  for (const item of data.buttons || [])
    deck.append(
      button(item.label, async () => {
        if (confirm('Mandare in onda ' + item.label + '?'))
          await api(`live/${selected.id}/button`, 'POST', { id: item.id });
      }),
    );
  deck.append(
    button(t('sceneResume', 'Riprendi live'), () => api(`live/${selected.id}/resume`, 'POST', {})),
  );
  const rows = view.querySelector('[data-mixer-rows]');
  (selected.scene?.items || []).forEach((item, index) => {
    if (item.sourceKind === 'OVERLAY') return;
    const volume = el('input', {
      type: 'range',
      min: 0,
      max: 200,
      step: 5,
      value: item.volume ?? 100,
      'aria-label': item.label || `Sorgente ${index + 1}`,
    });
    const enabled = el('input', { type: 'checkbox', checked: item.audioEnabled !== false });
    const output = el('output', {}, `${item.volume ?? 100}%`);
    const apply = () =>
      action(async () => {
        await api(`live/${selected.id}/volume`, 'POST', {
          index,
          volume: Number(volume.value),
          audioEnabled: enabled.checked,
        });
      });
    volume.addEventListener('change', apply);
    enabled.addEventListener('change', apply);
    volume.addEventListener('input', () => (output.textContent = `${volume.value}%`));
    rows.append(
      el(
        'label',
        { class: 'mixer-row' },
        el(
          'span',
          {},
          item.label || item.sourceTarget?.split('/').pop() || `Sorgente ${index + 1}`,
        ),
        enabled,
        volume,
        output,
      ),
    );
  });
}
/** Seleziona e mostra una live, come la pagina Anteprima di Windows. */
function preview(root) {
  const running = (data.live || []).filter((s) =>
    ['LIVE', 'STARTING', 'RECONNECTING'].includes(s.status),
  );
  if (renderWindowsPage(root, 'MainPreview')) {
    renderPreviewTemplate(root, running);
    return;
  }
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button(t('sceneDeck', 'Pulsanti scena'), () => navigate('buttons')),
    ),
  );
  if (!running.length) {
    empty(root, t('noLiveRunning', 'Nessuna live in corso'));
    root.append(button(t('startLiveButton', 'Avvia Live'), () => startWizard()));
    return;
  }
  const selected = running.find((s) => s.id === watched) || running[0];
  watched = selected.id;
  if (running.length > 1) {
    const picker = el(
      'select',
      { 'aria-label': t('runningLives', 'Live in corso') },
      ...running.map((s) =>
        el(
          'option',
          { value: s.id },
          data.channels?.find((c) => c.id === s.channelId)?.channelName || `Live ${s.id}`,
        ),
      ),
    );
    picker.value = String(selected.id);
    picker.addEventListener('change', () => {
      watched = Number(picker.value);
      render();
    });
    root.append(el('label', {}, t('runningLives', 'Live in corso'), picker));
  }
  root.append(
    el('img', {
      class: 'preview',
      'data-live-frame': selected.id,
      src: `/api/live/${selected.id}/frame?ts=${Date.now()}`,
      alt: t('livePreviewTitle', 'Anteprima live'),
    }),
  );
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button(t('sceneConfigure', 'Regia e mixer'), () => livePanel(selected)),
      button(
        t('stopLiveIconLabel', 'Ferma live'),
        () => api(`live/${selected.id}/stop`, 'POST', {}),
        'danger',
      ),
    ),
  );
}
/** Mostra le sessioni attive e i controlli di regia usando gli snapshot del motore. */
function live(root) {
  if (renderWindowsPage(root, 'MainLive')) {
    renderLiveRows(root);
    return;
  }
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Playlist', () => navigate('playlist')),
      button(t('liveHistory', 'Storico Live'), () => navigate('history')),
    ),
  );
  const grid = el('div', { class: 'grid' });
  for (const s of data.live || []) {
    const c = data.channels?.find((c) => c.id === s.channelId),
      running = ['LIVE', 'STARTING', 'RECONNECTING'].includes(s.status);
    const card = el(
      'article',
      { class: 'card' },
      el(
        'div',
        { class: 'actions' },
        el('h3', {}, c?.channelName || 'Diretta ' + s.id),
        el('span', { class: 'badge ' + s.status }, s.status),
        el('span', { class: 'badge' }, Math.round(s.outputKbps || 0) + ' kbps'),
      ),
      el(
        'p',
        {},
        `${s.scene?.name || 'Playlist'} · ${s.position + 1}/${s.total} · ${Math.floor(s.offsetSeconds || 0)} s`,
      ),
    );
    if (running)
      card.append(
        el('img', {
          class: 'preview',
          'data-live-frame': s.id,
          src: `/api/live/${s.id}/frame?ts=${Date.now()}`,
          alt: 'Anteprima composizione',
        }),
      );
    if (s.error) card.append(el('p', {}, s.error));
    card.append(
      el(
        'div',
        { class: 'actions' },
        ...(running
          ? [
              button('Regia e mixer', () => livePanel(s)),
              button('Player pubblico', async () => {
                const player = await api('platform/' + s.channelId);
                const content = el('div', {}, el('p', {}, player.reason));
                if (player.embed)
                  content.append(
                    el('iframe', {
                      src: player.embed,
                      width: '100%',
                      height: '450',
                      allow: 'autoplay; fullscreen',
                      allowfullscreen: '',
                      title: 'Diretta pubblica',
                    }),
                  );
                if (player.publicUrl)
                  content.append(
                    el(
                      'a',
                      { href: player.publicUrl, target: '_blank', rel: 'noopener noreferrer' },
                      'Apri pagina pubblica',
                    ),
                  );
                if (player.air === 'UNVERIFIED' && !sessionStorage.getItem('orbis-air-' + s.id))
                  content.append(
                    button('Chiudi avviso per questa diretta', () => {
                      sessionStorage.setItem('orbis-air-' + s.id, '1');
                      closeModal();
                    }),
                  );
                openModal('Player pubblico', content);
              }),
              button('Successivo', () => api(`live/${s.id}/next`, 'POST', {})),
              button('Ferma', () => api(`live/${s.id}/stop`, 'POST', {}), 'danger'),
            ]
          : []),
        button('Registro', async () => {
          const r = await fetch('/api/logs/' + s.id);
          openModal('Registro FFmpeg', el('pre', {}, await r.text()));
        }),
      ),
    );
    grid.append(card);
  }
  root.append(grid);
  if (!data.live?.length) empty(root, 'Nessuna diretta. Avvia un canale dalla toolbar.');
}
/** Riprende filtri e tabella di MainLive.cshtml: una playlist occupa una riga e dieci righe una pagina. */
function renderLiveRows(root) {
  const params = new URLSearchParams(location.search);
  const rows = data.liveRows || [];
  const fields = Object.fromEntries(
    ['liveStatus', 'channelName', 'platform'].map((key) => [
      key,
      root.querySelector(`[name=${key}]`),
    ]),
  );
  const channels = [
    ...new Set([
      ...(data.channels || []).map((channel) => channel.channelName),
      ...rows.map((row) => row.channelName),
      params.get('channelName'),
    ]),
  ]
    .filter(Boolean)
    .sort();
  for (const name of channels) fields.channelName.append(el('option', { value: name }, name));
  for (const [key, input] of Object.entries(fields)) {
    input.value = params.get(key) || '';
    input.addEventListener('change', () => {
      const next = new URLSearchParams(location.search);
      if (input.value) next.set(key, input.value);
      else next.delete(key);
      next.delete('p');
      window.history.replaceState({}, '', `${routes.live}${next.size ? '?' + next : ''}`);
      render();
    });
  }
  const table = root.querySelector('[data-live-table]');
  renderWindowsPage(table, '_LiveRows');
  const filtered = rows.filter(
    (row) =>
      (!params.get('liveStatus') || row.status === params.get('liveStatus')) &&
      (!params.get('channelName') || row.channelName === params.get('channelName')) &&
      (!params.get('platform') ||
        row.platform?.toLowerCase() === params.get('platform').toLowerCase()),
  );
  const totalPages = Math.ceil(filtered.length / 10);
  const requested = Number(params.get('p'));
  const pageNumber = Math.min(
    Math.max(0, Number.isSafeInteger(requested) ? requested : 0),
    Math.max(0, totalPages - 1),
  );
  const body = table.querySelector('[data-live-rows]');
  for (const row of filtered.slice(pageNumber * 10, pageNumber * 10 + 10)) {
    const session = data.live?.find((item) => item.id === row.id);
    const controls = el('div', { class: 'actions' });
    const addControl = (label, run, disabled = false, cls = '') => {
      const control = button(label, run, cls);
      control.disabled = disabled;
      controls.append(control);
    };
    addControl(t(row.isScene ? 'sceneDetails' : 'playlistDetails', 'Dettagli'), () =>
      openLiveDetails(row),
    );
    addControl(t('connectVideoSetting', 'Profilo video'), () => editHistoryProfile(row));
    if (row.running && session) {
      addControl(t('preview', 'Anteprima'), () => {
        watched = row.id;
        navigate('preview');
      });
      addControl(t('sceneDeckOpen', 'Regia e mixer'), () => livePanel(session));
      addControl(t('playSpot', 'Spot'), async () => {
        const path = prompt('Percorso del video spot:');
        if (path)
          await api(`live/${row.id}/spot`, 'POST', {
            scene: {
              name: 'Spot',
              items: [{ sourceKind: 'FILE', sourceTarget: path, audioEnabled: true }],
            },
          });
      });
      addControl(
        t('stopLiveIconLabel', 'Ferma'),
        async () => {
          if (confirm('Fermare la diretta?')) await api(`live/${row.id}/stop`, 'POST', {});
        },
        false,
        'danger',
      );
      if (session.takeover?.items?.length)
        addControl(t('sceneResume', 'Riprendi'), () => api(`live/${row.id}/resume`, 'POST', {}));
      if (row.channelId) addControl(row.platform || 'Player', () => showPlatformPlayer(session));
    }
    addControl(t('startLiveIconLabel', 'Riprendi'), () => replayHistory(row), row.running);
    addControl(t('restartLiveIconLabel', 'Ricomincia'), async () => {
      if (!confirm('Ricominciare dal primo video?')) return;
      if (row.running) await api(`live/${row.id}/stop`, 'POST', {});
      await replayHistory(row, true);
    });
    addControl(t('delete', 'Elimina'), () => remove('history', row.id), row.running, 'danger');
    body.append(
      el(
        'tr',
        { 'data-live-row': row.id },
        el('td', {}, el('span', { class: `badge ${row.status}` }, row.status)),
        el(
          'td',
          {},
          el('strong', {}, row.title || `Live ${row.id}`),
          ...(row.isPlaylist
            ? [el('span', { class: 'badge' }, ` ${row.position + 1}/${row.total}`)]
            : []),
          el('p', { class: 'path' }, `${row.platform || ''} · ${row.source || ''}`),
        ),
        el('td', {}, row.channelName || '—'),
        el('td', {}, new Date(row.startedAt).toLocaleString()),
        el('td', {}, controls),
      ),
    );
  }
  table.querySelector('[data-live-empty]').hidden = filtered.length > 0;
  const pager = table.querySelector('[data-live-pager]');
  for (const index of [...new Set([0, pageNumber - 1, pageNumber, pageNumber + 1, totalPages - 1])]
    .filter((index) => index >= 0 && index < totalPages)
    .sort((a, b) => a - b))
    pager.append(
      button(
        String(index + 1),
        () => {
          const next = new URLSearchParams(location.search);
          if (index) next.set('p', String(index));
          else next.delete('p');
          window.history.replaceState({}, '', `${routes.live}${next.size ? '?' + next : ''}`);
          render();
        },
        index === pageNumber ? 'primary' : '',
      ),
    );
}
/** Riprende la richiesta salvata; un replay usa video/offset interrotti, un restart torna al primo file. */
async function replayHistory(item, beginning = false, fromIndex = null) {
  const request = structuredClone(item.request);
  if (!request) throw Error('Snapshot della diretta non disponibile');
  const channel = data.channels?.find((value) => value.id === item.channelId);
  if (
    asksForKey(
      channel || { platform: item.platform || request.platform, streamUrl: request.streamUrl },
    )
  ) {
    const key = prompt('Nuova chiave TikTok per questa diretta:');
    if (!key?.trim()) return;
    request.streamKey = key.trim();
  }
  const restart = beginning || item.status === 'ENDED';
  request.startIndex = request.scene
    ? 0
    : (fromIndex ??
      (restart ? 0 : Math.min(Math.max(0, item.position || 0), (request.files?.length || 1) - 1)));
  request.offsetSeconds = fromIndex !== null || restart ? 0 : item.offsetSeconds || 0;
  const started = await api('live/start', 'POST', request);
  watched = started.id;
  closeModal();
  navigate('live');
}
/** Dettagli della playlist dalla stessa riga, con riproduzione locale e ripartenza da un video scelto. */
function openLiveDetails(row) {
  const content = el('div', {}, el('p', {}, `${row.channelName || '—'} · ${row.status}`));
  const files = row.request?.files || [];
  if (files.length) {
    const table = el(
      'table',
      {},
      el(
        'thead',
        {},
        el('tr', {}, ...['Video', 'Stato', 'Azioni'].map((label) => el('th', {}, label))),
      ),
    );
    files.forEach((path, index) => {
      const state =
        index < row.position ? 'ENDED' : index === row.position ? row.status : 'OFFLINE';
      const resume = button('Riparti da questo video', () => replayHistory(row, false, index));
      resume.disabled = row.running;
      table.append(
        el(
          'tr',
          {},
          el('td', { class: 'path' }, path),
          el('td', {}, state),
          el(
            'td',
            {},
            button('Riproduci', () =>
              openModal(
                path.split('/').pop(),
                el('video', {
                  controls: '',
                  src: `/api/history/${row.id}/video/${index}`,
                  style: 'width:100%',
                }),
              ),
            ),
            resume,
          ),
        ),
      );
    });
    content.append(table);
  } else content.append(el('pre', {}, JSON.stringify(row.request?.scene || {}, null, 2)));
  openModal(t(row.isScene ? 'sceneDetails' : 'playlistDetails', 'Dettagli diretta'), content);
}
/** Collega un profilo alla richiesta persistita; se la live è in corso aggiorna anche il produttore. */
function editHistoryProfile(row) {
  const content = field(
    t('videoSettings', 'Profilo video'),
    'linkedProfile',
    row.request?.profile?.id || row.request?.profileId || 0,
    'select',
    (data.profiles || []).map((profile) => [profile.id, profile.title]),
  );
  openModal(t('connectVideoSetting', 'Collega profilo video'), content, async () => {
    const profile = data.profiles.find((item) => item.id === number('linkedProfile'));
    if (!profile) throw Error('Profilo non disponibile');
    if (row.running) await api(`live/${row.id}/parameters`, 'POST', profile);
    const history = await api(`history/${row.id}`);
    history.request.profile = structuredClone(profile);
    history.request.profileId = profile.id;
    await api(`history/${row.id}`, 'PUT', history);
    closeModal();
  });
}
/** Aggiorna le immagini senza sovrapporre fetch per la stessa anteprima, contenendo il traffico durante il polling. */
function updatePreviews() {
  document.querySelectorAll('[data-live-frame]').forEach((img) => {
    if (!img.dataset.busy) {
      img.dataset.busy = '1';
      img.onload = img.onerror = () => delete img.dataset.busy;
      img.src = `/api/live/${img.dataset.liveFrame}/frame?ts=${Date.now()}`;
    }
  });
}
/** Apre i comandi della sessione scelta e attiva le sue scorciatoie solo mentre il pannello è in uso. */
function livePanel(s) {
  deckActive = true;
  watched = s.id;
  const root = el(
    'div',
    {},
    el('img', {
      class: 'preview',
      'data-live-frame': s.id,
      src: `/api/live/${s.id}/frame`,
      alt: 'Anteprima diretta',
    }),
    el(
      'p',
      {},
      'I cambi scena mantengono il publisher. La ripresa torna alla posizione del programma.',
    ),
  );
  const deck = el('div', { class: 'toolbar' });
  for (const b of data.buttons || [])
    deck.append(
      button(b.label + (b.hotkey ? ' · ' + b.hotkey : ''), async () => {
        if (confirm('Mandare in onda ' + b.label + '?'))
          await api(`live/${s.id}/button`, 'POST', { id: b.id });
      }),
    );
  deck.append(button('Riprendi programma', () => api(`live/${s.id}/resume`, 'POST', {})));
  root.append(deck);
  (s.scene?.items || []).forEach((item, i) => {
    const slider = el('input', { type: 'range', min: 0, max: 200, value: item.volume ?? 100 });
    const enabled = el('input', { type: 'checkbox', checked: item.audioEnabled !== false });
    const label = el('span', {}, (item.volume ?? 100) + '%');
    const apply = () =>
      action(async () => {
        label.textContent = slider.value + '%';
        await api(`live/${s.id}/volume`, 'POST', {
          index: i,
          volume: Number(slider.value),
          audioEnabled: enabled.checked,
        });
      });
    slider.addEventListener('change', apply);
    enabled.addEventListener('change', apply);
    root.append(
      el(
        'div',
        { class: 'card' },
        el('strong', {}, item.label || item.sourceTarget?.split('/').pop() || 'Sorgente'),
        el('label', { class: 'check' }, enabled, 'Audio'),
        slider,
        label,
      ),
    );
  });
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Cambia profilo in diretta', async () => {
        const n = prompt(
          data.profiles.map((p) => `${p.id}: ${p.title}`).join('\n') + '\nID profilo:',
        );
        const p = data.profiles.find((p) => p.id === Number(n));
        if (p) await api(`live/${s.id}/parameters`, 'POST', p);
      }),
      button('Spot in coda', async () => {
        const target = prompt('Percorso del video spot:');
        if (target)
          await api(`live/${s.id}/spot`, 'POST', {
            scene: {
              name: 'Spot',
              items: [{ sourceKind: 'FILE', sourceTarget: target, audioEnabled: true }],
            },
          });
      }),
      button('Vai al video', async () => {
        const i = prompt('Numero video (1–' + s.total + ')');
        if (i) await api(`live/${s.id}/jump`, 'POST', { index: Number(i) - 1 });
      }),
    ),
  );
  openModal('Regia · diretta ' + s.id, root);
}
document.addEventListener('keydown', (e) => {
  if (
    !deckActive ||
    !modalOpen ||
    !watched ||
    ['INPUT', 'TEXTAREA', 'SELECT'].includes(e.target.tagName)
  )
    return;
  const key =
    (e.ctrlKey ? 'Ctrl+' : '') + (e.altKey ? 'Alt+' : '') + (e.shiftKey ? 'Shift+' : '') + e.code;
  const b = data.buttons?.find((b) => b.hotkey === key);
  if (b) {
    e.preventDefault();
    action(async () => {
      if (confirm('Mandare in onda ' + b.label + '?'))
        await api(`live/${watched}/button`, 'POST', { id: b.id });
    });
  }
});
/** Scarica l'export usando il token API, che una navigazione diretta del browser non invierebbe. */
async function downloadExport() {
  const response = await fetch('/api/export', { headers: { 'X-Orbis-Token': token } });
  if (!response.ok) throw Error('Esportazione non riuscita');
  const url = URL.createObjectURL(await response.blob());
  const link = el('a', { href: url, download: 'orbis-linux.json' });
  link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
/** Aggiorna il progresso senza ricostruire i filtri o perdere le modifiche al modulo. */
function updateCleanupProgress() {
  const panel = $('[data-cleanup-progress]');
  if (!panel) return;
  const state = data.cleanupProgress || {};
  panel.hidden = !state.running && !state.error && !state.deletedLives;
  $('[data-history-cleanup]').disabled = Boolean(state.running);
  $('[data-cleanup-message]').textContent =
    state.error ||
    `${state.running ? 'Pulizia in corso' : 'Pulizia completata'}: ${state.deletedVideos || 0}/${state.totalVideos || 0} video, ${state.deletedLives || 0} dirette. ${state.currentVideo || ''}`;
  $('[data-cleanup-bar]').max = Math.max(1, state.totalVideos || 0);
  $('[data-cleanup-bar]').value = state.deletedVideos || 0;
}
/** Riprende i tre controlli Windows dalla prima configurazione canale attiva e ne salva solo i campi di pulizia. */
function configureHistoryCleanup(root) {
  const form = root.querySelector('[data-history-cleanup-settings]');
  const channel = data.channels?.find((item) => item.isActive !== false) || {};
  const enabled = form.elements.enabled;
  enabled.checked = channel.autoCleanupEnabled ?? channel.autoCleanupOlderThanMonths > 0;
  form.elements.intervalMonths.value = channel.autoCleanupIntervalMonths || 1;
  form.elements.olderThanMonths.value = channel.autoCleanupOlderThanMonths ?? 0;
  const showFields = () => {
    root.querySelector('[data-cleanup-fields]').hidden = !enabled.checked;
  };
  enabled.addEventListener('change', showFields);
  showFields();
  form.addEventListener('submit', (event) => {
    event.preventDefault();
    action(async () => {
      await api('history/cleanup/settings', 'POST', {
        enabled: enabled.checked,
        intervalMonths: Number(form.elements.intervalMonths.value),
        olderThanMonths: Number(form.elements.olderThanMonths.value),
      });
      notify('Impostazioni di pulizia salvate');
    });
  });
  root.querySelector('[data-history-cleanup]').addEventListener('click', () =>
    action(async () => {
      if (!confirm('Confermi la pulizia dello storico per il periodo selezionato?')) return;
      const response = await fetch('/api/history/cleanup', {
        method: 'POST',
        headers: { 'X-Orbis-Token': token, 'Content-Type': 'application/json' },
        body: JSON.stringify({ months: Number(form.elements.olderThanMonths.value), async: true }),
      });
      const result = await response.json();
      // Un lavoro già avviato si osserva invece di avviarne un secondo, come nella pagina Windows.
      if (!response.ok && response.status !== 409)
        throw Error(result.message || 'Pulizia non riuscita');
      data.cleanupProgress = result;
      updateCleanupProgress();
    }),
  );
  updateCleanupProgress();
}
/** Restringe e impagina gli snapshot come MainLiveHistory.cshtml, senza perdere i filtri nell'URL. */
function renderHistoryTemplate(root) {
  const params = new URLSearchParams(location.search);
  const channelOf = (item) =>
    item.channelName ||
    item.request?.channelName ||
    data.channels?.find((channel) => channel.id === item.channelId)?.channelName ||
    String(item.channelId || '—');
  const fields = {
    channelName: root.querySelector('[name=channelName]'),
    liveStatus: root.querySelector('[name=liveStatus]'),
    titleSearch: root.querySelector('[name=titleSearch]'),
  };
  const channels = [...new Set([...(data.history || []).map(channelOf), params.get('channelName')])]
    .filter(Boolean)
    .sort((a, b) => a.localeCompare(b));
  for (const name of channels) fields.channelName.append(el('option', { value: name }, name));
  for (const [key, input] of Object.entries(fields)) input.value = params.get(key) || '';
  const setFilter = (key, value) => {
    clearTimeout(historySearchTimer);
    const focused = document.activeElement === fields.titleSearch;
    const selection = [fields.titleSearch.selectionStart, fields.titleSearch.selectionEnd];
    const next = new URLSearchParams(location.search);
    value = value.trim();
    if (value) next.set(key, value);
    else next.delete(key);
    next.delete('p');
    window.history.replaceState({}, '', `${routes.history}${next.size ? '?' + next : ''}`);
    render();
    if (focused) {
      const input = $('#content [name=titleSearch]');
      input.focus();
      input.setSelectionRange(...selection);
    }
  };
  fields.channelName.addEventListener('change', () =>
    setFilter('channelName', fields.channelName.value),
  );
  fields.liveStatus.addEventListener('change', () =>
    setFilter('liveStatus', fields.liveStatus.value),
  );
  fields.titleSearch.addEventListener('input', () => {
    clearTimeout(historySearchTimer);
    const value = fields.titleSearch.value;
    historySearchTimer = setTimeout(() => {
      if (page === 'history') setFilter('titleSearch', value);
    }, 400);
  });
  root.querySelector('[data-history-clear]').addEventListener('click', () => {
    clearTimeout(historySearchTimer);
    window.history.replaceState({}, '', routes.history);
    render();
  });
  root
    .querySelector('[data-history-export]')
    .addEventListener('click', () => action(downloadExport));
  configureHistoryCleanup(root);
  const titleOf = (item) =>
    item.request?.scene?.name || item.request?.files?.[0]?.split('/').pop() || `Live ${item.id}`;
  const filters = Object.fromEntries(
    Object.keys(fields).map((key) => [key, (params.get(key) || '').trim()]),
  );
  const filtered = [...(data.history || [])]
    .sort(
      (a, b) => String(b.startedAt || '').localeCompare(String(a.startedAt || '')) || b.id - a.id,
    )
    .filter((item) => {
      const title = titleOf(item).toLowerCase();
      return (
        (!filters.channelName || channelOf(item) === filters.channelName) &&
        (!filters.liveStatus || item.status === filters.liveStatus) &&
        (!filters.titleSearch || title.includes(filters.titleSearch.toLowerCase()))
      );
    });
  const totalPages = Math.ceil(filtered.length / 20);
  const requestedPage = Number(params.get('p'));
  const pageNumber = Math.min(
    Math.max(0, Number.isSafeInteger(requestedPage) ? requestedPage : 0),
    Math.max(0, totalPages - 1),
  );
  const rows = root.querySelector('[data-history-rows]');
  for (const item of filtered.slice(pageNumber * 20, pageNumber * 20 + 20)) {
    const path = item.request?.files?.[0] || item.request?.scene?.items?.[0]?.sourceTarget || '—';
    const platform =
      item.platform ||
      item.request?.platform ||
      data.channels?.find((channel) => channel.id === item.channelId)?.platform ||
      '—';
    rows.append(
      el(
        'tr',
        {},
        el('td', {}, titleOf(item)),
        el('td', {}, el('span', { class: `badge ${item.status}` }, item.status)),
        el('td', {}, channelOf(item)),
        el('td', {}, platform),
        el('td', { class: 'path' }, path),
        el('td', {}, new Date(item.startedAt).toLocaleString()),
        el('td', {}, item.error || '—'),
        el(
          'td',
          { class: 'actions' },
          button('Riavvia', () => replayHistory(item)),
          button('Snapshot', () =>
            openModal('Snapshot della diretta', el('pre', {}, JSON.stringify(item, null, 2))),
          ),
          button(t('delete', 'Elimina'), () => remove('history', item.id), 'danger'),
        ),
      ),
    );
  }
  root.querySelector('[data-history-empty]').hidden = filtered.length > 0;
  const pager = root.querySelector('[data-history-pager]');
  // Limita i link per evitare una barra enorme quando lo storico contiene molti anni di dirette.
  for (const index of [...new Set([0, pageNumber - 1, pageNumber, pageNumber + 1, totalPages - 1])]
    .filter((index) => index >= 0 && index < totalPages)
    .sort((a, b) => a - b))
    pager.append(
      button(
        String(index + 1),
        () => {
          const next = new URLSearchParams(location.search);
          if (index) next.set('p', String(index));
          else next.delete('p');
          window.history.replaceState({}, '', `${routes.history}${next.size ? '?' + next : ''}`);
          render();
        },
        index === pageNumber ? 'primary' : '',
      ),
    );
}
/** Mostra gli esiti persistiti e consente riavvio o pulizia senza dipendere dalle sessioni ancora in memoria. */
function historyPage(root) {
  if (renderWindowsPage(root, 'MainLiveHistory')) {
    renderHistoryTemplate(root);
    return;
  }
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Esporta configurazione e storico', downloadExport),
      button('Pulisci storico', async () => {
        const months = prompt('Elimina le dirette terminate più vecchie di quanti mesi?', '3');
        if (months && confirm('Confermi la pulizia dello storico?')) {
          const r = await api('history/cleanup', 'POST', { months: Number(months) });
          notify(r.deleted + ' dirette eliminate');
        }
      }),
      search(),
    ),
  );
  const table = el(
    'table',
    {},
    el(
      'thead',
      {},
      el('tr', {}, ...['Avvio', 'Canale', 'Stato', 'Azioni'].map((t) => el('th', {}, t))),
    ),
  );
  for (const h of [...(data.history || [])].reverse())
    table.append(
      el(
        'tr',
        { 'data-search': h.startedAt + ' ' + h.status },
        el('td', {}, new Date(h.startedAt).toLocaleString()),
        el('td', {}, data.channels?.find((c) => c.id === h.channelId)?.channelName || h.channelId),
        el('td', {}, el('span', { class: 'badge ' + h.status }, h.status)),
        el(
          'td',
          {},
          button('Riavvia snapshot', async () => {
            const r = structuredClone(h.request);
            if (asksForKey(data.channels?.find((c) => c.id === h.channelId) || {}))
              r.streamKey = prompt('Nuova chiave TikTok:') || '';
            r.offsetSeconds = h.offsetSeconds || 0;
            const started = await api('live/start', 'POST', r);
            watched = started.id;
            navigate('live');
          }),
          button('Snapshot', () =>
            openModal('Snapshot della diretta', el('pre', {}, JSON.stringify(h, null, 2))),
          ),
          button('Elimina', () => remove('history', h.id), 'danger'),
        ),
      ),
    );
  root.append(table);
}
/** Visualizza metriche host e processi; i contatori indisponibili restano distinti dai valori reali pari a zero. */
function activity(root) {
  const m = data.metrics || {};
  root.append(
    el(
      'div',
      { class: 'grid' },
      ...Object.entries({
        CPU: m.cpu < 0 ? '—' : m.cpu?.toFixed(1) + '%',
        RAM: m.ram?.toFixed(1) + '%',
        Swap: m.swap?.toFixed(1) + '%',
        'Rete ↓': m.networkRx < 0 ? '—' : (m.networkRx / 1024).toFixed(0) + ' KiB/s',
        'Rete ↑': m.networkTx < 0 ? '—' : (m.networkTx / 1024).toFixed(0) + ' KiB/s',
        'Disco disponibile': (m.diskAvailable / 1024 ** 3).toFixed(1) + ' GiB',
        Uptime: Math.floor(m.uptime / 3600) + ' ore',
        'CPU logiche': m.threads,
        'Disco lettura': m.diskRead < 0 ? '—' : (m.diskRead / 1024 ** 2).toFixed(1) + ' MiB/s',
        'Disco scrittura': m.diskWrite < 0 ? '—' : (m.diskWrite / 1024 ** 2).toFixed(1) + ' MiB/s',
        GPU: m.gpu?.utilization === undefined ? '—' : m.gpu.utilization + '%',
      }).map(([key, v]) =>
        el('div', { class: 'card' }, el('small', {}, key), el('div', { class: 'metric' }, v)),
      ),
    ),
  );
  for (const t of m.temperatures || [])
    root.append(el('p', {}, `${t.label.trim()}: ${t.celsius.toFixed(1)} °C`));
  const table = el(
    'table',
    {},
    el(
      'tr',
      {},
      ...[
        'Diretta',
        'Producer PID',
        'CPU / RAM producer',
        'Publisher PID',
        'CPU / RAM publisher',
      ].map((s) => el('th', {}, s)),
    ),
  );
  for (const s of data.live || [])
    if (s.status === 'LIVE')
      table.append(
        el(
          'tr',
          {},
          el('td', {}, s.id),
          el('td', {}, s.producerPid),
          el(
            'td',
            {},
            s.producerMetrics?.cpu >= 0
              ? s.producerMetrics.cpu.toFixed(1) +
                  '% · ' +
                  Math.round(s.producerMetrics.rssBytes / 1024 ** 2) +
                  ' MiB'
              : '—',
          ),
          el('td', {}, s.publisherPid),
          el(
            'td',
            {},
            s.publisherMetrics?.cpu >= 0
              ? s.publisherMetrics.cpu.toFixed(1) +
                  '% · ' +
                  Math.round(s.publisherMetrics.rssBytes / 1024 ** 2) +
                  ' MiB'
              : '—',
          ),
        ),
      );
  root.append(el('h3', {}, 'Processi FFmpeg'), table);
}
/** Raccoglie preferenze, backup e manutenzione in una vista per gestire la configurazione locale. */
function settings(root) {
  root.append(
    el(
      'div',
      { class: 'card' },
      el('h3', {}, 'Sfondo panoramica'),
      button('Carica immagine di sfondo', async () => {
        const media = await upload('images');
        if (media)
          await api('preferences/1', 'PUT', {
            ...(data.preferences?.find((p) => p.id === 1) || {}),
            backgroundMediaName: media.name,
          });
      }),
      button('Rimuovi sfondo', () =>
        api('preferences/1', 'PUT', {
          ...(data.preferences?.find((p) => p.id === 1) || {}),
          backgroundMediaName: '',
        }),
      ),
    ),
  );
  root.append(
    el(
      'div',
      { class: 'card' },
      el('h3', {}, 'Aggiornamenti Linux'),
      field('URL manifest HTTPS', 'manifest', localStorage.getItem('orbis-manifest') || ''),
      button('Verifica aggiornamenti', async () => {
        const url = document.querySelector('#content [name=manifest]').value;
        localStorage.setItem('orbis-manifest', url);
        const update = await api('update', 'POST', { url });
        const content = el(
          'div',
          {},
          el('p', {}, 'Versione disponibile: ' + update.version),
          el(
            'a',
            { href: update.downloadUrl, target: '_blank', rel: 'noopener noreferrer' },
            'Scarica pacchetto Linux',
          ),
        );
        openModal('Aggiornamenti', content);
      }),
    ),
  );
  root.append(
    el(
      'div',
      { class: 'card' },
      el('h3', {}, 'Dati e configurazione'),
      el(
        'p',
        {},
        'Il database usa la cartella XDG Linux. L’esportazione esclude le chiavi di streaming.',
      ),
      el(
        'div',
        { class: 'actions' },
        button('Esporta JSON', () => {
          location.href = '/api/export';
        }),
        button('Importa JSON', async () => {
          const file = await pickFile('.json');
          if (
            file &&
            confirm('Sostituire canali, profili, scene e playlist con il file importato?')
          )
            await api('import', 'POST', JSON.parse(await file.text()));
        }),
      ),
    ),
    el(
      'div',
      { class: 'card' },
      el('h3', {}, 'Linux: sorgenti e accelerazione'),
      el(
        'p',
        {},
        'Fotocamere: V4L2. Audio: PulseAudio o PipeWire con compatibilità PulseAudio. Schermo: X11; su Wayland usa una sorgente video virtuale esterna. NVENC e QSV richiedono driver e un encoder funzionante sul sistema.',
      ),
      button('Proxy in preparazione', async () => {
        const jobs = await api('proxy');
        openModal(
          'Proxy video',
          el(
            'div',
            {},
            ...jobs.map((j) =>
              el(
                'div',
                { class: 'card' },
                el('p', {}, j.name + ' · ' + j.status + ' · ' + Math.floor(j.seconds) + ' s'),
                j.status === 'READY'
                  ? el('video', { src: '/api/proxy/' + j.name, controls: '', class: 'preview' })
                  : el('p', {}, j.error),
              ),
            ),
          ),
        );
      }),
      button('Rileva sorgenti', async () => {
        sourceCatalog = await api('sources');
        openModal('Sorgenti disponibili', el('pre', {}, JSON.stringify(sourceCatalog, null, 2)));
      }),
    ),
  );
}
/** Carica il manuale della lingua corrente e le risorse originali per mantenere la documentazione consultabile offline. */
async function manual(root) {
  root.append(
    el(
      'div',
      { class: 'toolbar' },
      button('Stampa / salva PDF', () => window.print()),
    ),
  );
  const guide = el(
    'div',
    { class: 'manual' },
    el(
      'article',
      {},
      el('h3', {}, 'OrbisStream Linux'),
      el(
        'p',
        {},
        'Crea un canale con server RTMP e chiave; configura un profilo H.264/AAC; importa una cartella nella playlist oppure componi una scena; avvia dalla toolbar. Il pannello Regia permette mixer, scene, ripresa, salto e spot.',
      ),
      el(
        'p',
        {},
        'Il manuale originale Windows seguente descrive anche funzioni avanzate ancora elencate nel documento MIGRAZIONE.md del port Linux. Gli screenshot rappresentano la versione Windows.',
      ),
    ),
  );
  root.append(guide);
  try {
    const r = await fetch('/resources/ui/manual.' + language + '.json');
    const doc = await r.json();
    for (const section of doc.sections || []) {
      const article = el(
        'article',
        {},
        el('h3', {}, section.title),
        el('p', {}, section.body),
        el('ol', {}, ...(section.steps || []).map((step) => el('li', {}, step))),
      );
      if (section.figure)
        article.append(
          el('img', {
            src: '/resources/manual/' + language + '/' + section.figure + '.webp',
            alt: section.callout || section.title,
          }),
          el('p', {}, section.callout || ''),
        );
      guide.append(article);
    }
  } catch (e) {
    guide.append(
      el('p', {}, 'Manuale originale non disponibile: consulta linux/docs/MIGRAZIONE.md.'),
    );
  }
}
window.addEventListener('beforeunload', (e) => {
  if ((data.live || []).some((s) => ['LIVE', 'STARTING', 'RECONNECTING'].includes(s.status))) {
    e.preventDefault();
    e.returnValue = '';
  }
});
$('#start').onclick = () => action(() => startWizard());
$('#stop-all').onclick = () =>
  action(async () => {
    if (confirm('Fermare tutte le dirette?')) await api('live/stop-all', 'POST', {});
  });
const languages = {
  it: 'Italiano',
  en: 'English',
  de: 'Deutsch',
  es: 'Español',
  fr: 'Français',
  pt: 'Português',
  ru: 'Русский',
  zh: '中文',
  ko: '한국어',
  ja: '日本語',
  hi: 'हिन्दी',
  ar: 'العربية',
  ro: 'Română',
  da: 'Dansk',
  nl: 'Nederlands',
  fil: 'Filipino',
  tr: 'Türkçe',
  la: 'Latina',
  tlh: 'Klingon',
  hod: 'Hodor',
  el: 'Ελληνικά',
};
$('#language').replaceChildren(
  ...Object.entries(languages).map(([key, name]) => el('option', { value: key }, name)),
);
$('#language').value = language;
/** Carica il dizionario selezionato con fallback, evitando che una lingua mancante impedisca l’avvio dell’interfaccia. */
async function loadLanguage() {
  try {
    const r = await fetch('/resources/ui/ui.' + language + '.json');
    bundle = await r.json();
    document.documentElement.lang = language;
    document.documentElement.dir = language === 'ar' ? 'rtl' : 'ltr';
  } catch (e) {
    bundle = {};
  }
}
$('#language').onchange = () =>
  action(async () => {
    language = $('#language').value;
    localStorage.setItem('orbis-language', language);
    await loadLanguage();
    render();
  });
$('#theme').value = localStorage.getItem('orbis-theme') || 'system';
/** Applica la preferenza memorizzata per conservare l’aspetto scelto tra le aperture del browser. */
function theme() {
  document.body.className = $('#theme').value === 'system' ? '' : $('#theme').value;
  localStorage.setItem('orbis-theme', $('#theme').value);
}
$('#theme').onchange = theme;
theme();
// Il bootstrap ottiene token e sorgenti prima del polling, così i pannelli ricevono uno stato iniziale completo.
(async () => {
  try {
    const boot = await api('bootstrap');
    token = boot.token;
    sourceCatalog = boot.sources;
    $('#version').textContent = 'v' + boot.version;
    page = pageFromLocation();
    if (location.pathname === '/' && !location.hash)
      window.history.replaceState({ page }, '', routes.dashboard);
    await loadLanguage();
    await loadPageTemplates();
    await refresh();
    if (
      location.pathname === routes.live &&
      new URLSearchParams(location.search).get('start') === '1'
    ) {
      window.history.replaceState({ page: 'live' }, '', routes.live);
      startWizard();
    }
    const changes = new EventSource('/api/updates?watch=rows');
    let queuedRefresh = false;
    changes.addEventListener('rows', () => {
      if (queuedRefresh) return;
      queuedRefresh = true;
      setTimeout(() => {
        queuedRefresh = false;
        refresh().catch((e) => notify(e.message));
      }, 50);
    });
    window.addEventListener('pagehide', () => changes.close(), { once: true });
    setInterval(() => {
      if (modalOpen) updatePreviews();
      else refresh().catch((e) => notify(e.message));
    }, 2000);
  } catch (e) {
    notify(e.message);
  }
})();
