#!/bin/sh
# Smoke tests for ToyOS-LabC.
# QEMU is started in background, then killed after 8 seconds.

set -u

LOG="build/qemu.log"

mkdir -p build

echo "[test] rebuilding ToyOS..."
make clean
make

echo "[test] running QEMU for 8 seconds..."
rm -f "$LOG"

qemu-system-riscv64 \
  -machine virt \
  -bios default \
  -m 128M \
  -smp 1 \
  -nographic \
  -kernel build/kernel.bin \
  > "$LOG" 2>&1 &

QEMU_PID=$!

sleep 8

if kill -0 "$QEMU_PID" 2>/dev/null; then
    kill "$QEMU_PID" 2>/dev/null || true
    sleep 1
    kill -9 "$QEMU_PID" 2>/dev/null || true
fi

echo "[test] checking log..."

check() {
    if grep -Fq "$1" "$LOG"; then
        echo "[ok] $1"
    else
        echo "[fail] missing: $1"
        echo "--- qemu log ---"
        sed -n '1,180p' "$LOG"
        exit 1
    fi
}

check "[ToyOS] kernel booting"
check "[ToyOS] console init ok"
check "[ToyOS] trap init ok"
check "[ToyOS] syscall table init ok"
check "[ToyOS] physical page allocator init ok"
check "[ToyOS] kernel page table init ok"
check "[ToyOS] process table init ok"
check "[ToyOS] ToyFS init ok, files=1"
check "[ToyOS] enter user mode"
check "user: hello from user program"
check "user: syscall write works"
check "user: getpid = 1"
check "user: fork test start"
check "parent: child pid = 2"
check "child: hello"
check "parent: wait child done"
check "cat /hello.txt: Hello from ToyFS!"
check "[ToyOS] all basic tests finished"
check "[ToyOS] demo completed, kernel halted."

echo "All smoke tests passed. Log: $LOG"
