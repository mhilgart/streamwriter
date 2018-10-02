# StreamWriter

This program writes CBF images from a Dectris Eiger detector.  It can be 
installed in minutes via an RPM, or built from source with the dependencies
listed below.

StreamWriter has been used at the 23-ID-B beamline at the Advanced Photon
Source (APS) since July 2017 for all Eiger image writing.

# RPM install

The RPM is tested with CentOS 7.5.  To install:

1. **Add the EPEL repository if needed**
   - Check if EPEL is installed with ``rpm -q epel-release``
   - If it isn't: ``yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm``
2. **Install the StreamWriter RPM**
   - ``yum install https://github.com/mhilgart/streamwriter/releases/download/v2018.3-1/streamwriter-2018.3-1.x86_64.rpm``
3. **Edit the configuration**
   - Open ``/etc/streamwriter.conf``
   - Set the IP and port of your Eiger control unit (ECU)
   - Set the IP of your server and the preferred HTTP API port
4. **Start the StreamWriter service**
   - ``systemctl start streamwriter``
5. **Optional: Permanently enable the service**
   - ``systemctl enable streamwriter``

# HTTP API

Filename information can be set and a counter can be read via HTTP.  If you don't set any configuration, StreamWriter will write files to its current working directory with the prefix streamwriter_test.  A new configuration can be set before each series with an HTTP POST:

```
curl -X POST \
  -d dir=/path/to/images \
  -d prefix=test \
  -d username=user \
  -d first_frame_label=1 \
  -d counter_start_value=1 \
  http://[ip:port]/set_config
```

To read the counter and path information: ``curl http://[host:ip]/counter``

The output will look something like this:

```
{
   "counter" : {
      "completeCounter" : 0,
      "latestCounter" : 0,
      "streamInProgress" : false
   },
   "path" : {
      "dir" : ".",
      "firstFrameLabel" : 1,
      "firstFramePath" : "./streamwriter_test_000001.cbf",
      "prefix" : "streamwriter_test_",
      "username" : "",
      "zmqReferenceIndex" : 0
   }
}
```
