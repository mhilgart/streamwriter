%define name          streamwriter 
%define version       2018.3
%define release       1
%define debug_package ${nil}
 
BuildRoot:      ${_topdir}
Summary:        CBF writer for the Eiger detector
Name:           %{name}
License:        JBluIce
Group:          Applications/System
Source:         %{name}-%{version}.tar.gz
Version:        %{version}
Release:        %{release}
 
%description
StreamWriter reads images from the Dectris Eiger Pixel Array Detector (PAD)
using the ZeroMQ PUSH-PULL protocol.  It writes those images to disk in the
Crystallographic Binary Format (CBF).
 
%prep
%setup
 
%build
 
%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system
mkdir -p $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1
install -m755 streamwriter $RPM_BUILD_ROOT/usr/bin/streamwriter
install -m755 streamwriter-start $RPM_BUILD_ROOT/usr/bin/streamwriter-start
install -m755 streamwriter-stop $RPM_BUILD_ROOT/usr/bin/streamwriter-stop
install -m644 streamwriter.service $RPM_BUILD_ROOT/usr/lib/systemd/system
install -m644 streamwriter.conf $RPM_BUILD_ROOT/etc/streamwriter.conf
install -m444 streamwriter.1 $RPM_BUILD_ROOT/usr/share/man/man1/streamwriter.1

%post
%systemd_post streamwriter.service

%preun
%systemd_preun streamwriter.service

%postun
%systemd_postun_with_restart streamwriter.service

%files
%defattr(-,root,root)
%config /etc/streamwriter.conf
%doc %attr(0444,root,root) /usr/share/man/man1/streamwriter.1.gz
/usr/bin/streamwriter
/usr/bin/streamwriter-start
/usr/bin/streamwriter-stop
%{_unitdir}/streamwriter.service
#%ghost /var/log/streamwriter/*.log
