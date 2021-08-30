# Compila os 4 binários
gcc emitter/emitting_client.c -o bin/e_client
gcc emitter/emitting_node.c -o bin/e_node
gcc receiver/receiving_node.c -o bin/r_node
gcc receiver/receiving_client.c -o bin/r_client

# Roda, na ordem de primeiro para último na lista, os binários
./bin/r_client                     & \
./bin/r_node 50 0.0.0.0 8000       & \
./bin/e_node 50 0.0.0.0 8000       & \
./bin/e_client samplefile.json
