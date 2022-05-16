# RDTProtocol
Implementation of the theoretical network protocol "RDT"

## :warning: OS Windows limits
Due to socket.h incompatibility, these programs do not work in Windows OS

## :notebook: Requirements

- gcc compiler
- C standard library

## :zap: Usage
```bash
git clone https://github.com/Daedalus9/RDTProtocol
cd RDTProtocol/<RDT version>/
gcc client.c -o client.out
gcc server.c -o server.out
./server.out
```

In another bash:

```bash
./client.out
```
