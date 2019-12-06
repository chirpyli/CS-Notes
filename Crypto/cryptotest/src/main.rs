#[macro_use]
extern crate log;
extern crate log4rs;
extern crate crypto;
extern crate rand;
extern crate time;

mod aes;
mod blowfish;
mod ctest;


fn main() {
    log4rs::init_file("log.yaml", Default::default()).unwrap();
    ctest::test_instance();
}
