## Trabalho 1

Para compilar, levantar a rede e fazer upload do arquivo "samplefile.json":

```
./run.sh
```

Para testar, é usado o arquivo de teste `samplefile.json`

Note que não foi usado MakeFile ou Docker, para que os scripts deixem bem 
transparente o que está acontecendo.

## Arquitetura

Aqui temos um diagrama da comunicação:

```
  samplefile.json ------                         --------> stdout final
                       |                         |
                       v                         |
                   e_client                  r_client
                       |                         ^
                _______|______              _____|_______
                |message queue|            |message queue|
                ‾‾‾‾‾‾‾|‾‾‾‾‾‾             ‾‾‾‾‾‾|‾‾‾‾‾‾‾
                       v       __________        |
                     e_node ---|canal TCP|---> r_node
                               ‾‾‾‾‾‾‾‾‾‾‾
```

O canal TCP, no transmissor, aleatóriamente introduz erros na mensagem prestes
a ser enviada, para simular erro de comunicação. Quem é responsável por detectar
ou requisitar a re-emissão do sinal é o cliente, até que, por pura chance, a
mensagem recebida seja válida.

## Uso

### e_client

```
./e_client <filepath>
```

### e_node

```
./e_node <tamanho do quadro> <ip para envio> <porta para envio>
```

### r_node

```
./r_node <tamanho do quadro> <ip do servidor> <porta do servidor>
```

### r_client

```
./r_client
```

## Diário

Fiz o código boilerplate para compilar e rodar o programa, além dos 4 arquivos
reposáveis por 4 processos: cliente de leitura de arquivo, nodo de envio de
arquivo, nodo de recebimento de arquivo e, finalmente, cliente que printa o
arquivo recebido no terminal. Com estes quatro, será possível testar o processo
de comunicação de redes pedido pelo projeto.

Completei a leitura de arquivo e criei sistema básico de transmissão entre
os processos dos emissores e os processos dos recebidores.

Encontrei um problema nas queues de transmissão de mensagem de um processo
para outro. O problema é que, as vezes, mensagens são lidas multiplas vezes
da fila e perdem a ordem, corrompendo a mesagem. Eu poderia resolver isso 
com múltiplas retransmissões mas sinto que este tipo de problema deveria
ser resolvido lendo a documentação com calma.

O problema é a natureza de como funciona a trasmissão de dados com as queues.
Foi resolvido com uma solução feia porém simples: o recebedor envia uma mensagem
de recebimento e o emissor recebe ela. Daí o próximo pacote é retransmitido até
que se complete a comunição IPC. Com certeza é possível encontrar uma solução
mais elegante, porém não é prioridade: a prioridade é comunicações TCP entre
os dois processos nodo.

Não é relacionado ao projeto, mas encontrei um bug que me gastou horas para
depurar: por algum motivo, quando eu rodava a função de capturar mensagens da
queue, as variáveis do argv paravam de funcionar. Não conseguia entender e
gastei umas boas horas tentando raciocinar. Finalmente, resolvi imprimir os locais
dos ponteiros de string e notei que eles ficavam muito perto um do outro,
principalmente o ponteiro que guarda string o arquivo no nodo. Suspeitei que a
memória do argv estava sendo sobrescrevida pelo conteúdo do arquivo. Então, eu
mudei o arquivo do que era para apenas "{}" e o problema sumiu, que confirmou a
hipótese. Daí alterei o código para instanciar o ponteiro apenas dentro da
função em um lugar de memória descontínuo do argv e o problema se resolveu.

backlog:
- fazer comunicações entre nodos
- criar sistema de "quebra randômica" dos dados para testar a integridade da 
  transmissão.
- criar sistema de recuperação por retransmissão de dados corrompidos (bytes).
- transformar tudo feito em half-duplex, realizado por 2 binários em 4 processos.

## Pontos de melhoria

- A comunicação na fila poderia ser feita de forma mais apropriada quando se trata
  de ordem. O transmissor só emite mensagens quando o receptor captura uma nova, o
  que, não necessáriamente, precisa acontecer.
- Para implementar half-duplex, seria interessante usar threads. Atualmente, existe
  apenas uma thread relevante em todos os processos: a principal. 