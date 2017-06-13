# README

One Tsung plugin demo, for the special protocol `Qmsg`, base on Tsung 1.6.0.

## The Qmsg Protocol

One binary protocol designed for Tsung plugin demo, called **Qmsg**, which has simple structure, surrounded with specials words `**####**`.

![Tsung Plugin](https://raw.githubusercontent.com/weibomobile/tsung_plugin_demo/master/img/qmsg_protocol.png)

0. User Id, Integer, 4 byte
1. PacketLength, little-endian integer, 4 byte
2. Request data structure:
    - User Comment, Strings
    - `PocketLen:**##UserId + UserComment##**`
3. Server Response, returned with one random code
    - Random Code, integer, 4 byte

## How to Compile it

First, you should have the tsung-1.6.0 source folder :
```bash
wget http://tsung.erlang-projects.org/dist/tsung-1.6.0.tar.gz
tar xf tsung-1.6.0.tar.gz
```

Then compile the Tsung `Qmsg` plugin :
```bash
sh build_plugin.sh /your_path/tsung-1.6.0
```

## Start the Qmsg Server

```bash
erlc qmsg_server.erl && erl -s qmsg_server start
```

> The Qmsg Server listening port **5678**

## Start the Tsung

```bash
tsung -f tsung_qmsg.xml start
```

Its output maybe :

```bash
tarting Tsung
Log directory is: /root/.tsung/log/20160621-1334
[os_mon] memory supervisor port (memsup): Erlang has closed
[os_mon] cpu supervisor port (cpu_sup): Erlang has closed
```

## View HTML Report

```bash
cd /root/.tsung/log/20160621-1334

/usr/local/lib/tsung/bin/tsung_stats.pl

echo "open your browser (URL: http://IP:8000/report.html) and visit the report now :))"
/usr/bin/python -m SimpleHTTPServer
```

