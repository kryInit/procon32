# procon32

[scrapbox](https://scrapbox.io/procon32-comp-knct/)

# ソルバー関係
`procon32/algo/src`にソースが存在  
cmakeを用いて実行ファイルを生成できる  
そのままコンパイルすると最適化がかからないので`-DCMAKE_BUILD_TYPE=Release`を指定すると良い

# テスト関係
`procon32/test/`にテスト用プログラムが存在  
`tester.py`でテストができる(画像復元だけ)  

# サーバー通信関係
`procon32/server/`に通信用プログラムが存在  
`server.py`が複数コンピュータから解答を受け取ったり共有したり大会側と通信するプログラム  
`solver_manager.py`がソルバーの起動や`general_server`との通信を行うプログラム  

# ビジュアライザ
`OpenSiv3D`を用いて作成  
`procon32/visualizer`にソースだけ存在(OSによってコンパイル方法が異なるため)  

# その他
`procon32/utility`に問題画像から問題設定を抜き出してテキストに書き出したり問題画像を複数の断片画像に分割したり解答と問題画像から画像を復元したりするプログラムが存在  
`procon32/.data`に問題や解答を保存する  
問題によってディレクトリが別れ、ディレクトリ名は問題画像のhash値になる  
