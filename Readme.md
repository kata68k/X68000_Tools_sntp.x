# X68000 SNTP Client for Human68k

X68000 / Human68k からネットワーク上の SNTP/NTP サーバへ問い合わせ、現在時刻を取得するための SNTP クライアントです。

Sharp X68000 と、計測技研（株式会社計測技研）の **Ethernet Starter Pack for X680x0（ESP/X）** に含まれる Human68k TCP/IP 環境および `libnetwork.a` のソケット API を対象として開発しています。
 
## 1. 開発のいきさつ

X68000 をネットワークへ接続して使用していると、ネットワーク上から現在時刻を取得して、本体の時計を合わせたくなることがあります。

現在の PC や UNIX 系 OS では NTP クライアントは一般的ですが、X68000 / Human68k の ESP/X 環境では、現代的な NTP ライブラリをそのまま利用することはできません。

そこで、ESP/X の `libnetwork.a` のソケット API を使って、

1. SNTP/NTP サーバへ UDP で問い合わせる
2. 応答パケットを受信する
3. NTP の時刻情報から UNIX 時刻を取得する
4. UTC を日本標準時（JST）へ変換する
5. Human68k の RTC に反映する

という処理を X68000 上で動作する C プログラムとして実装することを目的にしました。

## 2. 対象環境

- Sharp X68000
- Human68k
- 計測技研 Ethernet Starter Pack for X680x0（ESP/X）
- ESP/X の TCP/IP 環境
- ESP/X の `libnetwork.a`
- IPv4 ネットワーク
- UDP
- SNTP / NTP
- C90 相当の C コンパイラ

X68000 固有のネットワーク環境を前提としているため、一般的な Windows / Linux 用 NTP クライアントではありません。

## 3. 実装方針

X68000 での使用を考え、できるだけ単純な構成にしています。

- C90 ベース
- 浮動小数点を使用しない
- 64bit 整数を前提にしない
- NTP の 64bit タイムスタンプは 32bit の秒フィールドを中心に扱う
- IPv4 を対象とする
- UDP/123 を使用する
- ESP/X のソケット API を使用する
- 必要以上に大きなライブラリへ依存しない

NTP のタイムスタンプそのものは 32bit の秒＋32bit の小数部ですが、今回必要なのは現在時刻の取得なので、秒フィールドを利用することで 32bit 環境でも処理できるようにしています。

## 4. SNTP の通信

SNTP/NTP は UDP のポート 123 を使用します。

クライアントからサーバへ送る最小限の要求パケットは 48 バイトです。

```text
23 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
...
```

先頭の `0x23` は、

- LI = 0
- Version = 4
- Mode = 3（Client）

を表します。

## 5. 通信の流れ

```text
             X68000
                |
                | UDP/123
                v
        +----------------+
        | SNTP/NTP Server|
        +----------------+
                |
                | 48 byte reply
                v
             X68000
                |
                v
        NTP seconds field
                |
                v
          UNIX time
                |
                v
           UTC -> JST
                |
                v
        Human68k RTC
```

## 6. ESP/X の `socklen()` について

ESP/X のソケット API を調べる際には、付属ソースの `ping.c` が非常に参考になります。

`ping.c` では `recvfrom()` の前に、

```c
if (!socklen(ping_socket, 0))
    return;

len = recvfrom(
    ping_socket,
    buff,
    ping.len + sizeof(struct icmp),
    0,
    (char *)&from,
    &fromlen
);
```

という処理を行っています。

このことから、ESP/X では受信データの有無を

```c
socklen(sock, 0)
```

で確認してから `recvfrom()` を呼ぶ方法を採用しています。

本プログラムでも、この ESP/X の実装に合わせています。

## 7. 開発中に遭遇した重要な問題

今回、SNTP が動作しない原因として、ESP/X の `libnetwork.a` に関する非常に重要な問題が見つかりました。

### `gethostbyname()` の `h_length`

一般的な BSD ソケット環境では、以下のようなコードは普通に使用できます。

