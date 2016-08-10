# getnif

ホストの nic に割り当てられている IP アドレスなどを表示する  
ifconfig や ip a の出力だとシェルなどから使いずらいのでコマンド化  

*compile*
```
gcc -O2 -std=c11 -o getnif getnif.c
```

*run*
```
$ ./getnif -d eth0 -a
10.96.150.78

$ ./getnif -d eth0 -a -b
a,10.96.150.78
b,10.96.150.255

$ ./getnif -a
10.96.150.78

$ ./getnif
Usage: ./getnif -d DEVIDE {-h|-a|-b|-m|-n}
```
