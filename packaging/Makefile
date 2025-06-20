# Makefile
#
# Copyright (c) 2022-present Bartek Kryza <bkryza@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SHELL := /bin/bash
.ONESHELL:

.PHONY: download deb debian clean conda

NAME ?= clang-include-graph
REBUILD ?= 1
MAINTAINER_NAME ?= Bartek Kryza
MAINTAINER_EMAIL ?= bkryza@gmail.com
GPG_KEY ?= 702014E322FE5CA9B5D920F66CDA4566635E93B1
OS ?= ubuntu
DIST ?= focal
TAR_EXT ?= gz

build_dir = _BUILD/$(OS)/$(DIST)

VERSION	?= $(shell git describe --tags --always --abbrev=7)
COMMIT ?= $(shell git rev-parse HEAD)
BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)
SOURCE_ARCHIVE ?= $(NAME)-$(VERSION).tar.$(TAR_EXT)
SOURCE_ARCHIVE_DEB ?= $(NAME)-$(VERSION)-$(REBUILD).tar.$(TAR_EXT)
SOURCE_ARCHIVE_RPM ?= $(NAME)-$(VERSION).tar.$(TAR_EXT)
CONDA_TOKEN ?=
WITH_JSON ?= ON

#
# Replace mustache template variable in all files in directory recursively,
# e.g.:
#  $(call subst_template,VERSION,${VERSION},debian)
#
define subst_template_dir
        find $(3) -type f -exec sed -i "s/{{$(1)}}/$(2)/g" {} \;
endef

define subst_conda_meta_yaml
        find $(3) -name meta.yaml -exec sed -i "s/{{$(1)}}/$(2)/g" {} \;
endef



_BUILD/$(SOURCE_ARCHIVE):
		echo "############################"
		echo "Creating source archive from latest commit $(COMMIT) - $(SOURCE_ARCHIVE) - $(VERSION)"
		echo "############################"
		mkdir -p $(build_dir)
		git -C .. archive --format=tar.gz --prefix=$(NAME)-$(VERSION)/ $(VERSION) > _BUILD/$(SOURCE_ARCHIVE)

_BUILD/$(SOURCE_ARCHIVE_DEB):
		echo "############################"
		echo "Creating source archive for DEB from latest commit $(COMMIT) - $(SOURCE_ARCHIVE) - $(VERSION)"
		echo "############################"
		mkdir -p $(build_dir)
		git -C .. archive --format=tar.gz --prefix=$(NAME)-$(VERSION)/ $(VERSION) > _BUILD/$(SOURCE_ARCHIVE_DEB)

