# Redes de Computadores I - Ataque ARP Poisoning

- PUCRS
- Marina Serial Experiments Lain de Barros
- 11 de novembro de 2017

---

### Resumo

O objetivo do trabalho é em sua essência o estudo do protocolo ARP, que
será apresentado como uma aplicação usando sockets raw que possa ser
utilizada para estudar o protocolo ARP e demonstrar um ataque do tipo
ARP poisoning fazendo um _man-in-the-middle_. Esse tipo de ataque consiste
em enviar pacotes ARP de modo a modificar a tabela ARP de um computador
alvo e permitir o redirecionamento de tráfego de rede para um computador
intermediário.

## Introdução

Diante do que foi proposto, seguiremos as seguintes linhas:

- **ARP Discover**: Envia pacotes ARP do tipo _request_ para cada um dos
  possíveis endereços IP da rede local e lista todos os computadores que
  responderem.

- **ARP Spoofing**: A partir do *ARP Discover*, atacar o roteador e a
  maquina alvo, nesse ponto iremos mandar pacotes ARP do tipo _reply_
  informando informações falsas. Dizer para a máquina alvo que o MAC do
  roteador é o nosso, e dizer para o roteador que o MAC da máquina alvo
  é o nosso.

O sistema operacional utilizado para o desenvolvimento e teste da
aplicações foi o _Raspbian_, rodando em um _Raspberry Pi 3 - Model B_.

O fluxo do código acontece seguindo as seguintes etapas:

1. Configuração do _socket_.
2. Envio de ARP _request_ para a máquina alvo.
3. Recebimento e processamento do ARP _reply_ da máquina alvo.
    - Se obtém o endereço MAC da máquina alvo.
4. Envio de ARP _request_ para o roteador.
5. Recebimento e processamento do ARP _reply_ do roteador.
    - Se obtém o endereço MAC do roteador.
6. Construção e envio de ARP reply para a máquina alvo.
    - Passando o nosso MAC como sendo o do roteador.
7. Construção e envio de ARP reply para o roteador.
    - Passando o nosso MAC como sendo o da máquina alvo.

ARP Discover é a implementação das etapas 2, 3, 4 e 5. ARP Spoofing é a
repetição constante das etapas 6 e 7.

Para o desenvolvimento da nossa solução, foi utilizado a _struct_
`arphdr` declarada no _header_ `linux/if_arp.h`, encontrado em quase
todos sistemas GNU/Linux. Também foi definida a seguinte _struct_ para
trabalharmos com o conteúdo de um pacote ARP.

```c
struct arp_data
{
        unsigned char sender_mac[6];
        unsigned char sender_ip[6];
        unsigned char target_mac[6];
        unsigned char target_ip[6];
};
```

As demais estruturas de dados utilizadas foram provenientes dos códigos
exemplos encontrados no _moodle_ da disciplina. O trabalho foi todo
implementado utilizando a linguagem C e o **G**NU **C** **C**ompiler.

Os envolvidos no experimento realizado, utilizando o nosso programa,
foram os seguintes dispositivos:

|       Dispositivo       |       IP       |        MAC        |
|:-----------------------:|:--------------:|:-----------------:|
|        Roteador         | 192.168.86.1   | 70:3a:cb:80:96:47 |
| Raspberry Pi (atacante) | 192.168.86.143 | b8:27:eb:f4:bc:da |
| Notebook Samsung (alvo) | 192.168.86.102 | c4:85:08:92:f4:6e |

## ARP Discover

O programa começa com o recebimento de três parâmetros e na seguinte
ordem:

1. Interface de rede
2. IP da máquina alvo
3. IP do roteador

Na fase de ARP Discover, nosso objetivo é descobrir o endereço MAC dos
dois endereços de IP fornecidos. Não acontece nenhum tipo de ataque
durante o ARP Discover. São montados e enviados pacotes ARP _request_
para o roteador e a máquina alvo.  Ao receber os ARP _reply_, os dados
são impressos na tela do terminal.

![ARP Discover](https://i.imgur.com/aq2iKBk.png)

## ARP Spoofing

Tendo os MACS necessários para a realização do ataque, são montados e
enviados dois pacotes ARP *Reply* constantemente. Um que diz para a
máquina alvo que o MAC do roteador é o nosso MAC, e outro que diz para o
roteador que o MAC da máquina alvo é o nosso MAC. Isso pode ser
observado na captura de tela abaixo, onde mostramos a saída do comando
`arp -n`, executado na máquina alvo, após o ataque.

![arp -n](https://i.imgur.com/mdfcGpZ.png)

Utilizando o programa Wireshark, capturamos os pacotes de rede que
trafegavam no momento e conseguimos ver exatamente o fluxo descrito na
introdução acontecendo.

![wireshark](https://i.imgur.com/d9yIPIC.png)

## Verificação do funcionamento

Para verificar que o programa funcionava corretamente, realizamos
requisições HTTP, a partir da máquina alvo, após ter sido atacada. Se
tudo estiver funcionando como planejamos, o pacote que ela vai enviar
deve ser destinado ao MAC da máquina atacante, o Raspberry Pi. Sob
condições normais, o endereço correto seria o do roteador.

![Redirecionamento HTTP](https://i.imgur.com/kOZlppS.png)

## Conclusões

Com a realização desse trabalho foi possível constatar o quão frágil é a
segurança da rede sem as técnicas de segurança mais modernas. É possível
olhar os pacotes destinados a outro computador, e ainda analisar seu
conteúdo. Protocolos que utilizam criptografia, como o HTTPS, ajudam a
previnir o roubo das informações nesse tipo de ataque.
