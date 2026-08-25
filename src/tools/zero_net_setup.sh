#!/bin/sh
# LinuxOSZero Network Setup Wizard

echo "=================================================="
echo "          LinuxOSZero Network Configuration       "
echo "=================================================="

# Check interfaces
echo "[*] Detected Network Interfaces:"
ip link show | grep -E "^[0-9]+:" | awk '{print "    " $2}'

# Auto configure DHCP on first available interface
for iface in eth0 enp0s3 enp0s8 eth1; do
    if [ -d "/sys/class/net/$iface" ]; then
        echo "[*] Requesting IP via DHCP on $iface..."
        ip link set "$iface" up
        udhcpc -i "$iface" -n -q -b
        echo "[OK] Network configuration active on $iface."
        ip addr show "$iface" | grep "inet "
        exit 0
    fi
done

echo "[WARN] No active ethernet adapter detected."