rpm:
		echo "############################"
		echo "Creating rpm package for $(OS) $(DIST)"
		echo "Creating directory: ", $(build_dir)/$(NAME)-$(VERSION)
		echo "Extracting source archive..."
		echo "############################"
		#rm -rf $(build_dir)
		mkdir -p $(build_dir)
		dnf install -y fedora-packager rpmdevtools gcc gdb cmake git boost-devel clang-devel clang-tools-extra llvm-devel ccache wget
		rpmdev-setuptree
		cp $(build_dir)/$(SOURCE_ARCHIVE) /root/rpmbuild/SOURCES/
		cp fedora/clang-include-graph.spec /root/rpmbuild/SPECS/
		rpmbuild -ba --define 'git_version ${VERSION}' /root/rpmbuild/SPECS/clang-include-graph.spec
		cp /root/rpmbuild/RPMS/x86_64/* $(build_dir)
		cp /root/rpmbuild/SRPMS/* $(build_dir)

#
# Debian
#
debian: _BUILD/$(SOURCE_ARCHIVE_DEB)
		echo "############################"
		echo "Creating debian source package for $(OS) $(DIST)"
		echo "Creating directory: ", $(build_dir)/$(NAME)-$(VERSION)-$(REBUILD)
		echo "Extracting source archive..."
		echo "############################"
		rm -rf $(build_dir)
		mkdir -p $(build_dir)
		cp _BUILD/$(SOURCE_ARCHIVE_DEB) $(build_dir)
		cd $(build_dir)
		mkdir -p $(NAME)-$(VERSION)-$(REBUILD)
		tar xf $(SOURCE_ARCHIVE_DEB) -C $(NAME)-$(VERSION)-$(REBUILD) --strip-components 1
		cp -R ../../../debian $(NAME)-$(VERSION)-$(REBUILD)/debian
		cd $(NAME)-$(VERSION)-$(REBUILD)
		$(call subst_template_dir,DATETIME,$(shell date -R),debian)
		$(call subst_template_dir,OS,${OS},debian)
		$(call subst_template_dir,NAME,${NAME},debian)
		$(call subst_template_dir,VERSION,${VERSION},debian)
		$(call subst_template_dir,REBUILD,${REBUILD},debian)
		$(call subst_template_dir,DISTRIBUTION,${DIST},debian)
		$(call subst_template_dir,MAINTAINER_NAME,${MAINTAINER_NAME},debian)
		$(call subst_template_dir,MAINTAINER_EMAIL,${MAINTAINER_EMAIL},debian)
		$(call subst_template_dir,GIT_COMMIT,${COMMIT},debian)
		$(call subst_template_dir,GIT_BRANCH,${BRANCH},debian)
		$(call subst_template_dir,DIST,${DIST},debian)
		$(call subst_template_dir,WITH_JSON,${WITH_JSON},debian)
		cp debian/control.$(DIST) debian/control
		cp debian/changelog.$(DIST) debian/changelog
		DEB_BUILD_OPTIONS='parallel=16' dpkg-buildpackage -b -us -uc

#
# Ubuntu
#
deb: _BUILD/$(SOURCE_ARCHIVE)
		echo "############################"
		echo "Creating deb source package for $(OS) $(DIST)"
		echo "Creating directory: ", $(build_dir)/$(NAME)-$(VERSION)
		echo "Extracting source archive..."
		echo "############################"
		rm -rf $(build_dir)
		mkdir -p $(build_dir)
		cp _BUILD/$(SOURCE_ARCHIVE) $(build_dir)
		cd $(build_dir)
		mkdir -p $(NAME)-$(VERSION)
		tar xf $(SOURCE_ARCHIVE) -C $(NAME)-$(VERSION) --strip-components 1
		cp -R ../../../debian $(NAME)-$(VERSION)/debian
		cd $(NAME)-$(VERSION)
		$(call subst_template_dir,DATETIME,$(shell date -R),debian)
		$(call subst_template_dir,OS,${OS},debian)
		$(call subst_template_dir,NAME,${NAME},debian)
		$(call subst_template_dir,VERSION,${VERSION},debian)
		$(call subst_template_dir,REBUILD,${REBUILD},debian)
		$(call subst_template_dir,DISTRIBUTION,${DIST},debian)
		$(call subst_template_dir,MAINTAINER_NAME,${MAINTAINER_NAME},debian)
		$(call subst_template_dir,MAINTAINER_EMAIL,${MAINTAINER_EMAIL},debian)
		$(call subst_template_dir,GIT_COMMIT,${COMMIT},debian)
		$(call subst_template_dir,GIT_BRANCH,${BRANCH},debian)
		$(call subst_template_dir,DIST,${DIST},debian)
		$(call subst_template_dir,WITH_JSON,${WITH_JSON},debian)
		mk-origtargz ../$(NAME)-$(VERSION).tar.$(TAR_EXT)
		cp debian/control.$(DIST) debian/control
		# BUILD SOURCE PACKAGE FOR LAUNCHPAD
		debuild -S -sa -us -d -k$(GPG_KEY)
		# BUILD LOCALLY BINARY PACKAGE
		# debuild -us -uc

conda: _BUILD/$(SOURCE_ARCHIVE)
		echo "############################"
		echo "Creating conda archive from source file $(SOURCE_ARCHIVE)"
		echo "############################"
		conda config --add channels conda-forge
		conda config --set channel_priority strict
		mkdir -p _BUILD/conda
		cp _BUILD/$(SOURCE_ARCHIVE) _BUILD/conda/
		cp conda/meta.yaml.in conda/meta.yaml
		$(call subst_conda_meta_yaml,PKG_VERSION,${VERSION},conda)
		$(call subst_conda_meta_yaml,PKG_SOURCE,..\/_BUILD\/clang-include-graph-$(VERSION).tar.$(TAR_EXT),conda)
		$(call subst_conda_meta_yaml,GIT_COMMIT,${COMMIT},conda)
		$(call subst_conda_meta_yaml,GIT_BRANCH,${BRANCH},conda)
		conda build --user bkryza --token $(CONDA_TOKEN) conda
