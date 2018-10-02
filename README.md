# StreamWriter

This program writes CBF images from a Dectris Eiger detector.  It can be used in parallel with the Dectris HDF FileWriter or instead of it.  It installs in minutes via an RPM, or can be built from source.

StreamWriter has been used at the 23-ID-B beamline at the Advanced Photon Source (APS) since June 2017 for all Eiger image writing.

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

To install these on CentOS 7, first add the EPEL repository if needed following step 1 in the RPM section above.  Then use "rpm -q" to check if each package is installed.  If it isn't, use "yum install [package]" to install it.

## Compile

The StreamWriter repository contains git submodules of its dependencies that are not available in CentOS 7.5.  The following command checks out StreamWriter and all of the submodules:

``git clone --recursive -b v2018.3-1 https://github.com/mhilgart/streamwriter``

Edit ``CMakeLists.txt`` and change line 3 to ``set(USE_SYSTEM_LIBRARIES FALSE)``.  This is needed for building the RPM, but requires extra system libraries.  Now compile:

``cd streamwriter && make init && make``

The git clone step will take at least a few minutes and consists of many separate downloads.  This will take some time especially for Boost.  The compile step will compile each dependency and can take several minutes.

Copy the configuration and edit it:

``cp rpm/SOURCES/streamwriter-2018.3/streamwriter.conf /etc && vi /etc/streamwriter.conf``

Now you can run the program:

``build/streamwriter``

And take images following the instructions in the "Quick Test" section.

## If compilation fails

If a dependency fails to compile, you can either modify the build command in CMakeLists.txt, or go to the directory in modules/ and compile it manually.  Then you can comment out that dependency's section in CMakeLists.txt and recompile StreamWriter with

``cd [streamwriter dir] && make clean && make init && make``
