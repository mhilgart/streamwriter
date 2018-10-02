# StreamWriter

This program writes CBF images from a Dectris Eiger detector.  It can be used in parallel with the Dectris HDF FileWriter or instead of it.  It installs in minutes via an RPM, or can be built from source.

StreamWriter has been used at the 23-ID-B beamline at the Advanced Photon Source (APS) since June 2017 for all Eiger image writing.

**Jump to**:

- [RPM Install](#rpm-install)
- [Quick Test](#quick-test)
- [HTTP API](#http-api)
- [Source Install](#source-install)

# RPM Install

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

# Quick Test

After starting the service either from RPM or source:

1. **Verify the Eiger ZeroMQ stream is enabled**
   - To check: ``curl -s -H "Accept: application/json" http://[eiger_ip]/stream/api/1.6.0/config/mode | cut -d \, -f3``
   - To enable: ``curl -X PUT -H "Content-Type: application/json" -d "{\"value\":\"enabled\"}" http://[eiger_ip]/monitor/api/1.6.0/config/mode``
2. **Take some images using your existing control system**

# HTTP API

Filename information can be set and a counter can be read via HTTP.  If you don't set any configuration, StreamWriter will write files to its current working directory with the prefix streamwriter_test.  A new configuration can be set before each series with an HTTP POST.  When the HTTP returns 200, it's OK to proceed with the series.

```
To configure path information:

curl -X POST \
  -d dir=/path/to/images \
  -d prefix=test \
  -d username=user \
  -d first_frame_label=1 \
  -d counter_start_value=1 \
  http://[ip:port]/set_config
```

To read the counter and path information: ``curl http://[host:ip]/counter``

```
Sample counter output:

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

# Source install

## Requirements

These instructions are tested only on CentOS 7.5 but will hopefully work on any modern Linux OS.  Please send me feedback on how compilation works for you.  The required system libraries are listed below.  The package names are for CentOS 7 and may be different for different operating systems.

- git
- gcc-c++
- gcc-gfortran
- libtool
- pcre-devel
- zlib-devel
- libunwind-devel
- libsodium-devel
- cmake3

To install these on CentOS 7, first add the EPEL repository if needed following step 1 in the [RPM Install](#rpm-install) section.  Then use "rpm -q" to check if each package is installed.  If it isn't, use "yum install [package]" to install it.

## Compile

1. **Clone the repository**
   - ``git clone --recursive -b v2018.3-1 https://github.com/mhilgart/streamwriter``
   - This checks out all git dependencies (sub-modules) and may take a few minutes
2. **Disable system library usage in CMakeLists.txt**
   - Change line 3 to ``set(USE_SYSTEM_LIBRARIES FALSE)``
   - This is needed for building RPMs but requires extra system libraries
3. **Make**
   - ``cd streamwriter && make init && make``
4. **Set IP addresses and ports**
   - ``sudo cp rpm/SOURCES/streamwriter-2018.3/streamwriter.conf /etc && sudo vi /etc/streamwriter.conf``
5. **Run**
   - ``sudo build/streamwriter``
   - Running as root is needed to set file permissions on the images.  StreamWriter should run fine as a regular user as long as the username isn't set from the HTTP API.

Then follow steps in the [Quick Test](#quick-test) section.

## If compilation fails

If a dependency fails to compile, you can either modify the build command in CMakeLists.txt, or go to the directory in modules/ and compile it manually.  Then you can comment out that dependency's section in CMakeLists.txt and recompile StreamWriter with

``cd [streamwriter dir] && make clean && make init && make``
