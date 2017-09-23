# pg_block_systemcatalog

## 概要
pg_block_systemcatalogは、スーパーユーザおよび許可されたロールを持つユーザを除く、一般のユーザによるシステムカタログへのアクセスを行うクエリをエラーにします。
これにより、一般のユーザが本来見るべきでない、システムカタログ情報を見せなくすることができます。

## 設定
pg_block_systemcatalogを使うためには、PostgreSQL設定ファイル(postgresql.conf)に、以下の設定を追加します。

```
shared_preload_libraries = 'pg_block_systemcatalog'
```

pg_block_systemcatalogは、以下のカスタムパラメータを設定できます。

|パラメータ名|設定値|デフォルト値|パラメータコンテキスト|
|:--|:--|:--|:--|
|pg_block_systemcatalog.allow_role|システムカタログへの参照を許容するロール名|なし|postmaster|

pg_block_systemcatalog.allow_roleの設定例を以下に示します。

```
pg_block_systemcatalog.allow_role = 'monitor_role'
```

pg_block_systemcatalog.allow_role パラメータを設定しない場合、特権をもつユーザ(例：postgres)以外のユーザは、システムカタログにアクセスするクエリを発行するとエラーになります。

## 実行例
特権ユーザ(postgres)と一般ユーザ(user_a)が、データベース名一覧を取得しようとする例を示します。

```
$ psql -U postgres testdb -c "SELECT datname FROM  pg_database"
  datname  
-----------
 postgres
 testdb
 template1
 template0
(4 rows)

$ psql -U user_a testdb -c "SELECT datname FROM  pg_database"
ERROR:  pg_block_systemcatalog: Reference to the system catalog is not permitted.
$
```

# 作者
ぬこ＠横浜 (Twitter ID [@nuko.yokohama](https://twitter.com/nuko_yokohama) )

