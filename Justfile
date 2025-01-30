build:
  nix build

ins:
  sudo insmod result/lib/modules/$(uname -r)/extra/driver.ko

rm: 
  sudo rmmod result/lib/modules/$(uname -r)/extra/driver.ko

test: 
  echo
  sudo dmesg | tail
  echo

test-run: build
  echo
  echo "Running a test run..."
  echo "1. Pre test:"
  just test
  echo "2. After insmod:"
  just ins
  just test
  echo "3. After rmmod:"
  just rm
  just test
