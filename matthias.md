# Redes de Computadores I - Ataque ARP Poisoning

- PUCRS
- Marina Lain de Barros
- 11 de novembro de 2017

---

### Resumo

O objetivo do trabalho é em sua essência o estudo do protocolo ARP, que
será apresentado como uma aplicação usando sockets raw que possa ser
utilizada para estudar o protocolo ARP e demonstrar um ataque do tipo
ARP poisoning fazendo um man-in-the-middle. Esse tipo de ataque consiste
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
dois IPs fornecidos. Não acontece nenhum tipo de ataque durante o ARP
Discover.

Para o Arpdiscover, nos atentamos ao terminal que enviamos por parâmatro
a interface que é representada por wlan0, o IP da máquina alvo que é
192.168.86.102, e o IP do roteador 192.168.86.1

Tendo recebido os IPs por parâmentro são enviados dois ARP requests, um
para cada IP. Os ARP reply recebidos como resposta, são escritos na tela
como podemos ver na figura abaixo.

![Figura 1: Terminal Linux]()

## ARP Spoofing

Tendo os MACS necessários para a realização do ataque (descobertos no
Arpdiscover), são montados e enviados dois pacotes Arp Reply
constantemente. Um que diz para a máquina alvo que o MAC do roteador é o
nosso mac, e outro que diz para o roteador que o mac da máquina alvo é o
nosso MAC. Isso pode ser observado na captura de tela abaixo, onde
observávamos a rede através do programa Wireshark.

![Figura 2: ARP no Wireshark]()

## Verificação do funcionamento

![Figura 3: Tabelas ARP]()

![Figura 4: Tabelas ARP -n]()

## Redirecionamento de tráfego HTTP

Para o redirecionamento, utilizamos o comando ARP, que foi efetuado na
seção acima, e vimos no Wireshark filtrando por HTTP. Abaixo na imagem,
é possivel ver que o destino é de fato o RaspberryPi

![Figura 5: HTTP no Wireshark]()

## Conclusões

Foi possível constatar que através do Arpdiscover, é possível obter os
endereços físicos, passando os IPS, necessários, e que a partir deles,
temos o necessário para realizar o Arpspoofing que ocorre quando eu
informo o roteador que o MAC do alvo é o nosso, assim como eu digo para
o IP alvo que o MAC do roteador sou eu.
