# README

One Tsung plugin demo, for the special protocol `Natty`, base on Tsung 1.6.0.

## How to Compile it

First, you should have the tsung-1.6.0 source folder :
```bash
wget http://tsung.erlang-projects.org/dist/tsung-1.6.0.tar.gz
tar xf tsung-1.6.0.tar.gz
```

Then compile the Tsung `Natty` plugin :
```bash
sh build_plugin.sh /your_path/tsung-1.6.0
```

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

