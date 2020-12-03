因项目需要，在调研分布式KV存储，目前的重点是tikv，先在单机上部署一个三节点集群学习测试用，生产环境中，在单机上部署集群是没有意义的，这里只是资源有限，测试学习用。采用docker部署了3节点pd与3节点tikv。部署如下：

### 获取镜像

```shell
# 拉取镜像
docker pull pingcap/tikv:latest
docker pull pingcap/pd:latest

# 创建自定义网络
docker network create --subnet=172.18.0.0/16 db-network
```

#### 启动PD

分别启动pd1,pd2,pd3。
```shell
docker run -d --name pd1 \
-p 23791:2379 \
-p 23801:2380 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/pd1/data:/data \
--network db-network \
--ip 172.18.0.11 \
pingcap/pd:latest \
--name="pd1" \
--data-dir="/data/pd1" \
--client-urls="http://0.0.0.0:2379" \
--advertise-client-urls="http://172.18.0.11:2379" \
--peer-urls="http://0.0.0.0:2380" \
--advertise-peer-urls="http://172.18.0.11:2380" \
--initial-cluster="pd1=http://172.18.0.11:2380,pd2=http://172.18.0.12:2380,pd3=http://172.18.0.13:2380"
```


```shell
docker run -d --name pd2 \
-p 23792:2379 \
-p 23802:2380 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/pd2/data:/data \
--network db-network \
--ip 172.18.0.12 \
pingcap/pd:latest \
--name="pd2" \
--data-dir="/data/pd2" \
--client-urls="http://0.0.0.0:2379" \
--advertise-client-urls="http://172.18.0.12:2379" \
--peer-urls="http://0.0.0.0:2380" \
--advertise-peer-urls="http://172.18.0.12:2380" \
--initial-cluster="pd1=http://172.18.0.11:2380,pd2=http://172.18.0.12:2380,pd3=http://172.18.0.13:2380"
```

```shell
docker run -d --name pd3 \
-p 23793:2379 \
-p 23803:2380 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/pd3/data:/data \
--network db-network \
--ip 172.18.0.13 \
pingcap/pd:latest \
--name="pd3" \
--data-dir="/data/pd3" \
--client-urls="http://0.0.0.0:2379" \
--advertise-client-urls="http://172.18.0.13:2379" \
--peer-urls="http://0.0.0.0:2380" \
--advertise-peer-urls="http://172.18.0.13:2380" \
--initial-cluster="pd1=http://172.18.0.11:2380,pd2=http://172.18.0.12:2380,pd3=http://172.18.0.13:2380"
```

可运行`curl http://172.18.0.11:2379/pd/api/v1/members`验证是否运行成功，结果如下：
```json
{
  "header": {
    "cluster_id": 6901586045872439576
  },
  "members": [
    {
      "name": "pd2",
      "member_id": 2718137133866114599,
      "peer_urls": [
        "http://172.18.0.12:2380"
      ],
      "client_urls": [
        "http://172.18.0.12:2379"
      ],
      "deploy_path": "/",
      "binary_version": "v4.0.8",
      "git_hash": "775b6a5ef517f8ab2f43fef6418bbfc7d6c9c9dc"
    },
    {
      "name": "pd1",
      "member_id": 12322471550743596225,
      "peer_urls": [
        "http://172.18.0.11:2380"
      ],
      "client_urls": [
        "http://172.18.0.11:2379"
      ],
      "deploy_path": "/",
      "binary_version": "v4.0.8",
      "git_hash": "775b6a5ef517f8ab2f43fef6418bbfc7d6c9c9dc"
    },
    {
      "name": "pd3",
      "member_id": 16951774478298757141,
      "peer_urls": [
        "http://172.18.0.13:2380"
      ],
      "client_urls": [
        "http://172.18.0.13:2379"
      ],
      "deploy_path": "/",
      "binary_version": "v4.0.8",
      "git_hash": "775b6a5ef517f8ab2f43fef6418bbfc7d6c9c9dc"
    }
  ],
  "leader": {
    "name": "pd1",
    "member_id": 12322471550743596225,
    "peer_urls": [
      "http://172.18.0.11:2380"
    ],
    "client_urls": [
      "http://172.18.0.11:2379"
    ]
  },
  "etcd_leader": {
    "name": "pd1",
    "member_id": 12322471550743596225,
    "peer_urls": [
      "http://172.18.0.11:2380"
    ],
    "client_urls": [
      "http://172.18.0.11:2379"
    ],
    "deploy_path": "/",
    "binary_version": "v4.0.8",
    "git_hash": "775b6a5ef517f8ab2f43fef6418bbfc7d6c9c9dc"
  }
}
```
可见当前集群leader节点为pd1，集群有3个节点。

