#!/usr/bin/env bash

set -o errexit -o nounset -o pipefail

usage() {
  echo "Usage:"
  echo "    $(basename $0) <url|https://127.0.0.1:9000/api-docs/oas-3.0.0.json>"
}

args() {
  if [[ ! $# -eq 1 && ! $# -gt 2 ]]; then
    usage
    exit 0
  fi

  case $1 in
  "" | "-h" | "--help")
    usage
    exit 0
  esac

  URL=$1
}

# main
args "$@"

export JAVA_OPTS="-Dio.swagger.parser.util.RemoteUrl.trustAll=true -Dio.swagger.v3.parser.util.RemoteUrl.trustAll=true"

echo "[*] Generating python code..."
rm -rf ./python/*
java $JAVA_OPTS -jar openapi-generator-cli.jar generate -i "${URL}" -g python -o python --package-name=moonshine --skip-validate-spec
