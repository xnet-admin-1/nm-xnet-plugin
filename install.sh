#!/bin/bash
set -e

echo "Installing ZeroTier NetworkManager VPN plugin..."

# Install plugin service
install -m 755 nm-zerotier-service /usr/libexec/nm-zerotier-service

# Install NM VPN name file
install -m 644 nm-zerotier-service.name /usr/lib/NetworkManager/VPN/

# Install D-Bus policy
install -m 644 nm-zerotier-service.conf /usr/share/dbus-1/system.d/

# Reload D-Bus and NM
systemctl reload dbus 2>/dev/null || true
systemctl reload NetworkManager 2>/dev/null || nmcli general reload 2>/dev/null || true

echo "Done. Import a .zt file with: nmcli connection import type zerotier file <name>.zt"
