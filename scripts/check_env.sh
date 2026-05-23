#!/usr/bin/env sh
# Check whether the tools needed to build and run ToyOS are available.
set -u

ok=1

need() {
  if command -v "$1" >/dev/null 2>&1; then
    printf '[ok] %s -> %s\n' "$1" "$(command -v "$1")"
  else
    printf '[missing] %s\n' "$1"
    ok=0
  fi
}

if command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then
  need riscv64-unknown-elf-gcc
  need riscv64-unknown-elf-objcopy
elif command -v riscv64-linux-gnu-gcc >/dev/null 2>&1; then
  need riscv64-linux-gnu-gcc
  need riscv64-linux-gnu-objcopy
else
  printf '[missing] riscv64-unknown-elf-gcc or riscv64-linux-gnu-gcc\n'
  ok=0
fi

need make
need qemu-system-riscv64

if [ "$ok" -eq 1 ]; then
  printf 'Environment looks ready.\n'
  exit 0
fi

cat <<'MSG'

Ubuntu/WSL2 install example:
  sudo apt update
  sudo apt install -y build-essential gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-misc

Then run:
  make clean
  make
  make run
MSG
exit 1
