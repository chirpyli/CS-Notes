#[macro_use]
extern crate log;

use std::net::TcpStream;

fn main() {
    simple_logger::SimpleLogger::new().with_level(log::LevelFilter::Info).init().unwrap();

    let stream = TcpStream::connect("127.0.0.1:22222").unwrap();
    info!("client, connect to remote. {:?}", stream.peer_addr());

}