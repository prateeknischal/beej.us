use std::net;
use std::str;
use std::thread;

fn udp_client(addr: &str) -> std::io::Result<()> {
    let listener = match net::UdpSocket::bind("127.0.0.1:0") {
        Ok(l) => l,
        Err(e) => {
            return Err(e);
        }
    };

    let b = b"Hello from Rust";
    if let Err(e) = listener.send_to(b, addr) {
        return Err(e);
    }

    eprintln!("client: sent {} bytes", b.len());

    Ok(())
}
fn main() {
    let listener = match net::UdpSocket::bind("127.0.0.1:5000") {
        Ok(l) => l,
        Err(e) => {
            panic!("{}", e);
        }
    };

    let handle = thread::spawn(|| udp_client("127.0.0.1:5000"));

    let mut buf = [0u8; 1024];
    if let Ok((recv_bytes, remote_addr)) = listener.recv_from(&mut buf) {
        eprintln!("server: recvd {} bytes from {}", recv_bytes, remote_addr);
        println!("server: got '{}'", str::from_utf8(&buf).unwrap());
    }

    let _ = handle.join();
}