#### 启动Tikv
分别启动tikv1,tikv2,tikv3。
```shell
docker run -d --name tikv1 \
-p 20161:20160 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/tikv1/data:/data \
--network db-network \
--ip 172.18.0.14 \
pingcap/tikv:latest \
--addr="0.0.0.0:20160" \
--advertise-addr="172.18.0.14:20160" \
--data-dir="/data/tikv1" \
--pd="172.18.0.11:2379,172.18.0.12:2379,172.18.0.13:2379"
```

```shell
docker run -d --name tikv2 \
-p 20162:20160 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/tikv2/data:/data \
--network db-network \
--ip 172.18.0.15 \
pingcap/tikv:latest \
--addr="0.0.0.0:20160" \
--advertise-addr="172.18.0.15:20160" \
--data-dir="/data/tikv2" \
--pd="172.18.0.11:2379,172.18.0.12:2379,172.18.0.13:2379"
```

```shell
docker run -d --name tikv3 \
-p 20163:20160 \
-v /etc/localtime:/etc/localtime:ro \
-v /home/sl/Works/db/tikv/tikv3/data:/data \
--network db-network \
--ip 172.18.0.16 \
pingcap/tikv:latest \
--addr="0.0.0.0:20160" \
--advertise-addr="172.18.0.16:20160" \
--data-dir="/data/tikv3" \
--pd="172.18.0.11:2379,172.18.0.12:2379,172.18.0.13:2379"
```
可运行`curl 172.18.0.11:2379/pd/api/v1/stores`验证tikv是否与pd进行正常通讯。结果如下：
```json
{
  "count": 3,
  "stores": [
    {
      "store": {
        "id": 6,
        "address": "172.18.0.16:20160",
        "version": "4.0.8",
        "status_address": "127.0.0.1:20180",
        "git_hash": "83091173e960e5a0f5f417e921a0801d2f6635ae",
        "start_timestamp": 1606902296,
        "deploy_path": "/",
        "last_heartbeat": 1606903297314115592,
        "state_name": "Up"
      },
      "status": {
        "capacity": "313.5GiB",
        "available": "292.4GiB",
        "used_size": "31.5MiB",
        "leader_count": 0,
        "leader_weight": 1,
        "leader_score": 0,
        "leader_size": 0,
        "region_count": 1,
        "region_weight": 1,
        "region_score": 1,
        "region_size": 1,
        "start_ts": "2020-12-02T09:44:56Z",
        "last_heartbeat_ts": "2020-12-02T10:01:37.314115592Z",
        "uptime": "16m41.314115592s"
      }
    },
    {
      "store": {
        "id": 1,
        "address": "172.18.0.14:20160",
        "version": "4.0.8",
        "status_address": "127.0.0.1:20180",
        "git_hash": "83091173e960e5a0f5f417e921a0801d2f6635ae",
        "start_timestamp": 1606902254,
        "deploy_path": "/",
        "last_heartbeat": 1606903295082080004,
        "state_name": "Up"
      },
      "status": {
        "capacity": "313.5GiB",
        "available": "292.4GiB",
        "used_size": "31.5MiB",
        "leader_count": 1,
        "leader_weight": 1,
        "leader_score": 1,
        "leader_size": 1,
        "region_count": 1,
        "region_weight": 1,
        "region_score": 1,
        "region_size": 1,
        "start_ts": "2020-12-02T09:44:14Z",
        "last_heartbeat_ts": "2020-12-02T10:01:35.082080004Z",
        "uptime": "17m21.082080004s"
      }
    },
    {
      "store": {
        "id": 4,
        "address": "172.18.0.15:20160",
        "version": "4.0.8",
        "status_address": "127.0.0.1:20180",
        "git_hash": "83091173e960e5a0f5f417e921a0801d2f6635ae",
        "start_timestamp": 1606902284,
        "deploy_path": "/",
        "last_heartbeat": 1606903294321097291,
        "state_name": "Up"
      },
      "status": {
        "capacity": "313.5GiB",
        "available": "292.4GiB",
        "used_size": "31.5MiB",
        "leader_count": 0,
        "leader_weight": 1,
        "leader_score": 0,
        "leader_size": 0,
        "region_count": 1,
        "region_weight": 1,
        "region_score": 1,
        "region_size": 1,
        "start_ts": "2020-12-02T09:44:44Z",
        "last_heartbeat_ts": "2020-12-02T10:01:34.321097291Z",
        "uptime": "16m50.321097291s"
      }
    }
  ]
}
```
