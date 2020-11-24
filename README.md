### Trabalho Cliente Servidor - Protocolos de Comunicação - A 

Simulação de uma planta tanque em um servidor e controle da planta pelo cliente;

#### Build

Usar o arquivo Makefile ou maker.sh, respectivamente:

make

./maker.sh

#### Execução

Servidor:

./Server _port_ _maxOutput_ 

exemplo de servidor na porta 5010 com 40% de abertura máxima: ./Server 5010 40

Cliente: 

./Client _ip_ _port_ _reference_

exemplo de cliente ip local, porta 5010 e referencia de nível igual a 42%

./Cliente 127.0.0.1 5010 42

