use crypto::aessafe::*;
use crypto::aes::*;
use crypto::aesni::*;
use crypto::symmetriccipher::{Encryptor, Decryptor, BlockEncryptor, BlockDecryptor};
use crypto::blockmodes::{CtrMode, EcbEncryptor, EcbDecryptor, PaddingProcessor};
use crypto::buffer::*;
use rand::prelude::*;
use time;
use rand::seq::SliceRandom;
use super::ctest::TestCrypt;

pub fn aes_instance() {
    if crypto::util::supports_aesni() {
        info!("system support aesni!");
    }

    let mut e = EncryptConn::default();
    e.testcrypt(10);

    // let mut aes256 = Aes256::default();
    // aes256.testcrypt(10);

    // let mut aes128 = Aes128::default();
    // aes128.testcrypt(10);

    // test_aesni(10);
}

fn test_aesni(size :usize) {
    let mut rng = rand::thread_rng(); 
    let mut key = [0u8; 32];
    rng.fill_bytes(&mut key);

    let iv = vec![0u8; 16];
    let mut encoder = CtrMode::new(AesNiEncryptor::new(KeySize::KeySize256, &key), iv.to_vec());
    let mut decoder = CtrMode::new(AesNiEncryptor::new(KeySize::KeySize256, &key), iv.to_vec());

    let n = size * 1024 * 1024;
    let mut rng = rand::thread_rng();
    let mut plain = vec![0u8; n];
    let mut crypto = vec![0u8; n];
    let mut packet = vec![0u8; n];
    rng.fill_bytes(&mut packet);

    let ebt = time::precise_time_ns();
    encoder.encrypt(&mut RefReadBuffer::new(&packet), &mut RefWriteBuffer::new(&mut crypto), false).unwrap();
    let dbt = time::precise_time_ns();
    info!("aes-ni 256 encrypt plain size: {}M, time={}ms", size, (dbt - ebt) / 1000_000);

    decoder.decrypt(&mut RefReadBuffer::new(&crypto), &mut RefWriteBuffer::new(&mut plain), false).unwrap();
    info!("aes-ni 256 decrypt crypto size: {}M, time={}ms", size, (time::precise_time_ns() - dbt) / 1000_000);
        
    assert_eq!(packet, plain);
}


struct EncryptConn {
    encoder: Box<Encryptor>,
    decoder: Box<Decryptor>,
}

 impl Default for EncryptConn {
     fn default() -> Self {
        let mut rng = rand::thread_rng(); 
        let mut key = [0u8; 32];
        rng.fill_bytes(&mut key);
        let iv = vec![0u8; 16];

        if crypto::util::supports_aesni() {
            let mut algo = AesNiEncryptor::new(KeySize::KeySize256, &key);
            let mut ctr1 = CtrMode::new(algo, iv.clone());
            let mut ctr2 = CtrMode::new(algo, iv);            
            EncryptConn {
                encoder: Box::new(ctr1),
                decoder: Box::new(ctr2)
            }
        } else {
            let mut algo = AesSafe256Encryptor::new(&key);
            let mut ctr1 = CtrMode::new(algo, iv.clone());
            let mut ctr2 = CtrMode::new(algo, iv);            
            EncryptConn {
                encoder: Box::new(ctr1),
                decoder: Box::new(ctr2),
            }
        }
     }
 } 

 impl TestCrypt for EncryptConn {
     fn testcrypt(&mut self, size: usize) {
        let n = size * 1024 * 1024;
        let mut rng = rand::thread_rng();
        let mut plain = vec![0u8; n];
        let mut crypto = vec![0u8; n];
        let mut packet = vec![0u8; n];
        rng.fill_bytes(&mut packet);

        let ebt = time::precise_time_ns();
        self.encoder.encrypt(&mut RefReadBuffer::new(&packet), &mut RefWriteBuffer::new(&mut crypto), false).unwrap();
        let dbt = time::precise_time_ns();
        info!("aes256 encrypt plain size: {}M, time={}ms", size, (dbt - ebt) / 1000_000);

        self.decoder.decrypt(&mut RefReadBuffer::new(&crypto), &mut RefWriteBuffer::new(&mut plain), false).unwrap();
        info!("aes256 decrypt crypto size: {}M, time={}ms", size, (time::precise_time_ns() - dbt) / 1000_000);
        
        assert_eq!(packet, plain);
     }
 }

