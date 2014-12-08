#!/bin/bash

set -e

if [ "$(uname -s)" == "Darwin" ]
then
  PROTOC=./build/Debug/protoc
else
  PROTOC=./out/Debug/protoc
fi

find include src -type f -name '*.proto' |
while read PROTO_FILE
do
  echo Running protobuf compiler for $PROTO_FILE
  $PROTOC --proto_path=include --proto_path=src --cpp_out=src $PROTO_FILE
  if [[ $PROTO_FILE == include/* ]]
  then
    RELATIVE_PROTO_FILE=${PROTO_FILE#include/}
    mv src/${RELATIVE_PROTO_FILE%.proto}.pb.h ${PROTO_FILE%.proto}.pb.h
  fi
done

find use_cases -type f -name '*.proto' |
while read PROTO_FILE
do
  PROTO_DIR=${PROTO_FILE%/*}
  echo Running protobuf compiler for $PROTO_FILE
  $PROTOC --proto_path=include --proto_path=$PROTO_DIR --cpp_out=$PROTO_DIR $PROTO_FILE
done
