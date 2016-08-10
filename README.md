# getnif

ifconfig や ip a の出力だとシェルなどから使いずらいのでコマンド化  

*compile*
```
gcc -O2 -std=c11 -o getnif getnif.c
```

*run*
```
./getnif -d eth0 -a
./getnif -d eth0 -a -b
./getnif -a
```
