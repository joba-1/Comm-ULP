# ULP <=> Main Core Communication

Me trying to make an esp32 ULP talk to the main cores.

## Build - Flash - Monitor
* Prepare shell to use ESP-IDF (see my [Blink-ULP repo](https://github.com/joba-1/Blink-ULP/blob/master/README.md) for details)
```
. ~/esp32/env.sh
```

* Fetch and build this example, flash it and open serial console (adapt ESPPORT and ESPBAUD to your setup. Can be configured in sdkconfig file)
```
REPO="Comm-ULP"
BASE="/tmp/$REPO-$$"
mkdir -p "$BASE" && \
cd "$BASE" && \
git clone "https://github.com/joba-1/$REPO.git" && \
cd "$REPO" && \
make -j8 flash monitor ESPPORT=/dev/ttyUSB0 ESPBAUD=115200

```

The ULP does some counting and sends results as hex lines to the main cores where they are printed to serial.
The main cores sleep until the ULP wakes them up because the buffer is full.
The ULP sleeps as well between counts -> very low power consumption.

Exit the monitor with [Ctrl]-] (which is [Strg]-[Alt-Gr]-] on german keyboard)

JoBa1
