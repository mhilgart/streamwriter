# StreamWriter

This program writes CBF images from a Dectris Eiger detector.  It can be 
installed in minutes via an RPM, or built from source with the dependencies
listed below.

# RPM install

The RPM is tested with CentOS 7.5.  To install:

1. Check if EPEL is installed with ``rpm -q epel-release``.  If it isn't: ``yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm``
2. ``yum install https://github.com/mhilgart/streamwriter/releases/download/v2018.3-1/streamwriter-2018.3-1.x86_64.rpm``
3. Edit /etc/streamwriter.conf with your Eiger IP/port and your preferred StreamWriter IP and HTTP listen port
4. ``systemctl start streamwriter``
5. Optional: ``systemctl enable streamwriter``
