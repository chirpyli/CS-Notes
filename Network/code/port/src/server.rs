//! TCP network program example.
use std::net::{TcpListener, TcpStream, SocketAddr};
// use std::net::{Ipv4Addr, IpAddr};

pub fn run_server() -> Result<(), Box<dyn std::error::Error>> {
    // try bind
    {
        if let Err(e) = TcpListener::bind("0.0.0.0:22222") {
            error!("{}", e);
            return Err(Box::new(e));
        }
    }

    let listener = TcpListener::bind("0.0.0.0:22222").unwrap();
    for stream in listener.incoming() {
        let socket = stream?;
        std::thread::spawn( move || {
            if let Err(e) = handle_client(socket) {
                error!("{}", e);
            }
        });
    }

    Ok(())
}


fn handle_client(stream: TcpStream) -> Result<(), Box<dyn std::error::Error>> {
    info!("accept connect, local: {:?}, remote: {:?}", stream.local_addr()?, stream.peer_addr()?);


    Ok(())
}