いろいろな設定とか

実験のパラメータはすべてsettings.hに記入

各変数、パラメータについて
cam : どのカメラを使うか
    LEFT_CAM 左のカメラ
    CENTER_CAM 中央のカメラ
    RIGHT_CAM 右のカメラ

f : 
焦点距離．

zoom, zoomView : 
ズーム(1倍~10倍)．全てのカメラで同じ
画角(40°)から計算

fov:
画角．zoomより計算される

baseLine:
基線長の長さ．
zoom, winWidth, MAX_DISPARITYに依存

winWidth, winHeight : 
ウィンドウの大きさ

eye:
カメラの視点．基線長に依存
中央のカメラは原点にあり，
左のカメラは 基線長/2 だけ左に
右のカメラは 基線長/2 だけ右に
それぞれ平行移動した所のある

FocalDepth:
カメラの焦点のあっている奥行き．
それぞれのカメラによって異なる

aperturePattern:
開口形状

apertureSize:
開口径．全てのカメラで同じ
MAX_PSF_RADIUS, zMin, zMax, winWidth, zoomに依存する

DTPParam:
Disparity To Psfsize Parameter
視差とPSFサイズを変換するパラメータ
左右のカメラで異なる
PSFSize = disparity * DTPParam[0] + DTPParam[1]




使った式

視差(d)とカメラの移動距離(s)の計算
d = W*s / ( 2 * z * tan ( fov / 2)
W : ウィンドウ幅
z : 物体までの奥行き
fov : 視野角(x軸方向)


開口径(D)の計算式
B = D ( 1 - z/ z' ) R
B : 許容錯乱円径(カメラ座標系)
z : 合焦位置までの距離
z': ぼけBまでの距離
R : 倍率 ( f / z )

カメラ座標系からウィンドウ座標系への変換
2f * tan( fov / 2) : B = WinWidth : Max PSF Size
より
B = MPS* 2f tan(fov/2) / WinWidth
よって
D = 2 * R * z * z' * tan(fov/2) / w ( z' -z )

無限遠でぼけが最大になるという仮定をおくと
z' : 無限遠 = z : D
となる


画面上での距離(z)と視差(d)の変換
ウィンドウ幅 : W
基線長 : b
視野角 : fov
とすると

d = ( W*b )/ (2*z*tan(fov/2) )


ぼけ径と奥行きの関係
GL上でのぼけ径をB
奥行きをzとする
開口径:D
ピント位置:aとすると

B = D * f *( 1/b - 1/z )

でGL座標系におけるぼけ径が求まる．
これを画面上でのPSFサイズ(p)に変換すると
p = W*D/(2* tan(fov)/2 ) * ( 1/a - 1/z )
となる


ぼけ径と視差の関係
画面上での視差(d)は
d = W*b / (2 * z * tan(fov/2) )
となる (b : 基線長)
また画面上でのPSFサイズ(p)は
p = W*D/(2*tan(fov/2)) * ( 1/a - 1/z)
となる (D:開口径, a:ピントの奥行き)

この二つを合わせると
p = (D/b) * d - ( W*D/2*tan(fov/2) ) / a
で変換できる(視差とPSFサイズは線形関係)

zoomとtmfの使い方
よく出てくる tan(fov/2)は
fov = 2.0 * atan(tmf/zoom)
に代入すると
tan(fov/2) = tmf/zoom
とできる
