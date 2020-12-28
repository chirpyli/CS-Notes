#[macro_use]
extern crate log;


mod server;


fn main() {
    simple_logger::SimpleLogger::new().with_level(log::LevelFilter::Info).init().unwrap();
    info!("Hello, world!");

    server::run_server();

}