struct Aes256 {
    encoder: CtrMode<AesNiEncryptor>,
    decoder: CtrMode<AesSafe256Encryptor>,  // 加密用AesSafe256Encryptor不用AesSafe256Decryptor是CRT模式的工作原理决定的。无论加密还是解密，都是对Counter加密后再与明文或密文异或
}
 

impl Default for Aes256 {
    fn default() -> Self {
        let iv = vec![0u8; 16];
        let mut key = [0u8; 32];
        let mut rng = rand::thread_rng();
        rng.fill_bytes(&mut key);
        Aes256 {
            encoder: CtrMode::new(AesNiEncryptor::new(KeySize::KeySize256, &key), iv.clone()),
            decoder: CtrMode::new(AesSafe256Encryptor::new(&key), iv),
        }
    }
}

impl TestCrypt for Aes256 {
    fn testcrypt(&mut self, size: usize) {
        let n = size * 1024 * 1024;
        let mut rng = rand::thread_rng();
        let mut plain = vec![0u8; n];
        let mut crypto = vec![0u8; n];
        let mut packet = vec![0u8; n];
        rng.fill_bytes(&mut packet);

        let ebt = time::precise_time_ns();
        self.encoder.encrypt(&mut RefReadBuffer::new(&packet), &mut RefWriteBuffer::new(&mut crypto), false).unwrap();
        let dbt = time::precise_time_ns();
        info!("aes-ni 256 encrypt plain size: {}M, time={}ms", size, (dbt - ebt) / 1000_000);

        self.decoder.decrypt(&mut RefReadBuffer::new(&crypto), &mut RefWriteBuffer::new(&mut plain), false).unwrap();
        info!("aes256 decrypt crypto size: {}M, time={}ms", size, (time::precise_time_ns() - dbt) / 1000_000);
        
        assert_eq!(packet, plain);
    }
}

struct Aes128 {
    encoder: CtrMode<AesSafe128Encryptor>,
    decoder: CtrMode<AesNiEncryptor>,
}

impl Default for Aes128 {
    fn default() -> Self {
        let iv = vec![0u8; 16];
        let mut key = [0u8; 16];
        let mut rng = rand::thread_rng();
        rng.fill_bytes(&mut key);
        Aes128 {
            encoder: CtrMode::new(AesSafe128Encryptor::new(&key), iv.clone()),
            decoder: CtrMode::new(AesNiEncryptor::new(KeySize::KeySize128, &key), iv),
        }
    }
}

impl TestCrypt for Aes128 {
    fn testcrypt(&mut self, size: usize) {
        let n = size * 1024 * 1024;
        let mut rng = rand::thread_rng();
        let mut plain = vec![0u8; n];
        let mut crypto = vec![0u8; n];
        let mut packet = vec![0u8; n];
        rng.fill_bytes(&mut packet);

        let ebt = time::precise_time_ns();
        self.encoder.encrypt(&mut RefReadBuffer::new(&packet), &mut RefWriteBuffer::new(&mut crypto), false).unwrap();
        let dbt = time::precise_time_ns();
        info!("aes128 encrypt plain size: {}M, time={}ms.", size, (dbt - ebt) / 1000_000);

        self.decoder.decrypt(&mut RefReadBuffer::new(&crypto), &mut RefWriteBuffer::new(&mut plain), false).unwrap();
        info!("aes-ni 128 decrypt crypto size: {}M, time={}ms", size, (time::precise_time_ns() - dbt) / 1000_000);

        
        assert_eq!(packet, plain);
    }
}
