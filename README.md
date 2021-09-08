## Trabalho 1

O projeto foi feito:
- Wagner Martins
- Renato Britto
- Thiago Luiz

## Arquitetura

Aqui temos um diagrama da comunicação:

```
  samplefile.json ------                          --------> stdout final
                       |                          |
                       v                          |
                   e_client                   r_client
                       |                          ^
                _______|______               _____|_______
                |message queue|             |message queue|
                ‾‾‾‾‾‾‾|‾‾‾‾‾‾               ‾‾‾‾‾|‾‾‾‾‾‾‾
                       v       ___________        |
                     e_node ---|canal UDP|----> r_node
                               ‾‾‾‾‾‾‾‾‾‾‾
```


## Uso

Compile com
```
compile.sh
```

Rode, em um terminal, com
```
./run_half_duplex.sh server <tamanho do quadro>
```

Rode, em outro terminal terminal, com
```
./run_half_duplex.sh cliente <mesmo tamanho do quadro>
```

Procure arquivos com 
```
ls <path> 
por exemplo:
ls .
```

Envie arquivos com 
```
send <path para arquivo>
por exemplo:
send samplefile.json
```

Consulte o resultados dos arquivos transmitidos em
```
/tmp/client.out (quando enviado do server -> client)
/tmp/server.out (quando enviado do client -> server)
```