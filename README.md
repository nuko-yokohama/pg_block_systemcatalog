# pg_block_systemcatalog

## 概要
pg_block_systemcatalogは、スーパーユーザおよび許可されたロール/ユーザを除く、一般のユーザによるシステムカタログ(システムカタログ、稼動統計情報ビュー、情報スキーマ)へのアクセスを行うクエリをエラーにします。
これにより、一般のユーザが本来見るべきでない、システムカタログ情報を見せなくすることができます。

## 設定
pg_block_systemcatalogを使うためには、PostgreSQL設定ファイル(postgresql.conf)に、以下の設定を追加します。

```
shared_preload_libraries = 'pg_block_systemcatalog'
```

pg_block_systemcatalogは、以下のカスタムパラメータを設定できます。

|パラメータ名|設定値|デフォルト値|パラメータコンテキスト|
|:--|:--|:--|:--|
|pg_block_systemcatalog.allow_role|システムカタログへの参照を許容するロール名|'' (empty string)|superuser|

### allow_roleの設定と利用方法

pg_block_systemcatalog.allow_roleの設定例を以下に示します。
ロールは1つのみ指定します。複数のユーザでシステムカタログの参照を許容したい場合には、pg_block_systemcatalog.allow_role で設定したロールをGRANTで含めるようにします。

```
pg_block_systemcatalog.allow_role = 'monitor_catalog'
```

CREATE ROLEコマンドを用いて、上記で設定したロール(monitor_catalog)を作成します。

```
CREATE ROLE monitor_catalog
```

メタデータへのアクセス用ユーザ(monitor)を作成します。

```
CREATE ROLE monitor LOGIN 
```

メタデータへのアクセス用ユーザ(monitor)に、システムカタログへの参照用のロール(monitor_catalog)を含めます。

```
GRANT monitor_catalog TO monitor
```

monitorユーザでpg_stat_activityなどを監視したい場合には、デフォルトロール pg_monitor も設定したほうが良いでしょう。

```
GRANT pg_monitor TO monitor;
```

\du コマンドでロール状態を確認すると、以下のようになっているはずです。

```
    Role name    |                         Attributes                         |          Member of           
-----------------+------------------------------------------------------------+------------------------------
 db_owner        |                                                            | {}
 monitor         |                                                            | {pg_monitor,monitor_catalog}
 monitor_catalog | Cannot login                                               | {}
 postgres        | Superuser, Create role, Create DB, Replication, Bypass RLS | {}
```

monitorユーザでデータベースにログインすると、システムカタログを参照できます。
db_ownerユーザは monitor_catalog ロールのメンバではないため、システムカタログを参照できません。
また、権限分掌の観点から、monitorユーザから、実際のデータベースオブジェクトに対するアクセス権限を剥奪することを推奨します。

pg_block_systemcatalog.allow_role パラメータを設定しない場合、特権をもつユーザ(例：postgres)以外のユーザは、システムカタログにアクセスするクエリを発行するとエラーになります。

## 実行例
postgres(特権ユーザ)とmonitor(monitor_catalogロールのメンバ)はテーブル名の一覧を取得できます。

```
$ psql -U postgres testdb -c "\d test"
                Table "public.test"
 Column |  Type   | Collation | Nullable | Default 
--------+---------+-----------+----------+---------
 id     | integer |           | not null | 
 data   | text    |           |          | 
Indexes:
    "test_pkey" PRIMARY KEY, btree (id)

$ psql -U monitor testdb -c "\d test"
                Table "public.test"
 Column |  Type   | Collation | Nullable | Default 
--------+---------+-----------+----------+---------
 id     | integer |           | not null | 
 data   | text    |           |          | 
Indexes:
    "test_pkey" PRIMARY KEY, btree (id)

```

しかし、db_owner(monitor_catalogロールのメンバではない)は、テーブルカタログ情報を参照できずエラーとなります。
(たとえ、そのデータベースのオーナであってもエラーとなります)

```
$ psql -U db_owner testdb -c "\d test"
ERROR:  pg_block_systemcatalog: Reference to the system catalog is not permitted.
```

# 動作確認環境
## OS

- CentOS 7

## PostgreSQL

- PostgreSQL 10

# 制約事項

- この拡張を適用しても、バックエンドに関する統計情報を統計情報関数経由で取得できてしまいます。
  - 以下はpg_stat_get_activity関数によよる情報が取得できてしまう例です。
  - なお、pg_stat_get_activity関数結果をFROM句に設定した場合にはエラーとなります。

```
$ psql -U db_owner testdb -c "SELECT pg_stat_get_activity(pg_backend_pid())"
                                                                                                                pg_stat_get
_activity                                                                                                                 
---------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------
 (16387,10392,16385,psql,active,"SELECT pg_stat_get_activity(pg_backend_pid())",,,"2017-09-24 10:32:26.037066+09","2017-09-
24 10:32:26.037066+09","2017-09-24 10:32:26.035487+09","2017-09-24 10:32:26.037067+09",,,-1,,584,"client backend",f,,,,,)
(1 row)

$ psql -U db_owner testdb -c "SELECT * FROM pg_stat_get_activity(pg_backend_pid())"
ERROR:  pg_block_systemcatalog: Reference to the system catalog is not permitted.
$ 
```

- おそらくC言語で実装されたSQL関数内でシステムカタログの情報を取得するようなケースはブロックできないでしょう。SQLやpl/pgsqlで実装されたSQL関数であれば、ブロック可能です。

# 性能への影響
開発環境上でこの拡張を組み込まないときと、組み込んだときの性能差をpgbenchで測定したところ、有意な程の性能差は確認されませんでした。
- pg_block_systemcatalogの設定
  - pg_block_systemcatalog.allow_role = 'monitor_catalog'
- 測定内容
  - pgbench実行ユーザは、monitor_catalog ロールに含まれない一般ユーザ(db_owner)
  - スケールファクタ=5, --unlogged-table モードで作成
  - トランザクションパターンはデフォルトと参照のみ(-S)の2種類
  - 同時実行数は2, 1回の測定時間は60秒, 3回測定。

# TODO

- 他のPostgreSQLバージョンでの動作確認
- README.md の英語化

# 作者
ぬこ＠横浜 (Twitter ID [@nuko.yokohama](https://twitter.com/nuko_yokohama) )

