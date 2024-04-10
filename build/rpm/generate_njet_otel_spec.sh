#!/usr/bin/env bash

NJT_VERSION=`grep "define NJT_VERSION" ../../njet_main/src/core/njet.h |awk '{print $3}' |sed 's/"//g'`
sed -e "s/{{NJT_VERSION}}/$NJT_VERSION/g"  < njet-otel.spec.tmpl > njet-otel.spec

cp -r njet-otel njet-otel-$NJT_VERSION
tar cf njet-otel-$NJT_VERSION-1.el7.tar.gz njet-otel-$NJT_VERSION
mkdir -p /root/rpmbuild/SOURCES/
cp njet-otel-$NJT_VERSION-1.el7.tar.gz /root/rpmbuild/SOURCES/
