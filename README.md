# TuneBox — Modern Music Player & Playlist Manager

TuneBox is a desktop-inspired music player and playlist manager that lets users browse a local music library, search for tracks, organize custom playlists, and manage playback order. Users can listen to sample tracks with full playback controls, maintain an upcoming song queue, and save their favorite songs to curated collections.

## 1. Application Overview

### Core Functionality

- Browse Music Library: Explore a diverse catalog of 16 sample tracks across multiple genres with rich artwork, track details, and audio playback.
- Real-Time Search: Find songs quickly by searching across titles, artists, albums, and genres with targeted field filters.
- Manage Playlists: Create custom playlists, edit titles and descriptions, add or remove tracks, and reorder songs to build the perfect mix.
- Audio Player Controls: Control playback with play, pause, previous, next, progress seeking, volume adjustment, shuffle, and repeat modes.
- Up Next Playback Queue: Add upcoming songs to a playback queue and inspect or modify the upcoming sequence in a slide-out drawer.
- Liked Songs & History: Save favorite songs to a dedicated Liked Songs collection and view recently played tracks from the current listening session.

## 2. Debugging Challenge

TuneBox contains four user-facing issues that disrupt catalog browsing, search accuracy, playlist organization, and queue playback. These defects cause missing songs, broken search results, incorrect playlist ordering, and skipped queue tracks.

### Reported Bugs to Fix:

BUG 1: Missing Boundary Track in Music Library

- User Symptom: The last song in the catalog does not appear when browsing all tracks in the library.
- Task: Ensure that every track in the music catalog appears in the library without the final track being skipped.

BUG 2: Search Fails with Padded Spaces or Uppercase Words

- User Symptom: Searching with extra spaces or uppercase letters returns zero results even when matching songs exist in the library.
- Task: Allow search queries to match songs regardless of letter casing or extra whitespace around search terms.

BUG 3: Playlist Tracks Move to Wrong Positions During Reordering

- User Symptom: Moving a track up or down inside a playlist places it into the wrong position rather than the selected target slot.
- Task: Correct playlist track reordering so that moved songs land in their exact intended position while preserving overall order.

BUG 4: Queue Skips the First Queued Song on Playback

- User Symptom: When advancing to the next song from the queue, the player skips the first queued song and plays the one after it.
- Task: Ensure queued songs play strictly in first-in, first-out order so that every queued song is played in sequence.

## 3. Expected Behavior After Fixing Bugs

1. The music library displays all 16 tracks, including the first and last catalog entries, with none silently omitted.
2. Search queries successfully locate matching songs regardless of uppercase letters or surrounding whitespace.
3. Reordering playlist songs places each moved track into its selected position while preserving all other track positions.
4. Advancing playback from the queue plays the first queued song immediately and maintains the correct sequence for remaining tracks.

Once all issues are fixed, TuneBox provides an uninterrupted, dependable music listening and playlist management experience across all views.
