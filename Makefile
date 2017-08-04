version=0.3.5
BRANCH?=master

all:
	cd ./threadpool && $(MAKE) all
	cd ./src && $(MAKE) all

clean:
	cd ./threadpool && $(MAKE) clean
	cd ./src && $(MAKE) clean
	rm -rf output

prepare:
	cd ./threadpool && $(MAKE) prepare
	cd ./uthash && $(MAKE) prepare

install:
	cd ./src && sudo $(MAKE) install
	cd ./include && sudo $(MAKE) install

rpm:
	mkdir -p ~/rpmbuild/{RPMS,SRPMS,BUILD,SOURCES,SPECS,tmp}
	cp -f ./libprovenance.spec ~/rpmbuild/SPECS/libprovenance.spec
	rpmbuild -bb libprovenance.spec
	mkdir -p output
	cp ~/rpmbuild/RPMS/x86_64/* ./output

travis_checkout_dev:
	git clone https://github.com/CamFlow/camflow-dev.git
	cd camflow-dev && git checkout $(BRANCH)

travis_update_files: travis_checkout_dev
	sed -i -e 's/#include <linux\/provenance.h>/#include "..\/camflow-dev\/include\/uapi\/linux\/provenance.h"/g' ./include/provenance.h
	sed -i -e 's/#include <linux\/provenance.h>/#include "..\/camflow-dev\/include\/uapi\/linux\/provenance.h"/g' ./include/provenancefilter.h
	sed -i -e 's/#include <linux\/provenance.h>/#include "..\/camflow-dev\/include\/uapi\/linux\/provenance.h"/g' ./include/provenanceutils.h
	sed -i -e 's/#include <linux\/camflow.h>/#include "..\/camflow-dev\/include\/uapi\/linux\/camflow.h"/g' ./src/provenanceProvJSON.c
	sed -i -e 's/#include <linux\/xattr.h>/#include "..\/camflow-dev\/include\/uapi\/linux\/xattr.h"/g' ./src/libprovenance.c

travis: travis_update_files prepare all install

publish:
	cd ./output && package_cloud push camflow/provenance/fedora/26 libprovenance-$(version)-1.x86_64.rpm
