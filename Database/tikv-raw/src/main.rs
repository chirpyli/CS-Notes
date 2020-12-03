#[macro_use]
extern crate log;

use tikv_client::{Config, Key, Value, RawClient, Result};

fn main() {
    simple_logger::SimpleLogger::new().with_level(log::LevelFilter::Info).init().unwrap();
    let name = env!("CARGO_PKG_NAME");
    let version = env!("CARGO_PKG_VERSION");
    info!("{}-{}.", name, version);

    let mut runtime = tokio::runtime::Builder::new()
    .threaded_scheduler()
    .enable_all()
    .build().unwrap();
    runtime.block_on(async {
        info!("run code here.");
        if let Err(e) = connect_tikv().await {
            error!("{:?}", e);
        }
       
    });
    info!("end.");
}


async fn connect_tikv() -> Result<()> {
    let config = Config::default();
    let pd_cluster = vec!["172.18.0.11:2379", "172.18.0.12:2379", "172.18.0.13:2379"];
    let client = RawClient::new_with_config(pd_cluster, config).await?;

    let key1 = Key::from("key".to_string());
    let value1 = "value".to_owned();
    client.put(key1.clone(), value1.to_owned()).await?;
    info!("put key:{:?}, value:{:?}", key1, value1);

    let value = client.get(key1.clone()).await?;
    info!("get key:{:?}, value:{:?}", key1, value);

    Ok(())
}