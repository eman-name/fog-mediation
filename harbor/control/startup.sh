#!/bin/sh

set -eu

export PATH=${PATH}:${GOPATH}/bin

cd /mnt
sudo -E -u \#$HOST_UID mkdir -p .rpc/.go
sudo -E -u \#$HOST_UID go get -u github.com/golang/protobuf/proto
sudo -E -u \#$HOST_UID go get -u github.com/golang/protobuf/protoc-gen-go
sudo -E -u \#$HOST_UID protoc .rpc/control.proto --go_out=plugins=grpc:src
sudo -E -u \#$HOST_UID sh -c "cd src && go get -d ./..."
sudo -E -u \#$HOST_UID go run src/main.go
