# example of HTTP REST client with RocksDB and replicator over Pistache server

Full documentation for Pistache is located at [pistache.io](http://pistache.io)

Full documentation for RocksDB is located at [rocksdb.org](http://rocksdb.org/docs/getting-started.html)

# Build and run

`Docker` and `docker-compose` are used for building and running client.

```sh
./build.sh
```

After image fetched and binaries built command

```sh
docker-compose up -d
```

starts master along with 3 replicas. These are accessible via `localhost` ports `9080` to `9083`.

To populate cluster with data issue commands

```sh
curl -X POST 127.0.0.1:9080/record/1/10
curl -X POST 127.0.0.1:9080/record/103/103
```

Command to get all documents back

```sh
curl -X GET 127.0.0.1:9080/all
```

Command to get sequence number

```sh
curl -X GET 127.0.0.1:9080/seq
```

