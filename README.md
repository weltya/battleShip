# 2-player network naval battle

## to play :

`run make` :
* clone repository.
* go to the repository.
* run `make`.
```sh
git clone https://github.com/weltya/battleship.git
cd battleship
make
```

`run server` :
* args :
  * executable name.
  * port.
  
Example :
```sh
./server_battleship 1664
```

`run client` (copy this commands in 2 terminal) :
* args :
  * executable name.
  * server IP.
  * port.
  * Map.
  
Example :
```sh
./client_battleship 192.168.1.82 1664 battleship-map1
```

