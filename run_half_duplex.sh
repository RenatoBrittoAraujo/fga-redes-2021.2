#!/usr/bin/bash

# Cada script deve ser executado em um terminal diferente.

if [[ $1 == "" ]];
then
    echo "usage: $1 [client|server] <tamanho da dpu>"
fi

# Header + trailer da DPU tem 11 bytes, logo o tamanho mínimo são 12 para enviar alguma informação
if [[ $1 == "client" && $2 -gt 11 ]];
then
    ./bin/client $2 &
    ./bin/app client $!
fi

if [[ $1 == "server" && $2 -gt 11 ]];
then
    ./bin/server $2 &
    ./bin/app server $!
fi
