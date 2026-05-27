#!/bin/bash

# Exit on error
set -e

APP_NAME="vconvert"
VERSION="0.1.0"
ARCH="amd64"
DEB_DIR="${APP_NAME}_${VERSION}_${ARCH}"

echo "🚀 Starting packaging process for $APP_NAME v$VERSION..."

# 1. Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf build/
rm -rf "$DEB_DIR"
rm -f "${DEB_DIR}.deb"

# 2. Build the project
echo "🔨 Compiling the project (Release mode)..."
meson setup build --buildtype=release
ninja -C build

# 3. Create directory structure
echo "📁 Creating Debian directory structure..."
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/scalable/apps"

# 4. Create control file
echo "📄 Generating control file..."
cat <<EOF > "$DEB_DIR/DEBIAN/control"
Package: $APP_NAME
Version: $VERSION
Section: video
Priority: optional
Architecture: $ARCH
Maintainer: Vaxp Developer <admin@vaxp.os>
Description: Professional Media Converter for Vaxp OS
 A clean, fast, and professional media converter built with C, GTK3, and FFmpeg.
 Supports Image, Video, and Audio conversion with real-time progress tracking.
Depends: libc6, libglib2.0-0, libgtk-3-0, ffmpeg, libcairo2, libgdk-pixbuf-2.0-0
EOF

# 5. Create desktop file
echo "🖥️ Generating desktop file..."
cat <<EOF > "$DEB_DIR/usr/share/applications/vconvert.desktop"
[Desktop Entry]
Name=VConvert
Comment=Professional Media Converter
Exec=vconvert
Icon=vconvert
Terminal=false
Type=Application
Categories=AudioVideo;Utility;
EOF

# 6. Copy files
echo "📦 Copying binaries and assets..."
cp build/$APP_NAME "$DEB_DIR/usr/bin/"
if [ -f "logo/vconvert.svg" ]; then
    cp logo/vconvert.svg "$DEB_DIR/usr/share/icons/hicolor/scalable/apps/"
else
    echo "⚠️ Warning: Logo logo/vconvert.svg not found! Skipping icon."
fi

# 7. Set permissions
chmod 0755 "$DEB_DIR/DEBIAN/control"
chmod 0755 "$DEB_DIR/usr/bin/$APP_NAME"
chmod 0644 "$DEB_DIR/usr/share/applications/vconvert.desktop"
if [ -f "$DEB_DIR/usr/share/icons/hicolor/scalable/apps/vconvert.svg" ]; then
    chmod 0644 "$DEB_DIR/usr/share/icons/hicolor/scalable/apps/vconvert.svg"
fi

# 8. Build deb
echo "🏗️ Building .deb package..."
dpkg-deb --build "$DEB_DIR"

echo "✅ Package created successfully: ${DEB_DIR}.deb"
