kiban2dem
=========

基盤地図情報 標高DEMデータ変換ツール

このツールをLinuxへ移植して改良したもの
> 基盤地図情報 標高DEMデータ変換ツール | コンテンツ | 株式会社エコリス<br />
> http://www.ecoris.co.jp/contents/demtool.html


コンパイル<br />
gcc -I/usr/local/include/ -L/usr/local/lib dem_linux.cpp /usr/local/lib/libgdal.so -o xmldem

実行<br />
./xmldem filename nodetaflag
> nodetaflag=0 -> nodata=0<br />
> nodetaflag=1 -> nodata=-9999

ex) ./xmldem test.xml 1<br />


まとめて実行<br />
for i in *.xml ; do ../xmldem $i 1 ; mv *.tif ../demtif/ ; done