```c
host = gethostbyname(server);

memcpy(
    &addr.sin_addr,
    host->h_addr,
    host->h_length
);
```

ところが、今回使用している計測技研の `libnetwork.a` では、実際の動作確認において `gethostbyname()` 後の

```c
host->h_length
```

が正しく設定されないケースが確認されました。

そのため、上記の `memcpy()` を行うとコピーサイズが 0 バイトになり、

```c
addr.sin_addr
```

に送信先 IP アドレスが設定されません。

結果として、

- DNS は成功しているように見える
- `gethostbyname()` も戻ってくる
- しかし送信先アドレスが設定されていない
- `sendto()` は呼び出せる
- SNTP サーバから応答が来ない
- `socklen()` がいつまでも 0
- `recvfrom()` まで到達できない

という状態になりました。

### 対策

今回のプログラムでは IPv4 を対象としているため、コピーサイズを明示的に 4 バイトとしています。

```c
memcpy(
    &addr.sin_addr.s_addr,
    host->h_addr,
    4
);
```

これは一般的な BSD ソケットプログラムでは必ずしも必要な書き方ではありませんが、本プロジェクトで使用している ESP/X の `libnetwork.a` では重要な回避策となりました。

> 注意：これは今回使用した ESP/X / `libnetwork.a` 環境で確認された問題です。すべての Human68k ネットワーク環境に同じ問題があるとは限りません。

## 8. DNS を使わずに動作確認する

DNS 周りを切り分けたい場合は、SNTP サーバを直接 IPv4 アドレスで指定してテストできます。

この方法なら、

```text
DNS
 ↓
gethostbyname()
 ↓
hostent
```

の部分を経由しないため、DNS が原因なのか、UDP 通信が原因なのかを切り分けることができます。

## 9. `ntp_query()` の使い方

SNTP 通信部分は `ntp_query()` にまとめています。

概念的には以下のように使用します。

```c
unsigned long unix_time;
int result;

result = ntp_query(
    "pool.ntp.org",
    5000,
    &unix_time
);

if (result == NTP_SUCCESS)
{
    printf("UNIX TIME = %lu\n", unix_time);
}
```

第1引数：SNTP/NTP サーバ名または IPv4 アドレス

第2引数：タイムアウト時間（ms）

第3引数：取得した UNIX 時刻を格納する変数

## 10. エラーコード

| エラー | 内容 |
|---|---|
| `NTP_SUCCESS` | 成功 |
| `NTP_ERR_DNS` | DNS 名前解決失敗 |
| `NTP_ERR_SOCKET` | ソケット作成・設定失敗 |
| `NTP_ERR_SEND` | SNTP パケット送信失敗 |
| `NTP_ERR_RECV` | パケット受信失敗 |
| `NTP_ERR_PACKET` | 不正な SNTP/NTP パケット |
| `NTP_ERR_TIMEOUT` | タイムアウト |

実際の定義は `ntp.h` を参照してください。

## 11. タイムアウトについて

ESP/X では、受信データが到着したかどうかを

```c
socklen(sock, 0)
```

で確認できます。

そのため、

```c
for (;;)
{
    if (socklen(sock, 0) > 0)
        break;

    if (timeout)
    {
        /* timeout */
        break;
    }
}
```

というポーリング方式を採用しています。

### `clock()` の注意

C の `clock()` が返す値は必ずしもミリ秒ではありません。

そのため、

```c
clock() - start
```

を直接 `timeout_ms` と比較するのではなく、

```c
CLOCKS_PER_SEC
```

を利用してクロック単位へ変換する必要があります。

## 12. UNIX 時刻

NTP で得られる秒数は NTP epoch（1900-01-01）を基準にしています。

一方、一般的な UNIX time は、

```text
1970-01-01 00:00:00 UTC
```

を基準としています。

そのため、NTP seconds から UNIX time へ変換する場合は epoch の差を考慮します。

```text
NTP epoch
1900-01-01
      |
      | 70年
      v
UNIX epoch
1970-01-01
```

## 13. UTC から日本標準時（JST）へ

