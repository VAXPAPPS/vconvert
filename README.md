# 🔄 VConvert (محول الوسائط)

**VConvert** is a professional, high-performance, native media conversion application developed exclusively for **Vaxp OS**. It features a modern glassmorphism UI, real-time theming, and an extremely robust backend powered by `FFmpeg` and `Cairo`, strictly following **Clean Architecture** principles.

---

## ✨ Features

- **Blazing Fast Conversions:** Powered natively by C, GTK3, and `FFmpeg` utilizing `GSubprocess` for true asynchronous operations without UI freezing.
- **Modern UI & UX:** Features a beautiful Glassmorphism design with a dynamic empty state (Placeholder) that elegantly fades when files are dropped.
- **Drag & Drop:** Fully supports multiple file drag and drop for rapid queuing.
- **Smart Formatting:** The output format dropdown intelligently filters based on your input (e.g., dropping a video shows only video and audio extraction formats).
- **Audio Extraction:** Effortlessly extract audio from any video by simply selecting an audio output format (e.g., MP4 to MP3).
- **Live Progress Tracking:** Advanced `stderr` parsing provides real-time, accurate percentage progress bars for lengthy video and audio conversions.
- **Smart Routing:** Automatically saves files to their appropriate system folders based on the output type (`~/Pictures/VConvert`, `~/Videos/VConvert`, `~/Music/VConvert`).
- **Dynamic Theming:** Seamlessly integrates with Vaxp OS `theme_manager`, instantly reacting to system color changes in real-time (`~/.config/venom/settings.vaxp`).

---

## 📦 Supported Formats (20+)

### 🖼️ Images & Documents
Converts raster graphics natively via FFmpeg and vector/document graphics via GTK's native `Cairo` rendering:
`JPG`, `PNG`, `WEBP`, `BMP`, `TIFF`, `ICO`, `TGA`, `HEIC`, `SVG`, `PDF`, `EPS`, `AVIF`, `HDR`

### 🎥 Videos
`MP4`, `MKV`, `AVI`, `WEBM`, `MOV`, `FLV`, `GIF`

### 🎵 Audio
`MP3`, `WAV`, `FLAC`, `OGG`, `AAC`, `M4A`

---

## 🏗️ Architecture (Clean Architecture)
The project structure enforces strict separation of concerns for maintainability and future growth:
- **`domain/`**: Core logic and media types evaluation.
- **`usecases/`**: Central Conversion Manager routing requests to the right engine.
- **`infrastructure/`**: Specialized engines (`image_converter`, `video_converter`, `audio_converter`) interacting with the system (FFmpeg/Cairo).
- **`presentation/`**: GTK3 User Interface and dynamic `theme_manager`.

---

## 🚀 Building & Installation

VConvert uses the `Meson` build system and is bundled with a one-click automated `.deb` packager script.

```bash
# To build and package the application:
chmod +x package.sh
./package.sh

# To install it on Vaxp OS:
sudo dpkg -i vconvert_0.1.0_amd64.deb
```

---

## 🗺️ Roadmap (Future Features)

While **VConvert v1.0** is fully functional and production-ready, the following features are planned for future updates:

- **[ ] Safe Cancellation:** Add the ability to gracefully cancel an ongoing conversion and safely kill the underlying `FFmpeg` subprocess without leaving zombie processes.
- **[ ] System Notifications:** Integrate with `libnotify` to trigger a Vaxp OS desktop notification once a batch of conversions successfully finishes.
- **[ ] Advanced Output Settings:** Provide an expandable settings menu to customize video resolution (e.g., 1080p, 720p) and audio bitrate/quality before starting the conversion.
