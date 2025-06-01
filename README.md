# webserv
webservはHTTP、HTTPサーバーの学習を目的にC++98で作成されたHTTPサーバーです。以下のような特徴があります
- Non-Blocking I/Oによる入出力
- シングルプロセスでの動作
- CGIの呼び出しが可能

このサーバーはRFC9110, RFC9112について部分的に準拠いますが、学習を目的として作成されたため、他サーバーに比べて性能や機能に制限があります。


## システム要件

以下のアプリケーションがインストールされている必要があります
- `g++` (g++以外のC++用のコンパイラでもコンパイルできると思いますが、未検証です) 
- `make`

以下の環境での動作を確認しています。
|OS|コンパイラ|make|
|-----|------|-----|
|Debian GNU/Linux 12.9 (bookworm) (Linux debian 6.1.0-31-amd64 #1 SMP PREEMPT_DYNAMIC Debian 6.1.128-1 (2025-02-07) x86_64 GNU/Linux)|g++ (Debian 12.2.0-14) 12.2.0|GNU Make 4.3|
|macOS Monterey 12.3.1 (Darwin Kernel Version 21.4.0: Fri Mar 18 00:45:05 PDT 2022; root:xnu-8020.101.4~15/RELEASE_X86_64)|g++-14(Homebrew GCC 14.2.0_1) 14.2.0|GNU Make 3.81|

## ビルド方法
```
git clone git@github.com:ndmxjp/webserv.git
cd ./webserv
make
```
起動方法
```
./webserv ./path/to/config/file     # configファイルを直接指定する場合
./webserv                           # デフォルトのconfig(./config/default.conf)を使用する場合
```

#### configファイル
[config_bnf.lark](config_bnf.lark)に基づいたconfigファイルによってルーティングやオプションを設定することができます。

sample
```
server {
    listen 8080;
    server_name localhost;
    error_page 404 ./docs/error_page/404.html;


    location / {
        method GET;
        root ./docs/;
        index index.html;
        autoindex on;
    }
}
```

## Docker環境
動作確認を目的として、Dockerでの実行ができます。
- 初期状態では、`docker_resource/server.conf`がconfigファイルとして使用されます
- `docker_resource/html`内のファイルがドキュメントルートにコピーされます。
- [chess-timer](https://github.com/maitneel/chess-timer)がドキュメントルートにcloneされます。
- CGIのサンプルとして`./cgi_script/message_board`(匿名掲示板のようなプログラムです)がコンテナ内にコピーされ、`/message_board/`にルーティングされます。

`make docker`で実行ができるようになっていますが、Docker起動後にconfigや表示するファイル、Dockerfileなどを変更した場合は再度ビルドが必要になるため、`docker compose build`を実行してください。


