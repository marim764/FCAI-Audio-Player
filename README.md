# FCAI-Audio-Player
A sophisticated cross-platform audio application built with **C++** and the **JUCE Framework**. This project features a dual-deck architecture allowing simultaneous playback, real-time mixing, and visual waveform feedback.

## ✨ Core Features

### 1. Dual-Deck Playback
* **Independent Controls:** Each deck has its own Play, Pause, Stop, and Restart controls.
* **A-B Looping:** Set precise start and end points for seamless looping within a track.
* **Speed & Pitch:** Dynamic resampling for real-time playback speed adjustment.

### 2. Advanced Audio Engine
* **Real-time Mixing:** Centered mixer bar to balance gain between Deck A and Deck B.
* **Waveform Visualization:** Real-time thumbnail rendering for visual track navigation.
* **Metadata Extraction:** Displays track information using TagLib integration.
* **Multi-format Support:** Supports MP3, WAV, FLAC, and more.

### 3. User Experience
* **Drag & Drop:** Easily load tracks by dropping them directly onto the player.
* **Playlist Management:** Integrated ComboBox for quick switching between loaded files.
* **Interactive Seekbar:** Precise navigation through the audio buffer.

## 🚀 Getting Started
<img width="1718" height="876" alt="image" src="https://github.com/user-attachments/assets/85c04414-5555-477c-9347-d1c8721caa1c" />


### Prerequisites
* **JUCE Framework** (Projucer or CMake)
* A C++ compiler (MSVC, Clang, or GCC)
* **TagLib** library (for metadata support)

### Installation
1. Clone the repository.
2. Open the `.jucer` file in **Projucer**.
3. Save and open in your preferred IDE (Visual Studio / Xcode / CLion).
4. Build and run the project.

## 📂 Project Structure

* `PlayerAudio`: Handles the audio backend, resampling, and transport logic.
* `PlayerGUI`: Manages the visual components, waveform rendering, and user input.
* `MainComponent`: Orchestrates the two players and the central mixing logic.