NTP から取得した時刻は UTC です。

日本標準時は UTC+9 なので、

```c
unix_time += 9UL * 60UL * 60UL;
```

として JST へ変換できます。

本プログラムでは日本標準時を固定で +9 時間として扱います。

## 14. 日付への変換

UNIX time を

```text
YYYY-MM-DD HH:MM:SS
```

へ変換する処理は、`gmtime()` や `localtime()` などのホスト OS に依存する関数を使用せず、整数演算で行う構成を想定しています。

X68000 で使用することを考え、

- 浮動小数点を使用しない
- 64bit 整数を前提にしない
- 32bit の範囲で処理する

ことを重視しています。

## 15. Human68k RTC への反映

最終的には、

```text
SNTP
 ↓
NTP time
 ↓
UNIX time
 ↓
UTC
 ↓
JST
 ↓
年/月/日・時/分/秒
 ↓
Human68k RTC
```

という流れで、本体の時計をネットワーク時刻へ合わせることができます。

RTC の設定部分については、使用するコンパイラや Human68k のライブラリ/API によって利用できる関数が異なる場合があるため、環境依存部分として分離することを推奨します。

## 16. ソース構成

```text
README.md
ntp.h
ntp.c
date.h
date.c
rtc.h
rtc.c
main.c
```

### `ntp.c`

SNTP/NTP サーバとの UDP 通信を担当します。

```text
DNS
socket
bind
sendto
socklen
recvfrom
NTP packet validation
NTP -> UNIX time
```

### `date.c`

時刻変換処理を担当します。

```text
UNIX time
UTC
JST
年月日時分秒
```

### `rtc.c`

Human68k の RTC へ時刻を書き込む環境依存部分です。

## 17. デバッグ時の確認ポイント

SNTP が動作しない場合は、以下の順番で確認すると切り分けやすくなります。

1. ESP/X で通常のネットワーク通信ができることを確認する。
2. `gethostbyname()` の結果を表示する。
3. `host->h_length` を確認する。
4. `addr.sin_addr` に正しい送信先 IP が入っていることを確認する。
5. `sendto()` の戻り値が 48 であることを確認する。
6. `socklen(sock, 0)` が受信時に 0 より大きくなることを確認する。
7. `recvfrom()` で応答パケットを取得する。

## 18. SNTP が動作しない場合の切り分け

今回の開発では、最初は、

```text
sendto()
 ↓
socklen() == 0
 ↓
recvfrom() できない
```

という状態になりました。

この場合、SNTP サーバが悪いと決めつけるのではなく、まず送信先アドレスを確認してください。

特に、

```c
gethostbyname()
```

を使用している場合は、

```c
host->h_addr
host->h_length
addr.sin_addr
```

を確認します。

今回のケースでは、`h_length` が設定されないことによる 0 バイトコピーが原因でした。

## 19. 制約事項

### IPv4 専用

現在の実装は IPv4 を前提としています。IPv6 は対象外です。

### UDP/123

SNTP/NTP の標準的な UDP ポートである 123 番を使用します。

### ESP/X 依存

ソケット API は ESP/X の `libnetwork.a` を前提としています。

したがって、一般的な BSD socket / Winsock / Linux socket と完全互換ではありません。

### `libnetwork.a` の挙動

特に DNS の `hostent` 構造体については、一般的な UNIX 環境と異なる挙動が確認されています。

他の TCP/IP スタックで使用する場合は、動作を確認してください。

### 32bit 時刻

UNIX time を 32bit の `unsigned long` として扱う構成では、2038 年問題の影響を受けます。

本プロジェクトでは、X68000 / Human68k での実用性と実装の単純さを優先しています。

### JST 固定

現在の構成では日本標準時を前提として、

```text
UTC + 9時間
```

としています。

任意のタイムゾーンには対応していません。

## 20. なぜ `ping.c` を参考にしたのか

ESP/X のネットワーク API は、現在一般的に使われている BSD socket API と似ていますが、細かな動作を同じだと決めつけることはできません。

