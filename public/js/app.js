/**
 * TuneBox — Frontend Application Engine
 * Relative API requests and real backend state integration
 */

(function () {
  'use strict';

  // --- API Configuration ---
  const API = {
    state: 'api/state',
    tracks: 'api/tracks',
    search: 'api/search',
    playlists: 'api/playlists',
    queue: 'api/queue',
    player: 'api/player',
    history: 'api/history',
    liked: 'api/liked',
    reset: 'api/reset'
  };

  // --- Local State Cache (Mirrors backend) ---
  const state = {
    currentView: 'home',
    player: {
      current_track_id: '',
      is_playing: false,
      position_sec: 0,
      volume: 80,
      repeat_mode: 'off',
      shuffle: false,
      current_track: null
    },
    queue: [],
    history: [],
    playlists: [],
    likedTracks: [],
    allTracks: [],
    selectedPlaylistId: null,
    searchQuery: '',
    searchField: 'all',
    discoverGenre: 'all',
    discoverSort: 'title',
    libraryTab: 'all-tracks',
    isScrubbing: false
  };

  // --- DOM Elements ---
  const elements = {
    // Nav
    navItems: document.querySelectorAll('.nav-item'),
    sidebarPlaylists: document.getElementById('sidebar-playlist-list'),
    likedCountBadge: document.getElementById('liked-count-badge'),
    btnCreatePlaylist: document.getElementById('btn-create-playlist'),
    topBtnNewPlaylist: document.getElementById('top-btn-new-playlist'),
    pageTitle: document.getElementById('page-title'),

    // Views
    viewContainers: document.querySelectorAll('.view-container'),
    viewHome: document.getElementById('view-home'),
    viewDiscover: document.getElementById('view-discover'),
    viewSearch: document.getElementById('view-search'),
    viewLibrary: document.getElementById('view-library'),
    viewLiked: document.getElementById('view-liked'),
    viewHistory: document.getElementById('view-history'),
    viewPlaylistDetail: document.getElementById('view-playlist-detail'),

    // Home
    homeHero: document.getElementById('home-hero'),
    heroTrackTitle: document.getElementById('hero-track-title'),
    heroTrackArtist: document.getElementById('hero-track-artist'),
    btnHeroPlay: document.getElementById('btn-hero-play'),
    btnHeroQueue: document.getElementById('btn-hero-queue'),
    homeQuickGrid: document.getElementById('home-quick-grid'),
    homePlaylistsGrid: document.getElementById('home-playlists-grid'),
    homePopularTracks: document.getElementById('home-popular-tracks'),

    // Discover
    discoverGenreChips: document.getElementById('discover-genre-chips'),
    discoverCount: document.getElementById('discover-count'),
    discoverSortSelect: document.getElementById('discover-sort-select'),
    discoverTrackTable: document.getElementById('discover-track-table'),

    // Search
    mainSearchInput: document.getElementById('main-search-input'),
    btnClearSearchLarge: document.getElementById('btn-clear-search-large'),
    searchFilterPills: document.querySelectorAll('.filter-pill'),
    searchResultsContainer: document.getElementById('search-results-container'),
    searchResultsTitle: document.getElementById('search-results-title'),

    // Library
    libTabs: document.querySelectorAll('.lib-tab'),
    libraryTrackTable: document.getElementById('library-track-table'),
    libraryPlaylistsGrid: document.getElementById('library-playlists-grid'),
    libraryLikedTable: document.getElementById('library-liked-table'),

    // Liked
    likedSongsTable: document.getElementById('liked-songs-table'),
    btnPlayAllLiked: document.getElementById('btn-play-all-liked'),
    likedCollectionMeta: document.getElementById('liked-collection-meta'),

    // History
    historyTrackTable: document.getElementById('history-track-table'),

    // Playlist Detail
    playlistDetailCover: document.getElementById('playlist-detail-cover'),
    playlistDetailTitle: document.getElementById('playlist-detail-title'),
    playlistDetailDesc: document.getElementById('playlist-detail-desc'),
    playlistDetailMeta: document.getElementById('playlist-detail-meta'),
    btnPlayPlaylist: document.getElementById('btn-play-playlist'),
    btnEditPlaylistModal: document.getElementById('btn-edit-playlist-modal'),
    btnAddTracksModal: document.getElementById('btn-add-tracks-modal'),
    btnDeletePlaylist: document.getElementById('btn-delete-playlist'),
    playlistTrackTable: document.getElementById('playlist-track-table'),

    // Player Bar
    playerBar: document.getElementById('player-bar'),
    playerArtwork: document.getElementById('player-artwork'),
    playerTitle: document.getElementById('player-title'),
    playerArtist: document.getElementById('player-artist'),
    btnPlayerLike: document.getElementById('btn-player-like'),
    btnShuffle: document.getElementById('btn-shuffle'),
    btnPrev: document.getElementById('btn-prev'),
    btnPlayPause: document.getElementById('btn-play-pause'),
    iconPlay: document.getElementById('icon-play'),
    iconPause: document.getElementById('icon-pause'),
    btnNext: document.getElementById('btn-next'),
    btnRepeat: document.getElementById('btn-repeat'),
    repeatIndicator: document.getElementById('repeat-indicator'),
    currentTime: document.getElementById('current-time'),
    totalTime: document.getElementById('total-time'),
    progressBarFill: document.getElementById('progress-bar-fill'),
    progressScrubber: document.getElementById('progress-scrubber'),
    btnQueueToggle: document.getElementById('btn-queue-toggle'),
    playerQueueBadge: document.getElementById('player-queue-badge'),
    btnVolumeIcon: document.getElementById('btn-volume-icon'),
    iconVolHigh: document.getElementById('icon-vol-high'),
    iconVolMute: document.getElementById('icon-vol-mute'),
    volumeSlider: document.getElementById('volume-slider'),

    // Queue Drawer
    queueDrawer: document.getElementById('queue-drawer'),
    btnCloseQueue: document.getElementById('btn-close-queue'),
    btnClearQueue: document.getElementById('btn-clear-queue'),
    queueCurrentTrack: document.getElementById('queue-current-track'),
    queueList: document.getElementById('queue-list'),
    queueCountLabel: document.getElementById('queue-count-label'),

    // Audio Element
    audioPlayer: document.getElementById('audio-player'),

    // Modals
    playlistModal: document.getElementById('playlist-modal'),
    playlistModalTitle: document.getElementById('playlist-modal-title'),
    modalPlaylistName: document.getElementById('modal-playlist-name'),
    modalPlaylistDesc: document.getElementById('modal-playlist-desc'),
    modalGradientPicker: document.getElementById('modal-gradient-picker'),
    btnClosePlaylistModal: document.getElementById('btn-close-playlist-modal'),
    btnCancelPlaylistModal: document.getElementById('btn-cancel-playlist-modal'),
    btnSavePlaylistModal: document.getElementById('btn-save-playlist-modal'),

    addToPlaylistModal: document.getElementById('add-to-playlist-modal'),
    modalTracksToAdd: document.getElementById('modal-tracks-to-add'),
    btnCloseAddModal: document.getElementById('btn-close-add-modal'),
    btnCancelAddModal: document.getElementById('btn-cancel-add-modal'),

    // Toast Container
    toastContainer: document.getElementById('toast-container')
  };

  // --- Helper Functions ---
  function formatTime(seconds) {
    if (isNaN(seconds) || seconds < 0) return '0:00';
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${secs < 10 ? '0' : ''}${secs}`;
  }

  function showToast(message, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.innerHTML = `<span>${message}</span>`;
    elements.toastContainer.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(8px)';
      setTimeout(() => toast.remove(), 250);
    }, 2800);
  }

  async function apiFetch(url, options = {}) {
    try {
      const response = await fetch(url, {
        headers: {
          'Content-Type': 'application/json',
          ...(options.headers || {})
        },
        ...options
      });
      if (!response.ok) {
        const errorData = await response.json().catch(() => ({}));
        throw new Error(errorData.error || `HTTP error ${response.status}`);
      }
      return await response.json();
    } catch (err) {
      console.error(`API Error [${url}]:`, err);
      showToast(err.message, 'error');
      throw err;
    }
  }

  // --- Backend Sync ---
  async function fetchAppState() {
    try {
      const data = await apiFetch(API.state);
      state.player = data.player || state.player;
      state.queue = data.queue || [];
      state.history = data.history || [];
      state.playlists = data.playlists || [];
      state.likedTracks = data.liked || [];

      // Also refresh all tracks
      const tracks = await apiFetch(API.tracks);
      state.allTracks = tracks || [];

      renderAll();
    } catch (err) {
      console.warn('Initial sync error:', err);
    }
  }

  // --- View Navigation ---
  function navigateTo(viewName, playlistId = null) {
    state.currentView = viewName;
    if (playlistId) {
      state.selectedPlaylistId = playlistId;
    }

    elements.navItems.forEach(item => {
      if (item.dataset.view === viewName && !playlistId) {
        item.classList.add('active');
      } else {
        item.classList.remove('active');
      }
    });

    elements.viewContainers.forEach(container => {
      container.classList.remove('active');
    });

    const targetContainer = document.getElementById(`view-${viewName}`);
    if (targetContainer) {
      targetContainer.classList.add('active');
    }

    // Update title
    const titles = {
      home: 'Home',
      discover: 'Discover',
      search: 'Search',
      library: 'Your Library',
      liked: 'Liked Songs',
      history: 'Recently Played',
      'playlist-detail': 'Playlist Details'
    };
    elements.pageTitle.textContent = titles[viewName] || 'TuneBox';

    // Refresh specific view data if needed
    if (viewName === 'discover') {
      loadDiscoverTracks();
    } else if (viewName === 'search' && state.searchQuery) {
      performSearch();
    } else if (viewName === 'playlist-detail' && state.selectedPlaylistId) {
      loadPlaylistDetail(state.selectedPlaylistId);
    }
  }

  // --- Rendering Functions ---
  function renderAll() {
    renderSidebarPlaylists();
    renderPlayerBar();
    renderQueue();
    renderHomeView();
    renderLikedSongsView();
    renderHistoryView();
    renderLibraryView();
  }

  function renderSidebarPlaylists() {
    elements.sidebarPlaylists.innerHTML = '';
    state.playlists.forEach(pl => {
      const li = document.createElement('li');
      li.className = `playlist-item ${state.currentView === 'playlist-detail' && state.selectedPlaylistId === pl.id ? 'active' : ''}`;
      li.innerHTML = `
        <span class="playlist-dot" style="background: ${pl.cover_gradient || 'var(--accent-violet)'};"></span>
        <span>${escapeHtml(pl.title)}</span>
      `;
      li.addEventListener('click', () => {
        navigateTo('playlist-detail', pl.id);
      });
      elements.sidebarPlaylists.appendChild(li);
    });

    elements.likedCountBadge.textContent = state.likedTracks.length;
  }

  function renderPlayerBar() {
    const track = state.player.current_track;
    if (track) {
      elements.playerTitle.textContent = track.title;
      elements.playerArtist.textContent = `${track.artist} • ${track.album}`;
      elements.playerArtwork.style.background = track.artwork || 'var(--bg-surface-elevated)';
      elements.totalTime.textContent = formatTime(track.duration);

      const isLiked = state.likedTracks.some(t => t.id === track.id) || track.liked;
      elements.btnPlayerLike.className = `btn-like-player ${isLiked ? 'liked' : ''}`;
      elements.btnPlayerLike.dataset.trackId = track.id;

      // Sync HTML5 audio source if changed
      if (track.audio_url && (!elements.audioPlayer.src || !elements.audioPlayer.src.endsWith(track.audio_url.replace('./', '')))) {
        elements.audioPlayer.src = track.audio_url;
      }
    } else {
      elements.playerTitle.textContent = 'No track playing';
      elements.playerArtist.textContent = 'TuneBox Studio';
      elements.playerArtwork.style.background = 'var(--bg-surface-elevated)';
      elements.btnPlayerLike.className = 'btn-like-player';
      delete elements.btnPlayerLike.dataset.trackId;
    }

    // Play/Pause icon
    if (state.player.is_playing) {
      elements.iconPlay.style.display = 'none';
      elements.iconPause.style.display = 'block';
      if (elements.audioPlayer.src && elements.audioPlayer.paused) {
        elements.audioPlayer.play().catch(() => {});
      }
    } else {
      elements.iconPlay.style.display = 'block';
      elements.iconPause.style.display = 'none';
      if (!elements.audioPlayer.paused) {
        elements.audioPlayer.pause();
      }
    }

    // Shuffle & Repeat
    elements.btnShuffle.className = `btn-control-subtle ${state.player.shuffle ? 'active' : ''}`;
    elements.btnRepeat.className = `btn-control-subtle ${state.player.repeat_mode !== 'off' ? 'active' : ''}`;
    elements.repeatIndicator.textContent = state.player.repeat_mode === 'one' ? '1' : (state.player.repeat_mode === 'all' ? '•' : '');

    // Volume
    elements.volumeSlider.value = state.player.volume;
    elements.audioPlayer.volume = state.player.volume / 100;
    if (state.player.volume === 0) {
      elements.iconVolHigh.style.display = 'none';
      elements.iconVolMute.style.display = 'block';
    } else {
      elements.iconVolHigh.style.display = 'block';
      elements.iconVolMute.style.display = 'none';
    }

    // Queue Badge
    elements.playerQueueBadge.textContent = state.queue.length;
    elements.playerQueueBadge.style.display = state.queue.length > 0 ? 'flex' : 'none';
  }

  function renderQueue() {
    const current = state.player.current_track;
    if (current) {
      elements.queueCurrentTrack.innerHTML = `
        <div class="track-art-thumb" style="background: ${current.artwork};"></div>
        <div class="track-meta-col">
          <span class="track-title-text">${escapeHtml(current.title)}</span>
          <span class="track-artist-text">${escapeHtml(current.artist)}</span>
        </div>
        <span class="track-duration-text" style="margin-left: auto;">${formatTime(current.duration)}</span>
      `;
    } else {
      elements.queueCurrentTrack.innerHTML = `<span class="text-subtle">No track currently active</span>`;
    }

    elements.queueCountLabel.textContent = `${state.queue.length} track${state.queue.length === 1 ? '' : 's'}`;
    elements.queueList.innerHTML = '';

    if (state.queue.length === 0) {
      elements.queueList.innerHTML = `
        <div class="empty-state" style="padding: 24px 12px;">
          <p class="empty-state-title" style="font-size: 14px;">Queue is empty</p>
          <p class="empty-state-text" style="font-size: 12px;">Add tracks from library, search, or playlists to play next in FIFO order.</p>
        </div>
      `;
      return;
    }

    state.queue.forEach((track, index) => {
      const item = document.createElement('div');
      item.className = 'queue-list-item';
      item.innerHTML = `
        <span class="queue-pos">${index + 1}</span>
        <div class="track-art-thumb" style="background: ${track.artwork}; width: 32px; height: 32px;"></div>
        <div class="track-meta-col" style="flex: 1;">
          <span class="track-title-text" style="font-size: 13px;">${escapeHtml(track.title)}</span>
          <span class="track-artist-text" style="font-size: 11px;">${escapeHtml(track.artist)}</span>
        </div>
        <span class="track-duration-text" style="font-size: 11.5px;">${formatTime(track.duration)}</span>
        <button class="btn-remove-queue" data-track-id="${track.id}" title="Remove from queue">&times;</button>
      `;

      item.querySelector('.btn-remove-queue').addEventListener('click', async (e) => {
        e.stopPropagation();
        await removeFromQueue(track.id);
      });

      item.addEventListener('click', async () => {
        await playTrack(track.id);
      });

      elements.queueList.appendChild(item);
    });
  }

  function renderHomeView() {
    // 1. Featured Listening Hero
    if (state.allTracks.length > 0) {
      const featured = state.allTracks[0];
      elements.heroTrackTitle.textContent = featured.title;
      elements.heroTrackArtist.textContent = `${featured.artist} • ${featured.album} • ${featured.genre}`;
      elements.btnHeroPlay.onclick = () => playTrack(featured.id);
      elements.btnHeroQueue.onclick = () => enqueueTrack(featured.id);
    }

    // 2. Quick Grid (first 6 tracks)
    elements.homeQuickGrid.innerHTML = '';
    const quickTracks = state.allTracks.slice(0, 6);
    quickTracks.forEach(t => {
      const card = document.createElement('div');
      card.className = 'quick-card';
      card.innerHTML = `
        <div class="quick-art" style="background: ${t.artwork};">
          <svg viewBox="0 0 24 24" width="20" height="20" fill="currentColor">
            <polygon points="5 3 19 12 5 21 5 3"></polygon>
          </svg>
        </div>
        <span class="quick-title">${escapeHtml(t.title)}</span>
      `;
      card.addEventListener('click', () => playTrack(t.id));
      elements.homeQuickGrid.appendChild(card);
    });

    // 3. Playlists Cards
    elements.homePlaylistsGrid.innerHTML = '';
    state.playlists.forEach(pl => {
      const card = document.createElement('div');
      card.className = 'card';
      card.innerHTML = `
        <div class="card-artwork" style="background: ${pl.cover_gradient};">
          <button class="card-play-btn" title="Play playlist">
            <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
              <polygon points="5 3 19 12 5 21 5 3"></polygon>
            </svg>
          </button>
        </div>
        <div class="card-title">${escapeHtml(pl.title)}</div>
        <div class="card-desc">${escapeHtml(pl.description || `${pl.track_count} tracks`)}</div>
      `;
      card.querySelector('.card-play-btn').addEventListener('click', (e) => {
        e.stopPropagation();
        playPlaylist(pl.id);
      });
      card.addEventListener('click', () => {
        navigateTo('playlist-detail', pl.id);
      });
      elements.homePlaylistsGrid.appendChild(card);
    });

    // 4. Popular Tracks
    elements.homePopularTracks.innerHTML = '';
    const popularTracks = [...state.allTracks].sort((a, b) => b.play_count - a.play_count).slice(0, 5);
    popularTracks.forEach((t, idx) => {
      elements.homePopularTracks.appendChild(createTrackRow(t, idx + 1));
    });
  }

  function renderLikedSongsView() {
    elements.likedCollectionMeta.textContent = `${state.likedTracks.length} tracks • ${Math.round(state.likedTracks.reduce((acc, t) => acc + t.duration, 0) / 60)} min`;
    elements.likedSongsTable.innerHTML = '';

    if (state.likedTracks.length === 0) {
      elements.likedSongsTable.innerHTML = `
        <div class="empty-state">
          <p class="empty-state-title">No liked songs yet</p>
          <p class="empty-state-text">Tap the heart icon on any song you love to save it to this collection.</p>
        </div>
      `;
      return;
    }

    state.likedTracks.forEach((t, idx) => {
      elements.likedSongsTable.appendChild(createTrackRow(t, idx + 1));
    });
  }

  function renderHistoryView() {
    elements.historyTrackTable.innerHTML = '';
    if (state.history.length === 0) {
      elements.historyTrackTable.innerHTML = `
        <div class="empty-state">
          <p class="empty-state-title">No listening history</p>
          <p class="empty-state-text">Tracks you listen to will be recorded in this session history.</p>
        </div>
      `;
      return;
    }

    state.history.forEach((t, idx) => {
      elements.historyTrackTable.appendChild(createTrackRow(t, idx + 1));
    });
  }

  function renderLibraryView() {
    elements.libraryTrackTable.innerHTML = '';
    state.allTracks.forEach((t, idx) => {
      elements.libraryTrackTable.appendChild(createTrackRow(t, idx + 1));
    });

    elements.libraryPlaylistsGrid.innerHTML = elements.homePlaylistsGrid.innerHTML;
    // Re-bind listeners for library playlists
    elements.libraryPlaylistsGrid.querySelectorAll('.card').forEach((card, i) => {
      const pl = state.playlists[i];
      if (pl) {
        card.onclick = () => navigateTo('playlist-detail', pl.id);
      }
    });

    elements.libraryLikedTable.innerHTML = elements.likedSongsTable.innerHTML;
  }

  function createTrackRow(track, index, showReorder = false, playlistId = null) {
    const row = document.createElement('div');
    const isCurrent = state.player.current_track_id === track.id;
    row.className = `track-row ${isCurrent ? 'active' : ''}`;

    const isLiked = state.likedTracks.some(t => t.id === track.id) || track.liked;

    let reorderHtml = '';
    if (showReorder && playlistId) {
      reorderHtml = `
        <div class="reorder-btns">
          <button class="btn-reorder btn-move-up" title="Move Up">&uarr;</button>
          <button class="btn-reorder btn-move-down" title="Move Down">&darr;</button>
        </div>
      `;
    }

    row.innerHTML = `
      <span class="track-index">${index}</span>
      <div class="track-art-thumb" style="background: ${track.artwork};"></div>
      <div class="track-meta-col">
        <span class="track-title-text">${escapeHtml(track.title)}</span>
        <span class="track-artist-text">${escapeHtml(track.artist)}</span>
      </div>
      <span class="track-album-text">${escapeHtml(track.album)}</span>
      <span class="track-duration-text">${formatTime(track.duration)}</span>
      <div class="track-actions-col">
        <button class="btn-icon-subtle btn-row-like ${isLiked ? 'liked' : ''}" title="${isLiked ? 'Unlike' : 'Like'}">
          <svg class="heart-icon" viewBox="0 0 24 24" width="16" height="16" fill="${isLiked ? 'var(--accent-magenta)' : 'none'}" stroke="currentColor" stroke-width="2">
            <path d="M20.84 4.61a5.5 5.5 0 0 0-7.78 0L12 5.67l-1.06-1.06a5.5 5.5 0 0 0-7.78 7.78l1.06 1.06L12 21.23l7.78-7.78 1.06-1.06a5.5 5.5 0 0 0 0-7.78z"></path>
          </svg>
        </button>
        <button class="btn-icon-subtle btn-row-queue" title="Add to FIFO Queue">
          <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2">
            <line x1="12" y1="5" x2="12" y2="19"></line>
            <line x1="5" y1="12" x2="19" y2="12"></line>
          </svg>
        </button>
        ${reorderHtml}
        ${playlistId ? `<button class="btn-icon-subtle btn-row-remove-pl" title="Remove from playlist" style="color: #ef4444;">&times;</button>` : ''}
      </div>
    `;

    // Row click: play track
    row.addEventListener('click', (e) => {
      if (e.target.closest('button')) return;
      playTrack(track.id);
    });

    // Like button
    const likeBtn = row.querySelector('.btn-row-like');
    likeBtn.addEventListener('click', async (e) => {
      e.stopPropagation();
      await toggleLikeTrack(track.id);
    });

    // Queue button
    const queueBtn = row.querySelector('.btn-row-queue');
    queueBtn.addEventListener('click', async (e) => {
      e.stopPropagation();
      await enqueueTrack(track.id);
    });

    // Reorder buttons (Linked list order manipulation)
    if (showReorder && playlistId) {
      const btnUp = row.querySelector('.btn-move-up');
      const btnDown = row.querySelector('.btn-move-down');

      btnUp.addEventListener('click', async (e) => {
        e.stopPropagation();
        if (index > 1) {
          await reorderPlaylistTracks(playlistId, index - 1, index - 2);
        }
      });

      btnDown.addEventListener('click', async (e) => {
        e.stopPropagation();
        await reorderPlaylistTracks(playlistId, index - 1, index);
      });

      const removePlBtn = row.querySelector('.btn-row-remove-pl');
      if (removePlBtn) {
        removePlBtn.addEventListener('click', async (e) => {
          e.stopPropagation();
          await removeTrackFromPlaylist(playlistId, track.id);
        });
      }
    }

    return row;
  }

  // --- Playlist Detail Loading ---
  async function loadPlaylistDetail(playlistId) {
    try {
      const pl = await apiFetch(`${API.playlists}/${playlistId}`);
      elements.playlistDetailTitle.textContent = pl.title;
      elements.playlistDetailDesc.textContent = pl.description || 'No description provided.';
      elements.playlistDetailMeta.textContent = `${pl.track_count} tracks • ${Math.round(pl.total_duration / 60)} min`;
      elements.playlistDetailCover.style.background = pl.cover_gradient;

      elements.playlistTrackTable.innerHTML = '';
      if (!pl.tracks || pl.tracks.length === 0) {
        elements.playlistTrackTable.innerHTML = `
          <div class="empty-state">
            <p class="empty-state-title">This playlist is empty</p>
            <p class="empty-state-text">Click "Add Tracks" above to add songs to this playlist.</p>
          </div>
        `;
        return;
      }

      pl.tracks.forEach((t, idx) => {
        elements.playlistTrackTable.appendChild(createTrackRow(t, idx + 1, true, playlistId));
      });
    } catch (err) {
      console.error('Failed to load playlist detail:', err);
    }
  }

  // --- Discover Genre & Sorting ---
  async function loadDiscoverTracks() {
    try {
      let url = API.tracks;
      if (state.discoverGenre !== 'all') {
        url += `?genre=${encodeURIComponent(state.discoverGenre)}`;
      } else if (state.discoverSort) {
        url += `?sort=${encodeURIComponent(state.discoverSort)}&asc=true`;
      }

      const tracks = await apiFetch(url);
      elements.discoverCount.textContent = tracks.length;
      elements.discoverTrackTable.innerHTML = '';

      if (tracks.length === 0) {
        elements.discoverTrackTable.innerHTML = `
          <div class="empty-state">
            <p class="empty-state-title">No tracks found for genre "${escapeHtml(state.discoverGenre)}"</p>
          </div>
        `;
        return;
      }

      tracks.forEach((t, idx) => {
        elements.discoverTrackTable.appendChild(createTrackRow(t, idx + 1));
      });
    } catch (err) {
      console.error('Failed to load discover tracks:', err);
    }
  }

  // --- Search Execution ---
  let searchTimeout = null;
  async function performSearch() {
    const query = elements.mainSearchInput.value;
    state.searchQuery = query;

    if (!query || !query.trim()) {
      elements.searchResultsContainer.innerHTML = `
        <div class="empty-state">
          <p class="empty-state-title">Type a search query above</p>
          <p class="empty-state-text">Search for your favorite tracks, artists, albums, or genres.</p>
        </div>
      `;
      elements.searchResultsTitle.textContent = 'Search Results';
      return;
    }

    try {
      const url = `${API.search}?q=${encodeURIComponent(query)}&field=${encodeURIComponent(state.searchField)}`;
      const results = await apiFetch(url);

      elements.searchResultsTitle.textContent = `Search Results for "${escapeHtml(query)}" (${results.length})`;
      elements.searchResultsContainer.innerHTML = '';

      if (results.length === 0) {
        elements.searchResultsContainer.innerHTML = `
          <div class="empty-state">
            <p class="empty-state-title">No results found</p>
            <p class="empty-state-text">No tracks matched "${escapeHtml(query)}". Try another search keyword or clear filter.</p>
          </div>
        `;
        return;
      }

      const table = document.createElement('div');
      table.className = 'track-table';
      results.forEach((t, idx) => {
        table.appendChild(createTrackRow(t, idx + 1));
      });
      elements.searchResultsContainer.appendChild(table);
    } catch (err) {
      console.error('Search failed:', err);
    }
  }

  // --- Core Player Actions ---
  async function playTrack(trackId = '') {
    try {
      const resp = await apiFetch(`${API.player}/play`, {
        method: 'POST',
        body: JSON.stringify({ track_id: trackId })
      });
      state.player = resp;
      renderPlayerBar();
      renderQueue();
      showToast(`Now playing: ${state.player.current_track ? state.player.current_track.title : 'Track'}`);
    } catch (err) {
      console.error('Play track failed:', err);
    }
  }

  async function togglePlayPause() {
    if (state.player.is_playing) {
      await apiFetch(`${API.player}/pause`, { method: 'POST' });
      state.player.is_playing = false;
    } else {
      await apiFetch(`${API.player}/play`, { method: 'POST' });
      state.player.is_playing = true;
    }
    renderPlayerBar();
  }

  async function nextTrack() {
    try {
      const resp = await apiFetch(`${API.player}/next`, { method: 'POST' });
      state.player = resp;
      // Refresh queue state
      const q = await apiFetch(API.queue);
      state.queue = q.tracks || [];
      renderPlayerBar();
      renderQueue();
    } catch (err) {
      console.error('Next track failed:', err);
    }
  }

  async function prevTrack() {
    try {
      const resp = await apiFetch(`${API.player}/previous`, { method: 'POST' });
      state.player = resp;
      renderPlayerBar();
      renderQueue();
    } catch (err) {
      console.error('Previous track failed:', err);
    }
  }

  async function seekTo(seconds) {
    try {
      await apiFetch(`${API.player}/seek`, {
        method: 'POST',
        body: JSON.stringify({ position_sec: Math.floor(seconds) })
      });
      state.player.position_sec = seconds;
      elements.currentTime.textContent = formatTime(seconds);
      if (elements.audioPlayer.duration) {
        elements.audioPlayer.currentTime = seconds;
      }
    } catch (err) {
      console.error('Seek failed:', err);
    }
  }

  async function setVolume(vol) {
    try {
      await apiFetch(`${API.player}/volume`, {
        method: 'POST',
        body: JSON.stringify({ volume: vol })
      });
      state.player.volume = vol;
      elements.audioPlayer.volume = vol / 100;
    } catch (err) {
      console.error('Set volume failed:', err);
    }
  }

  async function toggleShuffle() {
    try {
      const resp = await apiFetch(`${API.player}/shuffle`, { method: 'POST' });
      state.player.shuffle = resp.shuffle;
      renderPlayerBar();
      showToast(`Shuffle ${state.player.shuffle ? 'enabled' : 'disabled'}`);
    } catch (err) {
      console.error('Toggle shuffle failed:', err);
    }
  }

  async function toggleRepeat() {
    const modes = ['off', 'all', 'one'];
    const nextIdx = (modes.indexOf(state.player.repeat_mode) + 1) % modes.length;
    const newMode = modes[nextIdx];

    try {
      const resp = await apiFetch(`${API.player}/repeat`, {
        method: 'POST',
        body: JSON.stringify({ mode: newMode })
      });
      state.player.repeat_mode = resp.repeat_mode;
      renderPlayerBar();
      showToast(`Repeat mode: ${newMode}`);
    } catch (err) {
      console.error('Toggle repeat failed:', err);
    }
  }

  // --- Queue Actions ---
  async function enqueueTrack(trackId) {
    try {
      const resp = await apiFetch(API.queue, {
        method: 'POST',
        body: JSON.stringify({ track_id: trackId })
      });
      state.queue = resp.queue || [];
      renderQueue();
      elements.playerQueueBadge.textContent = state.queue.length;
      elements.playerQueueBadge.style.display = state.queue.length > 0 ? 'flex' : 'none';
      showToast('Track added to FIFO queue');
    } catch (err) {
      console.error('Enqueue failed:', err);
    }
  }

  async function removeFromQueue(trackId) {
    try {
      const resp = await apiFetch(`${API.queue}/${trackId}`, { method: 'DELETE' });
      state.queue = resp.queue || [];
      renderQueue();
      elements.playerQueueBadge.textContent = state.queue.length;
      elements.playerQueueBadge.style.display = state.queue.length > 0 ? 'flex' : 'none';
      showToast('Track removed from queue');
    } catch (err) {
      console.error('Remove from queue failed:', err);
    }
  }

  async function clearQueue() {
    try {
      await apiFetch(`${API.queue}/clear`, { method: 'POST' });
      state.queue = [];
      renderQueue();
      elements.playerQueueBadge.textContent = 0;
      elements.playerQueueBadge.style.display = 'none';
      showToast('Queue cleared');
    } catch (err) {
      console.error('Clear queue failed:', err);
    }
  }

  // --- Liked Songs Actions ---
  async function toggleLikeTrack(trackId) {
    try {
      const resp = await apiFetch(`${API.tracks}/${trackId}/like`, { method: 'POST' });
      const likedList = await apiFetch(API.liked);
      state.likedTracks = likedList;
      renderAll();
      showToast(resp.liked ? 'Saved to Liked Songs' : 'Removed from Liked Songs');
    } catch (err) {
      console.error('Toggle like failed:', err);
    }
  }

  // --- Playlist Actions ---
  async function playPlaylist(playlistId) {
    try {
      const pl = await apiFetch(`${API.playlists}/${playlistId}`);
      if (!pl.tracks || pl.tracks.length === 0) {
        showToast('Playlist is empty', 'error');
        return;
      }
      // Play first track, enqueue remainder to FIFO queue
      await clearQueue();
      await playTrack(pl.tracks[0].id);
      for (let i = 1; i < pl.tracks.length; ++i) {
        await enqueueTrack(pl.tracks[i].id);
      }
      showToast(`Playing playlist "${pl.title}"`);
    } catch (err) {
      console.error('Play playlist failed:', err);
    }
  }

  async function reorderPlaylistTracks(playlistId, fromIdx, toIdx) {
    try {
      const resp = await apiFetch(`${API.playlists}/${playlistId}/reorder`, {
        method: 'POST',
        body: JSON.stringify({ from_index: fromIdx, to_index: toIdx })
      });
      await loadPlaylistDetail(playlistId);
      showToast('Playlist track order updated');
    } catch (err) {
      console.error('Reorder tracks failed:', err);
    }
  }

  async function removeTrackFromPlaylist(playlistId, trackId) {
    try {
      await apiFetch(`${API.playlists}/${playlistId}/tracks/${trackId}`, { method: 'DELETE' });
      await loadPlaylistDetail(playlistId);
      const pls = await apiFetch(API.playlists);
      state.playlists = pls;
      renderSidebarPlaylists();
      showToast('Track removed from playlist');
    } catch (err) {
      console.error('Remove track from playlist failed:', err);
    }
  }

  async function deletePlaylist(playlistId) {
    if (!confirm('Are you sure you want to delete this playlist?')) return;
    try {
      await apiFetch(`${API.playlists}/${playlistId}`, { method: 'DELETE' });
      state.playlists = await apiFetch(API.playlists);
      renderSidebarPlaylists();
      navigateTo('home');
      showToast('Playlist deleted');
    } catch (err) {
      console.error('Delete playlist failed:', err);
    }
  }

  // --- Audio Player Event Listeners & Progress Loop ---
  elements.audioPlayer.addEventListener('timeupdate', () => {
    if (state.isScrubbing) return;
    const cur = elements.audioPlayer.currentTime;
    const dur = elements.audioPlayer.duration || (state.player.current_track ? state.player.current_track.duration : 1);
    elements.currentTime.textContent = formatTime(cur);
    const pct = Math.min(100, (cur / dur) * 100);
    elements.progressBarFill.style.width = `${pct}%`;
    elements.progressScrubber.value = pct;
  });

  elements.audioPlayer.addEventListener('ended', async () => {
    // When track ends, advance to next track automatically
    await nextTrack();
  });

  // Simulated fallback progress timer if audio does not trigger events
  setInterval(() => {
    if (state.player.is_playing && (!elements.audioPlayer.src || elements.audioPlayer.paused)) {
      if (state.isScrubbing) return;
      state.player.position_sec += 1;
      const dur = state.player.current_track ? state.player.current_track.duration : 200;
      if (state.player.position_sec >= dur) {
        nextTrack();
      } else {
        elements.currentTime.textContent = formatTime(state.player.position_sec);
        const pct = Math.min(100, (state.player.position_sec / dur) * 100);
        elements.progressBarFill.style.width = `${pct}%`;
        elements.progressScrubber.value = pct;
      }
    }
  }, 1000);

  // --- Setup UI Event Handlers ---
  function setupEventHandlers() {
    // Nav items
    elements.navItems.forEach(item => {
      item.addEventListener('click', () => {
        const view = item.dataset.view;
        if (view) navigateTo(view);
      });
    });

    // Player Controls
    elements.btnPlayPause.addEventListener('click', togglePlayPause);
    elements.btnNext.addEventListener('click', nextTrack);
    elements.btnPrev.addEventListener('click', prevTrack);
    elements.btnShuffle.addEventListener('click', toggleShuffle);
    elements.btnRepeat.addEventListener('click', toggleRepeat);

    elements.btnPlayerLike.addEventListener('click', () => {
      const trackId = elements.btnPlayerLike.dataset.trackId;
      if (trackId) toggleLikeTrack(trackId);
    });

    // Progress Scrubber
    elements.progressScrubber.addEventListener('input', () => {
      state.isScrubbing = true;
      const pct = parseFloat(elements.progressScrubber.value);
      elements.progressBarFill.style.width = `${pct}%`;
      const dur = elements.audioPlayer.duration || (state.player.current_track ? state.player.current_track.duration : 0);
      elements.currentTime.textContent = formatTime((pct / 100) * dur);
    });

    elements.progressScrubber.addEventListener('change', async () => {
      state.isScrubbing = false;
      const pct = parseFloat(elements.progressScrubber.value);
      const dur = elements.audioPlayer.duration || (state.player.current_track ? state.player.current_track.duration : 0);
      const targetSec = (pct / 100) * dur;
      await seekTo(targetSec);
    });

    // Volume Slider
    elements.volumeSlider.addEventListener('input', (e) => {
      const val = parseInt(e.target.value, 10);
      setVolume(val);
      if (val === 0) {
        elements.iconVolHigh.style.display = 'none';
        elements.iconVolMute.style.display = 'block';
      } else {
        elements.iconVolHigh.style.display = 'block';
        elements.iconVolMute.style.display = 'none';
      }
    });

    elements.btnVolumeIcon.addEventListener('click', () => {
      if (state.player.volume > 0) {
        setVolume(0);
        elements.volumeSlider.value = 0;
        elements.iconVolHigh.style.display = 'none';
        elements.iconVolMute.style.display = 'block';
      } else {
        setVolume(80);
        elements.volumeSlider.value = 80;
        elements.iconVolHigh.style.display = 'block';
        elements.iconVolMute.style.display = 'none';
      }
    });

    // Queue Toggle
    elements.btnQueueToggle.addEventListener('click', () => {
      elements.queueDrawer.classList.toggle('open');
    });

    elements.btnCloseQueue.addEventListener('click', () => {
      elements.queueDrawer.classList.remove('open');
    });

    elements.btnClearQueue.addEventListener('click', clearQueue);

    // Discover Genre Chips
    elements.discoverGenreChips.querySelectorAll('.genre-chip').forEach(chip => {
      chip.addEventListener('click', () => {
        elements.discoverGenreChips.querySelectorAll('.genre-chip').forEach(c => c.classList.remove('active'));
        chip.classList.add('active');
        state.discoverGenre = chip.dataset.genre;
        loadDiscoverTracks();
      });
    });

    // Discover Sort
    elements.discoverSortSelect.addEventListener('change', (e) => {
      state.discoverSort = e.target.value;
      loadDiscoverTracks();
    });

    // Search Input
    elements.mainSearchInput.addEventListener('input', () => {
      const val = elements.mainSearchInput.value;
      elements.btnClearSearchLarge.style.display = val ? 'block' : 'none';
      clearTimeout(searchTimeout);
      searchTimeout = setTimeout(performSearch, 200);
    });

    elements.btnClearSearchLarge.addEventListener('click', () => {
      elements.mainSearchInput.value = '';
      elements.btnClearSearchLarge.style.display = 'none';
      performSearch();
    });

    // Search Filter Pills
    elements.searchFilterPills.forEach(pill => {
      pill.addEventListener('click', () => {
        elements.searchFilterPills.forEach(p => p.classList.remove('active'));
        pill.classList.add('active');
        state.searchField = pill.dataset.field;
        performSearch();
      });
    });

    // Library Tabs
    elements.libTabs.forEach(tab => {
      tab.addEventListener('click', () => {
        elements.libTabs.forEach(t => t.classList.remove('active'));
        tab.classList.add('active');
        const targetTab = tab.dataset.libTab;
        document.querySelectorAll('.lib-tab-pane').forEach(p => p.classList.remove('active'));
        const pane = document.getElementById(`lib-tab-content-${targetTab}`);
        if (pane) pane.classList.add('active');
      });
    });

    // Liked Songs Play All
    elements.btnPlayAllLiked.addEventListener('click', async () => {
      if (state.likedTracks.length === 0) return;
      await clearQueue();
      await playTrack(state.likedTracks[0].id);
      for (let i = 1; i < state.likedTracks.length; ++i) {
        await enqueueTrack(state.likedTracks[i].id);
      }
      showToast('Playing all Liked Songs');
    });

    // Playlist Detail Actions
    elements.btnPlayPlaylist.addEventListener('click', () => {
      if (state.selectedPlaylistId) playPlaylist(state.selectedPlaylistId);
    });

    elements.btnDeletePlaylist.addEventListener('click', () => {
      if (state.selectedPlaylistId) deletePlaylist(state.selectedPlaylistId);
    });

    // Create / Edit Playlist Modal
    let editingPlaylistId = null;
    let selectedGradient = 'linear-gradient(135deg, #8b5cf6, #d946ef)';

    elements.modalGradientPicker.querySelectorAll('.gradient-option').forEach(opt => {
      opt.addEventListener('click', () => {
        elements.modalGradientPicker.querySelectorAll('.gradient-option').forEach(o => o.classList.remove('active'));
        opt.classList.add('active');
        selectedGradient = opt.dataset.gradient;
      });
    });

    function openPlaylistModal(isEdit = false, pl = null) {
      editingPlaylistId = isEdit && pl ? pl.id : null;
      elements.playlistModalTitle.textContent = isEdit ? 'Edit Playlist' : 'Create Playlist';
      elements.modalPlaylistName.value = isEdit && pl ? pl.title : '';
      elements.modalPlaylistDesc.value = isEdit && pl ? (pl.description || '') : '';
      elements.playlistModal.style.display = 'flex';
      elements.modalPlaylistName.focus();
    }

    elements.btnCreatePlaylist.addEventListener('click', () => openPlaylistModal(false));
    elements.topBtnNewPlaylist.addEventListener('click', () => openPlaylistModal(false));
    elements.btnEditPlaylistModal.addEventListener('click', () => {
      const pl = state.playlists.find(p => p.id === state.selectedPlaylistId);
      if (pl) openPlaylistModal(true, pl);
    });

    function closePlaylistModal() {
      elements.playlistModal.style.display = 'none';
      editingPlaylistId = null;
    }

    elements.btnClosePlaylistModal.addEventListener('click', closePlaylistModal);
    elements.btnCancelPlaylistModal.addEventListener('click', closePlaylistModal);

    elements.btnSavePlaylistModal.addEventListener('click', async () => {
      const title = elements.modalPlaylistName.value.trim() || 'My Playlist';
      const desc = elements.modalPlaylistDesc.value.trim();

      if (editingPlaylistId) {
        await apiFetch(`${API.playlists}/${editingPlaylistId}`, {
          method: 'PUT',
          body: JSON.stringify({ title, description: desc })
        });
        showToast('Playlist updated');
        await loadPlaylistDetail(editingPlaylistId);
      } else {
        const created = await apiFetch(API.playlists, {
          method: 'POST',
          body: JSON.stringify({ title, description: desc, cover_gradient: selectedGradient })
        });
        showToast('Playlist created');
        navigateTo('playlist-detail', created.id);
      }

      closePlaylistModal();
      state.playlists = await apiFetch(API.playlists);
      renderSidebarPlaylists();
    });

    // Add Tracks to Playlist Modal
    elements.btnAddTracksModal.addEventListener('click', () => {
      const pl = state.playlists.find(p => p.id === state.selectedPlaylistId);
      if (!pl) return;

      elements.modalTracksToAdd.innerHTML = '';
      state.allTracks.forEach(t => {
        const alreadyInPl = pl.track_ids && pl.track_ids.includes(t.id);
        const item = document.createElement('div');
        item.className = 'track-select-item';
        item.innerHTML = `
          <div style="display: flex; align-items: center; gap: 8px;">
            <div class="track-art-thumb" style="background: ${t.artwork}; width: 28px; height: 28px;"></div>
            <span>${escapeHtml(t.title)} - ${escapeHtml(t.artist)}</span>
          </div>
          <button class="btn-secondary" style="padding: 4px 10px; font-size: 12px;" ${alreadyInPl ? 'disabled' : ''}>
            ${alreadyInPl ? 'Added' : '+ Add'}
          </button>
        `;

        const addBtn = item.querySelector('button');
        if (!alreadyInPl) {
          addBtn.addEventListener('click', async () => {
            await apiFetch(`${API.playlists}/${pl.id}/tracks`, {
              method: 'POST',
              body: JSON.stringify({ track_id: t.id })
            });
            addBtn.disabled = true;
            addBtn.textContent = 'Added';
            await loadPlaylistDetail(pl.id);
            state.playlists = await apiFetch(API.playlists);
            renderSidebarPlaylists();
            showToast(`Added "${t.title}" to playlist`);
          });
        }

        elements.modalTracksToAdd.appendChild(item);
      });

      elements.addToPlaylistModal.style.display = 'flex';
    });

    function closeAddModal() {
      elements.addToPlaylistModal.style.display = 'none';
    }
    elements.btnCloseAddModal.addEventListener('click', closeAddModal);
    elements.btnCancelAddModal.addEventListener('click', closeAddModal);
  }

  function escapeHtml(str) {
    if (!str) return '';
    return str
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }

  // --- Initialize Application ---
  window.addEventListener('DOMContentLoaded', async () => {
    setupEventHandlers();
    await fetchAppState();
  });

})();
