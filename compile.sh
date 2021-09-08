#!/usr/bin/bash

echo "Compilando camada de enlace: cliente"
gcc src/link_layer/bidiretional_node_a.c src/util/frame.c src/util/mq_utils.c -o bin/client -lrt

echo "Compilando camada de enlace: server"
gcc src/link_layer/bidiretional_node_b.c src/util/frame.c src/util/mq_utils.c -o bin/server -lrt

echo "Compilando camada superior"
gcc src/higher_layer/app.c src/util/mq_utils.c -o bin/app -lrt -pthread
