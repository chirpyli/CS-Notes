use crypto::blowfish::Blowfish;
use crypto::blockmodes::CtrMode;
use crypto::buffer::*;
use crypto::symmetriccipher::{BlockEncryptor, BlockDecryptor};
use rand::seq::SliceRandom;
use rand::prelude::*;
use time;
use super::ctest::TestCrypt;

const BLOCK_SIZE: usize = 8;        //64 bit

pub fn blowfish_instance() {
    let mut rng = rand::thread_rng();
    let mut key32 = vec![0u8; 16];
    rng.fill_bytes(&mut key32);
    let mut bf32 = Bf::new(&key32);
    bf32.testcrypt(10);
}



pub struct Bf {
    encoder: Blowfish,
    decoder: Blowfish,
}

impl Bf {
    pub fn new(key: &[u8]) -> Self {
        assert!(key.len()>=4 && key.len()<=56);
        Bf {
            encoder: Blowfish::new(key),
            decoder: Blowfish::new(key)
        }
    }
}

impl TestCrypt for Bf {
        fn testcrypt(&mut self, size: usize) {
        let n = size * 1024 * 1024;
        let mut rng = rand::thread_rng();
        let mut plain = vec![0u8; BLOCK_SIZE];
        let mut crypto = vec![0u8; BLOCK_SIZE];
        let mut packet = vec![0u8; BLOCK_SIZE];
        rng.fill_bytes(&mut packet);

        let ebt = time::precise_time_ns();
        let count = n / BLOCK_SIZE;
        for _i in 0..count {
            self.encoder.encrypt_block(&mut packet, &mut crypto);
        }
        let dbt = time::precise_time_ns();
        info!("blowfish encrypt plain size: {}M, time={}ms", size, (dbt - ebt) / 1000_000);

        for _i in 0..count {
            self.decoder.decrypt_block(&mut crypto, &mut plain);
        }
        info!("blowfish decrypt crypto size: {}M, time={}ms", size, (time::precise_time_ns() - dbt) / 1000_000);

        assert_eq!(packet, plain);
    } 
}