#!/usr/bin/env sh
# Smoke test: build, run QEMU, then grep the serial log for key milestones.
set -eu

mkdir -p build
make clean
make

LOG=build/qemu.log
if command -v timeout >/dev/null 2>&1; then
  timeout 20s make run >"$LOG" 2>&1 || true
else
  make run >"$LOG" 2>&1 || true
fi

check() {
  if grep -F "$1" "$LOG" >/dev/null 2>&1; then
    printf '[ok] %s\n' "$1"
  else
    printf '[fail] missing: %s\n' "$1"
    printf '--- qemu log ---\n'
    tail -n 80 "$LOG" || true
    exit 1
  fi
}

check '[ToyOS] kernel booting...'
check '[ToyOS] console init ok'
check '[ToyOS] trap init ok'
check '[ToyOS] physical page allocator init ok'
check '[ToyOS] kernel page table init ok'
check '[ToyOS] process table init ok'
check '[ToyOS] enter user mode'
check 'user: hello from user program'
check 'user: syscall write works'
check 'user: fork test start'
check 'parent: child pid ='
check 'child: hello'
check 'user: file system test'
check 'cat /hello.txt: Hello from ToyFS!'
check '[ToyOS] all basic tests finished'

printf 'All smoke tests passed. Log: %s\n' "$LOG"
