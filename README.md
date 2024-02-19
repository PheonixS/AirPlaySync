# AirplaySync - Repurposing of Harman-Kardon DVD(27) player 

Here is a full source code I'm using to control DVD27 from the Raspberry PI.
Please check https://pheonixs.nl for details over design and walkthrough over details of this project.

Service file to run software as a daemon:
`/etc/systemd/system/AirplaySync.service`

```
[Unit]
Description=Airplay sync service
After=network.target
StartLimitIntervalSec=0
Requires=docker.service

[Service]
Type=simple
Restart=always
RestartSec=5
ExecStart=/usr/bin/AirplaySync

[Install]
WantedBy=multi-user.target
```

Changes in `/boot/firmware/config.txt`:
```
dtparam=i2c_arm=on
dtparam=spi=on
```

* I2C used by ADC to read button/resistor array.
* SPI is used to control Vacuum Fluorescent Display (VFD).

## Board layout and production files
* [PCB](./pcb/pcb.pdf)
* [Gerber](./pcb/Gerber/)
