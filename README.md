kiban2dem
=========

基盤地図情報 標高DEMデータ変換ツール

このツールをLinuxへ移植して改良したもの

http://www.ecoris.co.jp/contents/demtool.html


コンパイル
gcc -I/usr/local/include/ -L/usr/local/lib dem_linux.cpp /usr/local/lib/libgdal.so -o xmldem

実行
./xmldem filename nodetaflag
 nodetaflag=0 -> nodata=0
 nodetaflag=1 -> nodata=-9999
ex) ./xmldem test.xml 1


まとめて実行
for i in *.xml ; do ../xmldem $i 1 ; mv *.tif ../demtif/ ; done