そこで、ESP/X の環境で実際に動作しているプログラムを確認することが重要になります。

今回特に参考になったのが ESP/X に付属している `ping.c` です。

`ping.c` から、

```c
socket()
sendto()
socklen()
recvfrom()
close_s()
```

などの実際の使用方法を確認し、それに合わせて SNTP クライアントを実装しました。

## 21. 最小構成での動作確認

最初から RTC を変更するのではなく、まず SNTP サーバから時刻を取得できるところまで確認することを推奨します。

```text
[STEP 1]

socket()
    |
    v
sendto()
    |
    v
SNTP server
    |
    v
socklen()
    |
    v
recvfrom()
    |
    v
NTP packet
```

これが成功したら、

```text
[STEP 2]

NTP seconds
    |
    v
UNIX time
```

を確認します。

さらに、

```text
[STEP 3]

UNIX time
    |
    v
UTC -> JST
    |
    v
YYYY/MM/DD HH:MM:SS
```

を確認します。

最後に、

```text
[STEP 4]

Human68k RTC
```

へ反映します。

## 22. プロジェクトの目的

このプログラムの目的は、最新の PC のような高機能な NTP クライアントを作ることではありません。

むしろ、

> 「1990年代の X68000 と ESP/X で、現代のネットワーク上にある NTP サーバから現在時刻を取得する」

ことそのものを目的としています。

古い TCP/IP 環境には、現在の環境では遭遇しないような仕様差やライブラリ固有の問題があります。

今回の `gethostbyname()` / `h_length` の問題も、その一例でした。

## 23. 今後の予定・拡張候補

必要に応じて以下の機能を追加できます。

- 複数の NTP サーバへのフォールバック
- SNTP 応答のより詳細な検証
- Stratum の確認
- Leap Indicator の確認
- Round Trip Delay の計算
- 複数回問い合わせによる時刻精度向上
- タイムゾーン指定
- RTC 更新処理の整理
- Human68k 用コマンドラインツール化
- 起動時の自動時刻合わせ

ただし、X68000 上で動かすことを最優先とし、必要以上に複雑な実装にはしない方針です。

## 24. ライセンスについて

このプロジェクトのソースコードについては、公開時に別途ライセンスを明記してください。

また、計測技研の ESP/X および `libnetwork.a` は本プロジェクトとは別の製品・ソフトウェアです。

ESP/X や `libnetwork.a` に含まれるソースコード、ヘッダ、ライブラリ等を GitHub に再配布する場合は、それらの権利関係および配布条件を確認してください。

本リポジトリでは、原則として自作した SNTP クライアントのソースのみを公開することを想定しています。

## 25. まとめ

X68000 から SNTP/NTP を利用する場合、プロトコル自体は非常に単純です。

```text
UDP/123
48 byte request
        ↓
48 byte response
        ↓
NTP seconds
        ↓
UNIX time
        ↓
UTC + 9
        ↓
JST
        ↓
Human68k RTC
```

しかし、実際に X68000 / ESP/X で動かしてみると、

- 現代の BSD socket と完全に同じとは限らない
- `socklen()` を使った受信待ちが必要
- `libnetwork.a` の DNS 実装に注意が必要
- 32bit 環境で時刻を扱う必要がある
- Human68k の RTC 更新部分には環境依存がある

など、X68000 ならではの問題があります。

特に今回の開発では、

```c
memcpy(&addr.sin_addr.s_addr, host->h_addr, 4);
```

という一見単純な修正が、SNTP 通信を成立させるための重要なポイントになりました。

古い X68000 を現在のネットワークへ接続し、実際にインターネット上のサービスと通信させるための一つのサンプルとして、このプロジェクトを公開します。

---

## Author

X68000 / Human68k / ESP/X 向けの個人開発プロジェクトです。

## 履歴

1.1　2026/11/23  
     ntp_query() の成功時とパケット異常時にソケットを閉じるようにしました。  
        renatus-novus-xさん ありがとうございました。  
1.0　2026/09/12  
     初回リリース  
