all:
	cp -pf ../build/streamwriter SOURCES/streamwriter-2018.3/streamwriter
	chrpath -d SOURCES/streamwriter-2018.3/streamwriter
	tar -C SOURCES -czvf SOURCES/streamwriter-2018.3.tar.gz streamwriter-2018.3
	rpmbuild --define "_topdir `pwd`" -bb SPECS/streamwriter.spec 
