use super::aes::*;
use super::blowfish::Bf;
use rand::seq::SliceRandom;
use rand::prelude::*;

pub trait TestCrypt {
    fn testcrypt(&mut self, size: usize);
}


pub fn test_instance() {
    aes_instance();
}