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

- Arpdiscover: Envia pacotes ARP do tipo request para cada um dos
  possíveis endereços IP da rede local e listar todos os computadores
  que responderem, que no caso será o IP do roteador e da Máquina Alvo.

- Arpspoofing: A partir do arpdiscover, atacar o roteador e a maquina
  alvo, nesse ponto iremos dizer para o alvo(maquina), que o MAC do
  roteador é o nosso MAC, e para o Roteador, o MAC do alvo é o nosso.

Ambiente utilizado: RaspberryPi3ModelB, e no uso do software foi o
Raspbian que é a distribuição Linux oficial do Raspberry Pi.

## Arpdiscover

Para o Arpdiscover, nos atentamos ao terminal que enviamos por parâmatro
a interface que é representada por wlan0, o IP da máquina alvo que é
192.168.86.102, e o IP do roteador 192.168.86.1

Tendo recebido os IPs por parâmentro são enviados dois ARP requests, um
para cada IP. Os ARP reply recebidos como resposta, são escritos na tela
como podemos ver na figura abaixo.

![Figura 1: Terminal Linux]()

## Arpspoofing

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
